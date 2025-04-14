#include "config.h"

#include <ESP32SPISlave.h>
#include "LX790_util.h"
#include "HAL_LX790_V1_1.h"
#include "config.h"
#include <PubSubClient.h>
#include <wifi.h>
#include <AsyncTCP.h>
#include <WiFiClient.h>


#include <Arduino.h>
#include <string.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

#warning "Don't forget to update the software version in settings.h"

WiFiClient espClient;
PubSubClient client(espClient);

String tmp_topic;

void callback_mqtt(char* topic, byte* message, unsigned int length) {
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
//process mqtt incoming messages
tmp_topic = "";
tmp_topic = config.mqtt_topic + "/cmd";

if (String(topic) == tmp_topic) {
  tmp_topic = "";
  tmp_topic = config.mqtt_topic + "/cmd";
  CMD_Type cmd;
  if(messageTemp == "START"){
    queueButton(BTN_START, 250);
    return_to_dock = false;
    cmd = {CMD_Type::WAIT, 250}; xQueueSend(cmdQueue, &cmd, 0);
    queueButton(BTN_OK, 250);
    
    client.publish(tmp_topic.c_str(), "WAIT_CMD");
  }
    else if(messageTemp == "DOCK"){
      return_to_dock = true;
      queueButton(BTN_STOP, 250);
      cmd = {CMD_Type::WAIT, 1500}; xQueueSend(cmdQueue, &cmd, 0);
      queueButton(BTN_HOME, 250);
      cmd = {CMD_Type::WAIT, 250}; xQueueSend(cmdQueue, &cmd, 0);
      queueButton(BTN_OK, 250);

      client.publish(tmp_topic.c_str(), "WAIT_CMD");
    }else if(messageTemp == "STOP"){
      //stop
      return_to_dock = false;
      queueButton(BTN_STOP, 250);
      client.publish(tmp_topic.c_str(), "WAIT_CMD");
    }
}

}

static inline byte getButtonPin(BUTTONS btn) {
  switch (btn) {
    case BTN_IO:
      return BTN_PIN_IO;
    case BTN_START:
      return BTN_PIN_START;
    case BTN_HOME:
      return BTN_PIN_HOME;
    case BTN_OK:
      return BTN_PIN_OK;
    case BTN_STOP:
      return BTN_PIN_STOP;
    default:
      return 0;
  }  
}

void HAL_buttonPress(BUTTONS btn) {
  byte pin = getButtonPin(btn);
  if ( btn == BTN_STOP ) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 1);
  } else {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
  }
}

void HAL_buttonRelease(BUTTONS btn) {
  byte pin = getButtonPin(btn);
  if ( btn == BTN_STOP ) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
  } else {
    pinMode(pin, INPUT);
  }
  //DEBUG_printf("button released (D%d)\n", pin);
}

bool decodeTM1668(const uint8_t raw[14], LX790_State &state) {
  byte val;
  bool updated = false;

  // LCD digits
  uint16_t segCnt = 0;
  bool error = false;
  for (int i=0; i<4; i++) {
    byte segments = 0;
    byte mask = 1 << i;
    for (int j=0; j<7; j++) {
      segments |= (raw[j*2] & mask) ? (1<<j) : 0;
    }
    if ( state.segments[i] != segments ) updated = true;
    state.segments[i] = segments;
    byte seg = segments;
    while (seg) {
      if (seg & 0x01)
        segCnt++; 
      seg = seg >> 1;
    }

    if ( decodeChar(segments) == '#' ) {
      // error reading display
      error = true;
    }
  }
  // return if error on decoding digits, if exactly 1 segment active -> running (no error)
  if ( error && segCnt != 1 )
    return false;

  for (int i=0; i<4; i++) {
    state.digits[i] = decodeChar(state.segments[i]);
  }

  // clock
  val = bitRead(raw[0*2], 4) | bitRead(raw[1*2], 4);
  if ( state.clock != val ) updated = true;
  state.clock = val;

  // wifi
  // wifi is output / state.wifi = bitRead(raw[2*2], 4) | bitRead(raw[3*2], 4);

  // lock
  val = bitRead(raw[4*2], 4) | bitRead(raw[5*2], 4);
  if ( state.lock != val ) updated = true;
  state.lock = val;

  // battery
  val = bitRead(raw[2*2], 6) + bitRead(raw[1*2], 6) + bitRead(raw[0*2], 6);
  if ( state.battery != val ) updated = true;
  state.battery = val;

  // dots
  val = ' ';
  if ( bitRead(raw[3*2], 6) )
      val = '\'';
  if ( bitRead(raw[4*2], 6) )
      val = '.';
  if ( bitRead(raw[3*2], 6) && bitRead(raw[4*2], 6) )
      val = ':';
  if ( state.point != val ) updated = true;
  state.point = val;

  if ( updated ) {
    state.updated = true;
    state.updateTime = millis();
  }

  return true;
}

