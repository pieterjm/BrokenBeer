#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <LittleFS.h>
#include <SimpleCLI.h>
#include <WebSocketsClient.h>
#include <Hash.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include "Servo.h"

FS &FlashFS = LittleFS;

// beer servo PIN
Servo beerServo;

// Strings
#define WIFI_SSID "VulnerableBeer"
#define WIFI_PSK ""
#define ERROR_UNKNOWN_COMMAND_ARGUMENT "Unknown command argument."
#define ERROR_UNKNOWN_COMMAND "Unknown command."
#define ERROR_ACCESS_DENIED "Not allowed."
#define WSC_BEER_COMMAND "1-59"

// config parameters
int config_gpio_pin = 13;    // PIN number of the servo 13 == D7 on the D1 mini
int config_initial_delay = 5000;   // delay before opening the valve
int config_valve_open_delay = 2000;  // delay when valve remains open
int config_after_beer_delay = 2000; // delay after valve is closed
int config_servo_valve_open = 0;  // angle for servo in open position
int config_servo_valve_close = 90; // angle for servo in closed position

String device_id = "ZTSPSCkG";
bool bAdminMode = true; // admin mode enabled or not
bool bDebugMode = true; // verbose debug mode

#define BEER_STATE_UNAVAILEBLE 0    // Beer tap not ready
#define BEER_STATE_AVAILABLE   1    // Beer tap ready to pour a beer
#define BEER_STATE_POURING     2    // Beer tap pouring beer
int beerState = BEER_STATE_UNAVAILEBLE;

String configURL = "https://pastebin.com/raw/";
String adminPassword = "bfd3617727eab0e800e62a776c76381defbc4145"; // SHA1 hash of correcthorsebatterystaple
#define WEBSOCKET_HOST "legend.lnbits.com"
#define WEBSOCKET_PATH "/lnurldevice/ws/fGXLWdJXvgyRfTZ8G2jieM" // legend.lnbits.com

const uint8_t LED_ACTIVE = LOW;

ESP8266WebServer server;
WebSocketsClient webSocket;

// Command Line Stuff
SimpleCLI cli;
String input = "";
Command cmdAdmin;
Command cmdHelp;
Command cmdConfig;
Command cmdDebug;
Command cmdBeer;
Command cmdReboot;

// callback function for CLI 'help' command. Displays a list of available commands
void helpCallback(cmd *cmdPtr)
{
  Command cmd(cmdPtr);
  int argNum = cmd.countArgs();

  if (argNum != 1)
  {
    Serial.println("Available commands");
    Serial.println("help [command] | list available commands");
    Serial.println("admin   | enable/disable administration mode");
    Serial.println("reboot  | reboot device");

    if (bAdminMode)
    {
      Serial.println("beer  | pour a beer to test");
      Serial.println("debug  | enable/disable debug mode");
      Serial.println("config | change configuration settings");
    }

    return;
  }

  String arg = cmd.getArgument(0).getValue();
  if (arg == "admin")
  {
    Serial.println("The admin command enables the administration mode.");
    Serial.println("Provide the admin password as argument to enable administration mode.");
    Serial.println("Provide quit as argument to disable administration mode.");
    return;
  }

  if (bAdminMode && arg == "debug")
  {
    Serial.println("debug | toggle debug mode");
    return;
  }

  if (bAdminMode && arg == "reboot")
  {
    Serial.println("reboot | reboot the device");
    return;
  }

  if (bAdminMode && arg == "config")
  {
    Serial.println("config get | list curren settings");
    Serial.println("config set parameter value | set parameter to value");
    Serial.println("config reload | reload configuration");
    return;
  }

  Serial.println(ERROR_UNKNOWN_COMMAND_ARGUMENT);
}

// this function pours some beer
bool beer()
{
  // check if we're ready to poor a beer
  if ( beerState == BEER_STATE_AVAILABLE ) {
    beerState = BEER_STATE_POURING;
  } else {
    if ( bDebugMode ) {
      Serial.println("Current beerState does not allow pouring beer.");
    }
    return false;
  }

  if ( bDebugMode ) {
    Serial.println("Waiting initial delay");
  }
  delay(config_initial_delay);
  if ( bDebugMode ) {
    Serial.println("Opening valve");
  }
  digitalWrite(LED_BUILTIN, LOW);
  beerServo.write(config_servo_valve_open);
  if ( bDebugMode ) {
    Serial.println("Valve open");
  }
  delay(config_valve_open_delay);
  if ( bDebugMode ) {
    Serial.println("Closing valve");
  }
  beerServo.write(config_servo_valve_close);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println(LED_BUILTIN);
  if ( bDebugMode ) {
    Serial.println("Waiting after a beer has been poured");
  }
  delay(config_after_beer_delay);

  if ( bDebugMode ) {
    Serial.println("Returning to available beerState");
  }
  beerState = BEER_STATE_AVAILABLE;
  return true;
}

