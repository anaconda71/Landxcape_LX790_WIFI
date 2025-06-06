#include <Arduino.h>
#include "LX790_util.h"
#include "HAL_LX790.h"


/*
 * segments:
 *
 *   -- 1 --
 *  |       |
 *  6       2
 *  |       |
 *   -- 7 --
 *  |       |
 *  5       3
 *  |       |
 *   -- 4 --
 */
unsigned long time_source_code = 0, last_bat_time = 0;
uint8_t last_bat_state = 10; //Give not real battery start value to force first update


#ifndef SEG1
  #define SEG1 0
  #define SEG2 0
  #define SEG3 0
  #define SEG4 0
  #define SEG5 0
  #define SEG6 0
  #define SEG7 0
#endif

struct
{
  const char c;
  const char pattern;
} const SegChr[] =
{
  {' ', 0x00},
  {'1', SEG2 | SEG3},
  {'2', SEG1 | SEG2 | SEG7 | SEG5 | SEG4 },
  {'3', SEG1 | SEG2 | SEG7 | SEG3 | SEG4 },
  {'4', SEG6 | SEG7 | SEG2 | SEG3 },
  {'5', SEG1 | SEG6 | SEG7 | SEG3 | SEG4 },
  {'6', SEG1 | SEG6 | SEG7 | SEG5 | SEG3 | SEG4 },
  {'7', SEG1 | SEG2 | SEG3 },
  {'8', SEG1 | SEG6 | SEG2 | SEG7 | SEG5 | SEG3 | SEG4 },
  {'0', SEG1 | SEG6 | SEG2 | SEG5 | SEG3 | SEG4 },
  {'9', SEG1 | SEG6 | SEG2 | SEG7 | SEG3 | SEG4}, 
  {'E', SEG1 | SEG6 | SEG7 | SEG5 | SEG4 },
  {'r', SEG7 | SEG5 },
  {'o', SEG7 | SEG3 | SEG4 | SEG5},                   // off is "0FF"
  {'F', SEG1 | SEG6 | SEG7 | SEG5},
  {'t', SEG6 | SEG7 | SEG5 | SEG4 },
  {'^', SEG1 },
  {'-', SEG7 },
  {'_', SEG4 },
  {'[', SEG1 | SEG6 | SEG5 | SEG4},
  {']', SEG1 | SEG2 | SEG3 | SEG4},
  {'=', SEG1 | SEG4},
  {'A', SEG1 | SEG6 | SEG2 | SEG7 | SEG5 | SEG3 },
  {'I', SEG3 | SEG2 },                               // !! wie '1'
  {'d', SEG2 | SEG7 | SEG5 | SEG3 | SEG4 },
  {'L', SEG6 | SEG5 | SEG4 },
  {'P', SEG1 | SEG6 | SEG2 | SEG7 | SEG5 },
  {'n', SEG5 | SEG7 | SEG3 },
  {'O', SEG1 | SEG6 | SEG2 | SEG5 | SEG3 | SEG4 },   // !! wie '0'
  {'U', SEG6 | SEG2 | SEG5 | SEG3 | SEG4},
  {'S', SEG1 | SEG6 | SEG7 | SEG3 | SEG4},           // !! wie '5'
  {'b', SEG6 | SEG7 | SEG5 | SEG3 | SEG4},
  {'H', SEG6 | SEG2 | SEG7 | SEG5 | SEG3 },
  {0, 0 }
};


bool return_to_dock = false;