ESP32SPISlave tm1668;
uint8_t spi_slave_rx_buf[32];



void HAL_setup()
{
  // init HW Communication
  tm1668.setDataMode(SPI_MODE1);
  tm1668.setSlaveFlags(SPI_SLAVE_BIT_LSBFIRST);
  tm1668.begin(VSPI, CLK_PIN_DISPLAY, VSPI_MISO, DIO_PIN_DISPLAY, CS_PIN_DISPLAY);
//  tm1668.begin(HSPI);

  pinMode(CS_PIN_DISPLAY, INPUT);
  pinMode(DIO_PIN_DISPLAY, INPUT);
  pinMode(CLK_PIN_DISPLAY, INPUT);

  digitalWrite(BTN_PIN_IO, 0);
  digitalWrite(BTN_PIN_START, 0);
  digitalWrite(BTN_PIN_HOME, 0);
  digitalWrite(BTN_PIN_OK, 0);
  digitalWrite(BTN_PIN_STOP, 0);
  pinMode(BTN_PIN_IO, INPUT);
  pinMode(BTN_PIN_START, INPUT);
  pinMode(BTN_PIN_HOME, INPUT);
  pinMode(BTN_PIN_OK, INPUT);
  pinMode(BTN_PIN_STOP, OUTPUT);
  
  //init MQTT
  client.setServer(config.mqtt_server_ip.c_str(), config.mqtt_port.toInt());//config.mqtt_server_ip.c_str() config.mqtt_port.toInt()
  client.setCallback(callback_mqtt);

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect("LX790", config.mqtt_user.c_str(), config.mqtt_password.c_str())) {
      // Subscribe
      tmp_topic = "";
      tmp_topic = config.mqtt_topic + "/cmd";
      client.subscribe(tmp_topic.c_str());
    } else {
      // Wait 1 seconds before retrying
      delay(1000);
    }
  }
}

int period = 1000;
unsigned long time_now = 0;
unsigned long previousTime = 0;



int old_rssi = 0;
uint8_t old_batval = 105;
LX790_Mode old_mode = LX790_UNKNOWN;
LX790_Mode old_detectedMode = LX790_UNKNOWN;  
String old_msg = "x";
String old_msg2 = "x";
String old_waitcmd = "x";