// CLI callback function for 'beer' command. test beer pouring function from the CLI
void beerCallback(cmd *cmdPtr)
{
  if (!bAdminMode)
  {
    Serial.println(ERROR_ACCESS_DENIED);
  }
  Command cmd(cmdPtr);
  int argNum = cmd.countArgs();

  if (argNum != 0)
  {
    Serial.println(ERROR_UNKNOWN_COMMAND_ARGUMENT);
    return;
  }

  Serial.println("Pouring a test beer");
  if ( beer() == true) {
    Serial.println("Finished");
  } else {
    Serial.println("Beer tap not available");
  }

 
}

// CLI callback function for 'reboot' command. Reboors the uc
void rebootCallback(cmd *cmdPtr)
{
  if (!bAdminMode)
  {
    Serial.println(ERROR_ACCESS_DENIED);
  }

  Command cmd(cmdPtr);
  int argNum = cmd.countArgs();

  if (argNum != 0)
  {
    Serial.println(ERROR_UNKNOWN_COMMAND_ARGUMENT);
    return;
  }

  Serial.println("Rebooting in 5 seconds");
  delay(5000);
  ESP.restart();
}

// CLI callback for 'debug' command, toggles debug mode
void debugCallback(cmd *cmdPtr)
{
  if (!bAdminMode)
  {
    Serial.println("Not allowed");
  }

  Command cmd(cmdPtr);
  int argNum = cmd.countArgs();

  if (argNum != 0)
  {
    Serial.println(ERROR_UNKNOWN_COMMAND_ARGUMENT);
    return;
  }

  bDebugMode = !bDebugMode;

  if (bDebugMode)
  {
    Serial.println("Debug mode enebled");
  }
  else
  {
    Serial.println("Debug mode disabled");
  }
}

// CLI callback for 'config' command, gets/sets config
void configCallback(cmd *cmdPtr)
{
  if (!bAdminMode)
  {
    Serial.println(ERROR_ACCESS_DENIED);
    return;
  }

  Command cmd(cmdPtr);
  int argNum = cmd.countArgs();

  if ((argNum == 1) && (cmd.getArgument(0).getValue() == "get"))
  {
    Serial.println("Current settings");
    Serial.println("initial_delay    | milliseconds | Initial delay before valv open | " + String(config_initial_delay));
    Serial.println("valve_open_delay | milliseconds | Valve opening time in ms       | " + String(config_valve_open_delay));
    Serial.println("after_beer_delay | milliseconds | Wait time after beer in ms     | " + String(config_after_beer_delay));
    Serial.println("gpio_pin         | integer      | GPIO pin nummber of valve      | " + String(config_gpio_pin));
    Serial.println("device_id        | String       | device identifier              | " + device_id);
    return;
  }

  // the reload function is currently out of order, not all config variables are missing, etc. etc.
  if ((argNum == 1) && (cmd.getArgument(0).getValue() == "reload"))
  {
    Serial.println("Reloading configuration not available");
    return;
  }

  // set config through the CLI
  if ((argNum == 3) && (cmd.getArgument(0).getValue() == "set"))
  {
    String param = cmd.getArgument(1).getValue();
    String value = cmd.getArgument(2).getValue();

    if ( param == "initial_delay" ) {
      config_initial_delay = value.toInt();
    } else if ( param == "valve_open_delay" ) {
      config_valve_open_delay = value.toInt();
    } else if ( param == "after_beer_delay" ) {
      config_after_beer_delay = value.toInt();
    } else if ( param == "gpio_pin" ) {
      config_gpio_pin = value.toInt();
    } else if ( param == "device_id" ) {
      device_id = value;
    } else {
      Serial.println("Unknown parameter");
    }

    return;
  }

  Serial.println(ERROR_UNKNOWN_COMMAND_ARGUMENT);
}

// callback function for handling websocket events
void webSocketEventHandler(WStype_t type, uint8_t *payload, size_t length) 
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    if ( bDebugMode ) {
      Serial.printf("[WSc] Disconnected!\n");
    }
    break;
  case WStype_CONNECTED:
  {
    if ( bDebugMode ) {
      Serial.printf("[WSc] Connected to url: %s\n", payload);
      Serial.println("[WSc] SENT: Connected");
    }
    webSocket.sendTXT("Connected");
  }
  break;
  case WStype_TEXT:
    if ( bDebugMode ) {
      Serial.printf("[WSc] RESPONSE: %s\n", payload);
    }

    // Check if the payload is the beer command
    if ( String((char *)payload) == WSC_BEER_COMMAND ) {
      if ( bDebugMode ) {
        Serial.println("Pouring beer from websocker");
      }
      if ( beer() ) {
        Serial.println("Beer from websocket completed");
      } else {
        Serial.println("Beer from websocket not allowed. Customer could be wanting money back!");
      }
    }
    break;
  case WStype_BIN:
    if ( bDebugMode ) {
      Serial.printf("[WSc] get binary length: %u\n", length);
    }
    break;
  case WStype_PING:
    // pong will be send automatically
    //Serial.printf("[WSc] get ping\n");
    break;
  case WStype_PONG:
    // answer to a ping we send
    //Serial.printf("[WSc] get pong\n");
    break;
  case WStype_ERROR:
    if ( bDebugMode ) {
      Serial.println("]Wsc] Got error");
    }
    break;
  default:
    break;
  }
}

