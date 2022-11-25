# VulnerableBeer

VulnerableBeer is a Vulnerabl-by-Design IoT beer tender application. This version runs on a ESP8266 microcontroller, in this case a D1 mini pro. 

## Compiling the firmware

 1. Install Visual Studio code
 2. Install the Platform IO extension. 
    1. Open VSCode Extension Manager
    2. Search for official PlatformIO IDE extension
    3. Install PlatformIO IDE.
 3. In Visual Studio code, open the command palette (Ctrl-Shift-P) and type 'gitcl'.
 4. When prompted for the repository URL, enter the URL of this repository: git@github.com:pieterjm/VulnerableBeer.git
 5. Select a local directrory where to store the repository and open the repository.
 6. Click the checkmark icon in the blue bar at the bottom to compile the repository. This may take a while as external libraries have to be loaded.
 7. Connect the D1 mini microcontroller to a USB port and click the Arrow button in the blue bar at the bottom to compile and flash the firmware on the D1 mini. The application should automatically run after flashing.
 8. To open the serial monitor, and access the serial CLI of the D1 mini, enter Ctrl-Alt-S
 
 
 
 
  