/*****************************************************************************/
struct
{
  const char * Display;
  const LX790_Mode Mode;
  const char * Str;
  const char * Str_en;
} const LcdToMode[] =
{
  {"-F1-", LX790_RAIN,  "Eső miatti késleltetés", "Rain delay activated"},
  {"-E1-", LX790_ERROR, "A munkavégzés övezetén kívül rekedt", "Mower outside work area"},
  {"-E2-", LX790_ERROR, "Kerék elakadt", "Wheel motor blocked"},
  {"-E3-", LX790_ERROR, "Késtárcsa elakadt", "Blade disc blocked"},
  {"-E4-", LX790_ERROR, "Fűnyíró megakadt", "Mower trapped"},
  {"-E5-", LX790_ERROR, "Fűnyírót felemelték", "Mower lifted up"},
  {"-E6-", LX790_ERROR, "Fűnyírót felfordították", "Mower upside down"},
  {"-E7-", LX790_ERROR, "Akku hiba", "Battery error"},
  {"-E8-", LX790_ERROR, "Túl sokáig tart a dokkolóba visszaérkezés", "Takes too long to go back dock station"},
  {"-EE-", LX790_ERROR, "Ismeretlen hiba", "Unknown error"},
  {" OFF", LX790_OFF,   "Kikapcsolva", "Turned off"},
  {"STOP", LX790_STOP,  "Megállítva", "Stopped"},
  {"IDLE", LX790_READY, "Várakozik", "Waiting"},
//  {"[  ]", LX790_DOCKED, "in Ladestation"},
//  {"^^^^", LX790_BLOCKED, "Mähen... Hindernis..."},
  {"Pin1", LX790_SET_PIN, "Új PIN", "New PIN"},
  {"Pin2", LX790_SET_PIN, "Új PIN megerősítése", "New PIN again"},
  {"A 50", LX790_SET_AREA, "Terület 50m2 (kedd, péntek: 9:00-9:30)", "Area 50m2 (Tuesday, Friday: 9:00-9:30)"},
  {"A100", LX790_SET_AREA, "Terület 100m2 (kedd, péntek: 9:00-9:45)", "Area 100m2 (Tuesday, Friday: 9:00-9:45)"},
  {"A150", LX790_SET_AREA, "Terület 150m2 (hétfő, szerda, péntek: 9:00-9:45)", "Area 150m2 (Monday, Wednesday, Friday: 9:00-9:45)"},
  {"A200", LX790_SET_AREA, "Terület 200m2 (Hétfő-péntek: 9:00-9:45)", "Area 200m2 (Monday-Friday: 9:00-9:45)"},
  {"A300", LX790_SET_AREA, "Terület 300m2 (Hétfő-péntek: 9:00-10:00)", "Area 300m2 (Monday-Friday: 9:00-10:00)"},
  {"A400", LX790_SET_AREA, "Terület 400m2 (Hétfő-péntek: 9:00-10:15)", "Area 400m2 (Monday-Friday: 9:00-10:15)"},
  {"A500", LX790_SET_AREA, "Terület 500m2 (Hétfő-péntek: 9:00-10:45)", "Area 500m2 (Monday-Friday: 9:00-10:45)"},
  {"A600", LX790_SET_AREA, "Terület 600m2 (Hétfő-péntek: 9:00-11:00)", "Area 600m2 (Monday-Friday: 9:00-11:00)"},
  {" USB", LX790_USB,     "USB meghajtó észlelve", "USB drive detected"},
  {nullptr, LX790_UNKNOWN, "Ismeretlen hiba", "Unknown error"}
};


/*
ER:50	Error updating main cpu firmware (.pck)
ER:51	Error opening .pck file
ER:52	Error firmware downgrade not allowed
EF:80	Error updating motor firmware (factory first program)
ER:80	Error updating motor firmware
ER:82	Error updating sensorMcu firmware (ultrasound/wire)
ER:83	Error updating imu (BOSCH BNO055)
ER:84	Error initializing imu (BOSCH BNO055)
*/

struct
{
  const char * Display;
  const char * Str;
} const SegmentToLetter[] =
{
  {"5toP", "STOP"},
  {"^^^^", "^^^^"},
  {"____", "^^^^"},
  {"[==]", "[  ]"},
  {"1dLE", "IDLE"},
  {"   -", "IDLE"},
  {"  -1", "IDLE"},
  {" -1d", "IDLE"},
  {"-1dL", "IDLE"},
  {"1dLE", "IDLE"},
  {"dLE-", "IDLE"},
  {"LE- ", "IDLE"},
  {"E-  ", "IDLE"},
  {"-   ", "IDLE"},
  {"   0", " OFF"},
  {"  0F", " OFF"},
  {" 0FF", " OFF"},
  {"0FF ", " OFF"},
  {"FF  ", " OFF"},
  {"F   ", " OFF"},
  {"P1n1", "Pin1"},
  {"P1n2", "Pin2"},
  {" U56", " USB"},
  {"U56 ", " USB"},
  {"  _ ", "  _ "},
  {" _  ", " _  "},
  {nullptr,""}
};

