#pragma once
#include <Arduino.h>
#define LX790_V1_1 0x11


#define SW_VER "v2.4"


#define DEBUG_SERIAL_PRINT 0

#if DEBUG_SERIAL_PRINT
  #define DEBUG_println(x)  Serial.println(x)
  #define DEBUG_print(x)  Serial.print(x)
  #define DEBUG_printf(x, y)  Serial.printf(x, y)
#else  
  #define DEBUG_println(x)
  #define DEBUG_print(x)
  #define DEBUG_printf(x, y)
#endif

/*
 * config structure
 */
typedef struct {
  bool wifiEnabled;
  bool captivePortal;
  String portalPassword;
  uint32_t portalTimeout;
  String wifiSSID;
  String wifiPassword;
  String hostname;
  String mqtt_server_ip;
  String mqtt_user;
  String mqtt_port;
  String mqtt_password;
  String mqtt_topic;
  char pin[5];
} LX790_Config;
extern LX790_Config config;

