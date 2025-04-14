# 1 "C:\\Users\\Szabolcs\\AppData\\Local\\Temp\\tmpj1kk9hhc"
#include <Arduino.h>
# 1 "C:/Users/Szabolcs/Desktop/Landxcape_LX790_WIFI/src/LX790_ESP32.ino"
#include <Arduino.h>
#include <string.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "LX790_util.h"


#include "config.h"


TaskHandle_t hTaskHW;
TaskHandle_t hTaskWeb;

void TaskHW( void * pvParameters );
void TaskWeb( void * pvParameters );

SemaphoreHandle_t stateQueue = NULL;
QueueHandle_t cmdQueue = NULL;

LX790_Config config;
void setup();
void loop();
#line 22 "C:/Users/Szabolcs/Desktop/Landxcape_LX790_WIFI/src/LX790_ESP32.ino"
void setup()
{
  Serial.begin(115200);
  while (!Serial);
  Serial.print("Build date: "); Serial.print (__DATE__);
  Serial.print(" time: "); Serial.println(__TIME__);

  if(!SPIFFS.begin(true))
    Serial.println(F("init SPIFFS error"));


  config.captivePortal = false;
  config.portalTimeout = 10 * 60;

  File file = SPIFFS.open("/config.json", "r");


  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));


  config.hostname = doc["name"] | "LX790";
  strncpy(config.pin, doc["pin"] | "1234", 4);
  config.wifiEnabled = doc["wifiEnabled"] | true;
  config.wifiSSID = doc["wifiSSID"] | "SSID";
  config.wifiPassword = doc["wifiPassword"] | "";
  config.captivePortal = doc["captivePortal"] | true;
  config.portalTimeout = doc["portalTimeout"] | 600;
  config.portalPassword = doc["portalPassword"] | "";
  config.mqtt_server_ip = doc["mqtt_server_ip"] | "";
  config.mqtt_port = doc["mqtt_port"] | "";
  config.mqtt_user = doc["mqtt_user"] | "";
  config.mqtt_password = doc["mqtt_password"] | "";
  config.mqtt_topic = doc["mqtt_topic"] | "";
  file.close();


  stateQueue = xQueueCreate(2, sizeof(LX790_State));
  if (stateQueue == NULL) {
    Serial.println(F("init state queue error"));
    while(1) {}
  }

  cmdQueue = xQueueCreate(16, sizeof(CMD_Type));
  if (cmdQueue == NULL) {
    Serial.println(F("init command queue error"));
    while(1) {}
  }

  xTaskCreatePinnedToCore(
    TaskHW,
    "TaskHW",
    10000,
    NULL,
    1,
    &hTaskHW,
    0);

  delay(500);

  xTaskCreatePinnedToCore(
    TaskWeb,
    "TaskWeb",
    10000,
    NULL,
    1,
    &hTaskWeb,
    1);
}

void loop()
{

  delay(1000);
}