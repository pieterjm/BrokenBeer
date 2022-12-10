# BrokenBeer

BrokenBeer is a Vulnerable-by-Design IoT beer tender application. The core of BrokenBeer is an ESP8266 microcontroller, in this case a D1 mini.

WARNING: THIS APPLICATION CONTAINS SECURITY VULNERABILTIIES. USE AT YOUR OWN RISK!!!!

The goal of BrokenBeer is to experiment with vulnerabilities in IoT applications. While it is fully operational, there are several routes that compromise the beertap.

## Vulnerable-by-Design beer tender

### Hardware construction
BrokenBeer consists of the following hardware components:
  * A servo motor (https://www.tinytronics.nl/shop/nl/mechanica-en-actuatoren/motoren/servomotoren/td-8120mg-waterproof-digitale-servo-20kg),
  * An ESP8266 microcontroller (https://www.benselectronics.nl/wemos-d1-mini-pro.html).
  * A 5V power adapter
  * Some wires, wood, screws, etc. 
  * A second hand beertender (optional)
  * A 'tapvat' (https://www.beerwulf.com/nl/p/bier/heineken-5l-tapvat)

The servo, controlled by the ESP, operates the lever of the 'tapvat'. Some pictures of my build are shown below:

![](/topview.jpg) ![](/overview.jpg)

### Wiring

The 5V from the powersupply is connected to the servo and the 5V port of the D1 mini. The ground from the powersupply is connected to the servo and the GND port of the D1 mini. Pin D7 of the D1 mini is connected to the signal wire of the servo. 

## Happy flow, ordering a beer

The happy flow is the process of ordering a beer by making a payment. Payments are done using LNbits (https://lnbits.com) an open-source wallet system with various features to connect Bitcoin lightning wallets to devices. After succesfull payment, the beertap receives a message from LNbits via a websocket and starts the process of pouring a beer. 

BrokenBeer requires internet access. When started for the first time (or when no network can be found), an open accesspoint is started (BrokenBeer). When connected to this network, a web page should be opened with a menu to configure internet access. These settings are stored in the D1 mini so that when starting the next time, the network connection is automatically started. If the configured Wi-Fi network is not detected, the open accesspoint will be provided as fall back to reconfigure the wireless network.

Once connected to the internet, a WebSocket connection will be opened to 'legend.lnbits.com'. This is an open 'test' server of LNbits. A wallet in LNbits is configured to send a message via the WebSocket, when someone has payed for a beer. The payment URL is available in the QR code. This can be scanned with any modern Bitcoin wallet (Bluewallet, Wallet of Satoshi, ...).

![](data/lnurlpaymentvulnerablebeer.png)

After a succesfull payment, a message is sent over the WebSocket connection, the valve is opened, a beer is poured, and the valve is closed. 

### Serial interface
A serial command-line interface (CLI) is accessible through the USB connector of the D1 mini. Through the CLI commands can be entered, debug messages can be seen, etc. etc.

As a normal user, three commands are available: 'help', 'reboot' and 'admin'. To enter admin mode, the password has to be entered. The SHA1 hash of the password ('correcthorsebatterystaple') is hardcoded in the application. 

In admin mode, more commands are available. Type 'help' to see them. In this mode, a beer can be entered. 

### Web interface
The beer tap also operates a web interface. Initially only for configuring the Wi-Fi, but later on also as a welcome message with instructions. The web pages are stored in the 'data' directory of the project and stored in flash on the microcontroller.

## Compiling and flashing the firmware

 1. Install Visual Studio code
 2. Install the Platform IO extension. 
    1. Open VSCode Extension Manager
    2. Search for official PlatformIO IDE extension
    3. Install PlatformIO IDE.
 3. In Visual Studio code, open the command palette (Ctrl-Shift-P) and type 'gitcl'.
 4. When prompted for the repository URL, enter the URL of this repository: git@github.com:pieterjm/BrokenBeer.git
 5. Select a local directrory where to store the repository and open the repository.
 6. Click the checkmark icon in the blue bar at the bottom to compile the repository. This may take a while as external libraries have to be loaded.
 7. Connect the D1 mini microcontroller to a USB port and click the Arrow button in the blue bar at the bottom to compile and flash the firmware on the D1 mini. 
 8. To flash the filesystem to the microcontroller, click the PlatformIO (alien) icon on the left in Visual Studio. Expand 'd1_mini' and 'Platform' and click 'Build Filesystem Image' and 'Flash Filesystem Image'. Make sure that the serial monitor is disabled to prevent upload errors.

![Uploading Filesystem](flash_filesystem.png)

 9. After flashing is complete, open the serial monitor, to access the serial CLI of the D1 mini, enter Ctrl-Alt-S
 
 
 
 
  