// CLI callback function for 'admin' command. Login function to obtain admin privs
void adminCallback(cmd *cmdPtr)
{
  Command cmd(cmdPtr);
  int argNum = cmd.countArgs();
  if (argNum != 1)
  {
    Serial.println(ERROR_UNKNOWN_COMMAND_ARGUMENT);
    return;
  }

  String arg = cmd.getArgument(0).getValue();
  if (arg == "quit")
  {
    bAdminMode = false;
    bDebugMode = false;
    Serial.println("Admin mode disabled.");
    return;
  }

  if (bAdminMode)
  {
    Serial.println("You are already an admin. There is no God mode.");
    return;
  }

  String hashHex = sha1(arg);
  if (bDebugMode)
  {
    Serial.println("hash: " + hashHex);
  }

  if (hashHex == adminPassword)
  {
    bAdminMode = true;
    Serial.println("Admin mode enabled");
    return;
  }
  else
  {
    Serial.println(ERROR_ACCESS_DENIED);
    delay(5000);
    return;
  }
}

// CLI callback function in case of an error
void errorCallback(cmd_error *errorPtr)
{
  CommandError e(errorPtr);
  Serial.println("ERROR: " + e.toString());
}

// get content type based on extension
String getContentType(String filename) {
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  else if (filename.endsWith(".txt")) return "text/plain";
  return "text/plain";
}

// utility function for the webserver, provides files from the data directory
void handleFileRequest(String path) {
  if ( bDebugMode ) {
    Serial.println("Request for resource: " + path);
  }

  String contentType = getContentType(path);
  File file = LittleFS.open(path,"r");
  if ( file ) {
    server.streamFile(file, contentType);                 //Send it to the client
    file.close();                                                  //Close the file again
  } else {
    Serial.println("File does not exist");
    server.send(404, "text/plain", "404: Not Found");  
  }
}

void setup()
{
  // Initialise serial port and print welcome message
  delay(1000);
  Serial.begin(115200);
  delay(5000);
  Serial.println("Initializing Vulnerable Beer Dispenser");
  Serial.println("");

  // initialize flash storage
  Serial.println("Initializing flash storage");
  if ( !LittleFS.begin() ) {
    Serial.println("Could not start Flash Filesystem");
  }


  // attaching servo  
  Serial.println("Attaching servo");
  pinMode(LED_BUILTIN, OUTPUT);  // Enable the LED on the D1 mini to light when a beer is poured
  beerServo.attach(config_gpio_pin);
  delay(1000);
  beerServo.write(config_servo_valve_close); // close servo valve
  digitalWrite(LED_BUILTIN, HIGH);  // turn off the LED


  // bind callbacks to commands on the CLI
  cmdHelp = cli.addBoundlessCommand("help", helpCallback);
  cmdAdmin = cli.addBoundlessCommand("admin", adminCallback);
  cmdConfig = cli.addBoundlessCommand("config", configCallback);
  cmdDebug = cli.addBoundlessCommand("debug", debugCallback);
  cmdBeer = cli.addBoundlessCommand("beer", beerCallback);
  cmdReboot = cli.addBoundlessCommand("reboot", rebootCallback);
  cli.setOnError(errorCallback);

  // Wi-Fi portal start
  Serial.println("Starting Wi-Fi portal");
  WiFiManager wifiManager;
  wifiManager.autoConnect(WIFI_SSID);

  // Start websockets client
  delay(1000);
  Serial.println("Initializing WebSocket client");
  webSocket.beginSSL(WEBSOCKET_HOST, 443, WEBSOCKET_PATH);
  webSocket.onEvent(webSocketEventHandler);
  webSocket.setReconnectInterval(5000);

  // web server config
  server.serveStatic("/",LittleFS,"/");
  server.begin();

  delay(1000);
  Serial.println("CLI ready");
  Serial.print(" > ");

  // make beer tap available
  beerState = BEER_STATE_AVAILABLE;
}

void loop()
{
  // keep webserver running
  server.handleClient();

  // keep websocket connection running
  webSocket.loop();

  // CLI event loop. Reads input from serial port, if enter is pressed, the gathered input is parsed and processed
  if (Serial.available())
  {
    char c = Serial.read();
    Serial.print(c);
    if (c == '\n')
    {
      cli.parse(input);
      input = "";
      Serial.print(" > ");
    }
    else
    {
      input += c;
    }
  }
}