# Landxcape LX790.1 ESP32 WIFI addon

This is a modified version of the original https://github.com/ahermann86/LX79x_WiFi and https://github.com/the-real-orca/LX79x_WiFi repos.

This project helps to add WIFI function to your lawn mower.

*The usage of the project is under your reponsibility. The programmers not takes reponsibles for any damages. This is not an official update of your robot.*

- Some part of the GUI is in Hungarian version, not translated yet to English. This is in process.

### Further tasks on this repo
- finalize the wiring diagram
- translate the whole stuff to English
- update add summary for MQTT commands
- Add Home Assistant configuration.json example
- finalize first data upload
- document first time wifi config
- sense battery charging

<p align="center">
  <img src=pic/Display.jpg height="450"/>
  <img src=pic/main_page.png height="450"/>
</p>



### Main functions
- Read the display remotely
- Show error messages easily
- Remote control for the buttons
- Communication between the lawn mower and the ESP32
- Added MQTT function & web settings
- Added wifi signal icon
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
1. Open the .ino file
2. Connect ESP32 via USB and choose the desired COM port
3. Upload the "data" folder with 'Devices->ESP32 Sketch Data Upload' function

##### Home Assistant integration
Add the following lines into your configuration.yaml.
Please don't forget to modify the MQTT topic to yours. My topic is "funyiro".
```c
mqtt:
   lawn_mower:
      name: "Lawn Mower"
      unique_id: garden.funyiro
      activity_state_topic: "funyiro/msg2"
      #activity_value_template: "{{ value_json.activity }}"
      pause_command_topic: "funyiro/cmd"
      pause_command_template: "STOP"
      dock_command_topic: "funyiro/cmd"
      dock_command_template: "DOCK"
      start_mowing_command_topic: "funyiro/cmd"
      start_mowing_command_template: "START"

   sensor:
    - name: "Lawn Mower msg"
      unique_id: garden.funyiro_msg
      state_topic: "funyiro/msg"
      
    - name: "Lawn Mower state"
      unique_id: garden.funyiro_mode
      state_topic: "funyiro/mode"
      
    - name: "Lawn Mower RSSI"
      unique_id: garden.funyiro_rssi
      state_topic: "funyiro/rssi"
      device_class: signal_strength
      unit_of_measurement: "dBm"
      
    - name: "Lawn Mower battery"
      unique_id: garden.funyiro_battery
      state_topic: "funyiro/battery"
      device_class: battery
      unit_of_measurement: "%"
```

##### Disable Hunter irrigation with Home Assistant
Add to automation.yaml, Hunter Hydrawise integration neccessary.
```c
- id: ''
  alias: Disable irrigation during lawn mowing
  description: ''
  triggers:
  - trigger: state
    entity_id:
    - sensor.funyiro_mode
    to: LX790_RUNNING
  conditions:
  actions:
  - action: hydrawise.suspend
    metadata: {}
    data:
      until: '2029-12-17 08:00:00'
    target:
      device_id:
      - select your zones to disable
  mode: single
- id: ''
  alias: Enable irrigation when the LX790 in docking station
  description: ''
  triggers:
  - trigger: state
    entity_id:
    - sensor.funyiro_mode
    to: LX790_DOCKED
  - trigger: state
    entity_id:
    - sensor.funyiro_mode
    to: LX790_CHARGING
  conditions:
  actions:
  - action: hydrawise.resume
    metadata: {}
    data: {}
    target:
      device_id:
      - select your zone
  mode: single
  ```