char decodeChar (char raw)
{
  int i = 0;
  
  for (i = 0; SegChr[i].c; i++)
  {
    if (SegChr[i].pattern == raw)
    {
      return SegChr[i].c;
    }
  }
  
  return '#';
}

uint8_t encodeSeg (uint8_t c)
{
  int i = 0;
  
  for (i = 0; SegChr[i].c; i++)
  {
    if (SegChr[i].c == c)
    {
      return SegChr[i].pattern;
    }
  }
  
  return (SEG1 | SEG4 | SEG7);
}

inline bool compareDigits(const char a[4], const char b[4]) {
  return memcmp(a,b,4) == 0;
}

void decodeDisplay(LX790_State &state) {
  static unsigned long lastModeUpdate = 0;
  unsigned long time = millis();
  unsigned long delta = time - lastModeUpdate;
  static bool unlockPin = true;
  //state.msg = "";
  //state.msg2 = "";

  // process segments
  int segCnt=0;
  for (int i=0; i<4; i++)
  {
    byte seg = state.segments[i];
    if ( seg & SEG7 ) {
      segCnt = 0; // invalidate '-' segment
      break;
    }
    while (seg) {
      if (seg & 0x01)
        segCnt++; 
      seg = seg >> 1;
    }
  }

  // normalize digits (e.g. for scrolling text)
  for (int i = 0; SegmentToLetter[i].Display; i++)
  {
    if ( compareDigits(state.digits, SegmentToLetter[i].Display) )
    {
      memcpy(state.digits, SegmentToLetter[i].Str, 4);
      break;
    }
  }
 
  // mode
  // try to decode text
  LX790_Mode detectedMode = LX790_UNKNOWN;
  for (int i = 0; LcdToMode[i].Display; i++)
  {
    if ( compareDigits(state.digits, LcdToMode[i].Display) )
    {
      state.msg = LcdToMode[i].Str;
      detectedMode = LcdToMode[i].Mode;
      state.mode = detectedMode;
      break;
    }
  }
  if ( detectedMode == LX790_UNKNOWN ) {
    if ( compareDigits(state.digits, "8888") && state.point == ':' ) { // all segments active -> self test / power up
      detectedMode = LX790_POWER_UP;
      unlockPin = true;
    } else if (state.lock == true && unlockPin ) {  // mower is locked and last digit is flashing '-' -> enter pin
//      if ( state.mode != LX790_ERROR && state.mode == LX790_RAIN && // do not unlock if in error or rain mode
      if ( ( compareDigits(state.digits, "0---") ||
          ( state.mode == LX790_ENTER_PIN && (state.digits[3]=='-' || delta < 5000) ) )) {
        detectedMode = LX790_ENTER_PIN;
        state.msg = "PIN beírás";
        state.msg2 = "paused";
        if ( state.digits[3]=='-' )
          lastModeUpdate = time; 
      }
    } else if ( compareDigits(state.digits, "[  ]") ) { // display shows box -> in docking station, charging?
      return_to_dock = false;
        //here lx790 is docked, should be find out in the future charged or charging
      time_source_code++;
      if(time_source_code > 4200000000)
      time_source_code = 0;
        
      if(time_source_code > 2000)
      {
        //value fixed, charged
        state.msg2 = "docked";
        detectedMode = LX790_DOCKED;
        state.msg = "Dokkolóban";
      }else
      {
        //value not fixed, charging
        state.msg2 = "docked";
        detectedMode = LX790_CHARGING;
        state.msg = "Töltődik";
      }

      if(state.battery != last_bat_state)
      {
        time_source_code = 0;
      }
      last_bat_state = state.battery;

      /*
        state.msg2 = "docked";
        detectedMode = LX790_DOCKED;
        state.msg = "Dokkolóban";*/

    } else if ( state.mode != LX790_ERROR && state.mode != LX790_RAIN ) {
      if ( segCnt == 1  || (state.mode == LX790_RUNNING && delta < 5000) ) {  // only one dash / segment active , or empty display and was running -> running

        if(return_to_dock == true)
        {
          detectedMode = LX790_RETURN_DOCK;
          state.msg = "Visszatérés a dokkolóra";
          state.msg2 = "mowing"; //mowing, paused, docked, and error
        }
        else
        {
          detectedMode = LX790_RUNNING;
          state.msg = "Fűnyírás";
          state.msg2 = "mowing"; //mowing, paused, docked, and error
        }

        /*for (int i = 0; i<4; i++)
        {
          state.digits[i] = state.segments[i] ? ' ' : '*';
        }*/
        if (segCnt == 1)
          lastModeUpdate = time;
      }
    } else if ( compareDigits(state.digits, "^^^^") && (state.mode == LX790_RUNNING || state.mode == LX790_BLOCKED) ) {
        // blocked
        detectedMode = LX790_BLOCKED;
        state.msg = "Akadály";
        state.msg2 = "error"; //mowing, paused, docked, and error
        lastModeUpdate = time;
    } else if ( compareDigits(state.digits, "    ") ) { // display off -> standby or off
      if ( (state.clock || state.battery) && state.mode != LX790_OFF ) {
        detectedMode = LX790_STANDBY;
        state.msg = "standby";
        state.msg2 = "paused"; //mowing, paused, docked, and error
      } else {
        detectedMode = LX790_OFF;
        memcpy(state.digits, " OFF", 4);
        state.msg = "kikapcsolva";
        state.msg2 = "error"; //mowing, paused, docked, and error
      }
//      unlockPin = true; TODO does mower lock in standby???
    }
  }
  if ( detectedMode != LX790_UNKNOWN ) 
  {
    if ( state.detectedMode == detectedMode ) {
      state.mode = detectedMode;
    }
    state.detectedMode = detectedMode;
  }

  if (state.mode == LX790_ENTER_PIN ) {
    if ( config.pin[0] ) {
      // unlock robot if connected to WiFi
      if ( unlockPin && state.wifi) {
        // unlock robot if connected to WiFi
        static int8_t digitPos = 4;
        state.msg = "Automatikus PIN beírás";
        state.msg2 = "error"; //mowing, paused, docked, and error

        // set current digit position on start
        if ( digitPos >= 4 ) {
          if ( state.digits[0] == '-' ) {
            digitPos = 0;
          }
        }

        // key in current digit
        if ( digitPos < 4 && state.digits[digitPos] != '-' 
              && (uxQueueMessagesWaiting(cmdQueue) == 0) ) {
          if ( state.digits[digitPos] < config.pin[digitPos] ) {
            // send '+' button
            queueButton(BTN_START);
          } else if ( state.digits[digitPos] > config.pin[digitPos] ) {
            // send '-' button
            queueButton(BTN_HOME);
          } else {
            // send OK
            queueButton(BTN_OK);
            CMD_Type cmd = {CMD_Type::WAIT, 400}; xQueueSend(cmdQueue, &cmd, 0);        
            digitPos++;
            if ( digitPos==4 ) {
              unlockPin = false;
            }
          }
        }
      }
    } else {
      // no pin configured
      state.msg = "PIN nincs megadva";
      state.msg2 = "error"; //mowing, paused, docked, and error
    }
  }

}

void queueButton(BUTTONS btn, int delay) {
  CMD_Type cmd;
  cmd = {CMD_Type::BTN_PRESS, btn};
  xQueueSend(cmdQueue, &cmd, 0);
  cmd = {CMD_Type::WAIT, delay};
  xQueueSend(cmdQueue, &cmd, 0);
  cmd = {CMD_Type::BTN_RELEASE, btn};
  xQueueSend(cmdQueue, &cmd, 0);
}