void HAL_loop(LX790_State &state) {
  static LX790_State oldState = {0};
  static long btnHomeTime = -1;
  static byte btnHomeVal = -1;

  // read data via SPI
  if (tm1668.remained() == 0)
    tm1668.queue(spi_slave_rx_buf, sizeof spi_slave_rx_buf);

  // LCD
  while (tm1668.available()) {
    int size = tm1668.size();
    if ( size ) {
      uint8_t cmd = spi_slave_rx_buf[0];
      switch ( cmd & DISPLAY_CMD_MASK ) {
        case DISPLAY_CMD_MODE_SET:
        case DISPLAY_CMD_DATA_SET:
          // ignore
          break;
        
        case DISPLAY_CMD_CONTROL:
          state.brightness = bitRead(cmd, 4) ? 0 : (cmd & 0xE) + 1;
          break;

        case DISPLAY_CMD_ADDRESS:
          if ( size == 15) {
            decodeTM1668(spi_slave_rx_buf+1, state);
          }
          break;
      }
    }
    tm1668.pop();
  }

  // press "HOME" button for 10 seconds to start captive portal
  unsigned long time = millis();
  byte pin = getButtonPin(BTN_HOME);
  byte val = digitalRead(pin);
  if ( btnHomeVal != val ) {
    //DEBUG_print("button home: "); DEBUG_println(val);
    btnHomeVal = val;
    btnHomeTime = time;
  }
  
  if ( !config.captivePortal && state.brightness> 0 && btnHomeVal == 0 && ( (time-btnHomeTime) > 9000) ) {
    btnHomeTime = time; // reset timer for home botton to prevent multiple execution
    DEBUG_println("home button pressed 10s -> activate captive portal ");
    CMD_Type xcmd;
    xcmd = {CMD_Type::DISCONNECT, 0}; xQueueSend(cmdQueue, &xcmd, 0);
    xcmd = {CMD_Type::WAIT, 1000}; xQueueSend(cmdQueue, &xcmd, 0);
    xcmd = {CMD_Type::WIFI_PORTAL, 0}; xQueueSend(cmdQueue, &xcmd, 0);
  }



  if (!client.connected()) {

    reconnect();
  }
  client.loop();

  /* Updates frequently */
  time_now = millis();
  /* This is the event */
  if (time_now - previousTime >= 2000) {
    /* Event code */
    

  
    tmp_topic = "";
    tmp_topic = config.mqtt_topic + "/SW_VER";
    client.publish(tmp_topic.c_str(), SW_VER);
  
    tmp_topic = "";
    tmp_topic = config.mqtt_topic + "/cmd";
    client.publish(tmp_topic.c_str(), "WAIT_CMD");
    
    int rssi = WiFi.RSSI();
    /*if(old_rssi != rssi)
    {*/
      char msg_out[20];
      sprintf(msg_out, "%d",rssi);

      tmp_topic = "";
      tmp_topic = config.mqtt_topic + "/rssi";
      client.publish(tmp_topic.c_str(), msg_out);
    /*}
    old_rssi = rssi;*/
    
    char msg_out2[20];
    int batval = 0;
    switch (state.battery)
    {
      case 0:
        batval = 10;
        break;

      case 1:
        batval = 40;
        break;

      case 2:
        batval = 70;
        break;

      case 3:
        batval = 100;
        break;
      
      case 4: //toltes
        batval = 100; 
        break;

      default:
        batval = 0;
    }
    /*if(old_batval != batval)
    {*/
      sprintf(msg_out2, "%d",batval);
      tmp_topic = "";
      tmp_topic = config.mqtt_topic + "/battery";
      client.publish(tmp_topic.c_str(), msg_out2);
    /*}
    old_batval = batval;*/

    /*if(old_detectedMode != state.detectedMode)
    {*/
      tmp_topic = "";
      tmp_topic = config.mqtt_topic + "/mode";
      client.publish(tmp_topic.c_str(), ModeNames[state.detectedMode]);
    /*}
    old_detectedMode = state.detectedMode;

    if(old_msg != state.msg)
    {*/
      int str_len = state.msg.length() + 1; 
      char msg_out3[str_len];
      state.msg.toCharArray(msg_out3, str_len);

      tmp_topic = "";
      tmp_topic = config.mqtt_topic + "/msg";
      client.publish(tmp_topic.c_str(), msg_out3);
    /*}
    old_msg = state.msg;

    if(old_msg2 != state.msg2)
    {*/
      int str_len2 = state.msg2.length() + 1; 
      char msg_out32[str_len2];
      state.msg2.toCharArray(msg_out32, str_len2);

      tmp_topic = "";
      tmp_topic = config.mqtt_topic + "/msg2";
      client.publish(tmp_topic.c_str(), msg_out32);
    /*}
    old_msg2 = state.msg2;*/

   /* Update the timing for the next time around */
    previousTime = time_now;
  }

}
