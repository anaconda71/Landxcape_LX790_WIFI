# Landxcape LX790.1 ESP32 WIFI addon

This is a modified version of the original https://github.com/ahermann86/LX79x_WiFi and https://github.com/the-real-orca/LX79x_WiFi repos.

This project helps to add WIFI function to your lawn mower.

*The usage of the project is under your reponsibility. The programmers not takes reponsibles for any damages. This is not an official update of your robot.*

- Some part of the GUI is in Hungarian version, not translated yet to English.

### Further tasks on this repo
- finalize the wiring diagram
- translate the whole stuff to English
- upda- add summary for MQTT commands
- Add Home Assistant configuration.json example
- finalize first data upload
- document first time wifi config

<p align="center">
  <img src=pic/Display.jpg height="450"/>
  <img src=pic/Web_with_mower.png height="450"/>
</p>



### Main functions
- Read the display remotely
- Show error messages easily
- Remote control for the buttons
- SPI communication between the lawn mower and teh ESP32
- Added MQTT function & web settings
- Added wifi signal icon
- No debugging, no syslog, no any other unneccessary thing
- Easy control from a website
- Home Assistant integration easily possible
- Easily can be disable Hunter X2 wifi irrigation controller during lawn mowing

### Requirements
#### Hardware
Any ESP32 module can be used if have SPI bus. My suggestion to use external antenna if your garden is big place it inside the lawn mower housing.

##### SPI bus
The lawn mower mainboard communicate with the display via SPI bus, this ESP32 can listen this bus

#### Software
You must use Arduino IDE to flash the ESP32 first time.

##### Preparation steps
1. Install Arduino IDE
2. ESP32 Core install (https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
3. Boardtype choose: 'Devices->Board->DOIT ESP32 DEVKIT V1'
4. Add CRC library: 'Devices->Libraries...->CRC (Rob Tillaart)'
5. Filesystem Uploader: (https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/)
  1. Download ESP32FS ZIP: https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/
  2. UNZIP to "c:\Program Files (x86)\Arduino\tools\" entpacken
  3. Restart arduino

##### ESP32 flash and update data:
1. Open the .info file
2. Connect ESP32 via USB and choose the desired COM port
3. In the ino code please modify your SSID:
```c
const char* ssid     = "DEINESSID";
const char* password = "DEINPASSWORT";
```
4. Upload the "data" folder with 'Devices->ESP32 Sketch Data Upload' function