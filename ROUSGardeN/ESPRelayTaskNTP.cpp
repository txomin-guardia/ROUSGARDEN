/*
   ESPrelayTask library for ESP8266

   Executes the realy task based in the time schedule.

   This routine gets the unixtime from a NTP server and adjusts it to the time zone and the
   Middle European summer time if requested

   Copyright (c) 2019 Txomin Guardia

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.

   V1.0 2019-10-6

*/

#include "ESPRelayTaskNTP.h"

extern "C" {
#include "user_interface.h" // this is for the RTC memory read/write functions
}

enum  ActivateOption {
  Off,
  On
};

enum statusDef {
  RESET,
  COUNTING,
  CHECK,
  WORK
};

typedef struct {
  byte markerFlag; //checked for memory data integrity
  byte counter; //remaining hours for the next task to execute
  byte nextTask; //number of the next task to execute
  unsigned int timeTaskOn; //in seconds
  bool wifiConnectionFailed;
  statusDef status; //actual status of the tasks

#ifdef WORK_WITH_E_PAPER
  strDateTime wakeuptime;
#endif

} rtcStore __attribute__((aligned(4)));

const ActivateOption WithSerialDebug = Off;
const unsigned long ONE_SECOND = 1000000; //In us
const unsigned long ONE_HOUR =  3600 * ONE_SECOND; //In us
const unsigned long MIN_SLEEP_TIME = 5000;  //Minimun sleep time for correct restart in WEMOS board (in ms)
const unsigned long MAX_WIFI_WAIT_TIME = 20000;  //Maximun waiting time for correct WIFI connection (in ms)
const char *statusDefNames[] = { "RESET", "DESCONT.", "COMPROB.", "ON + WiFi"};

byte _resetPin;
char *_ssid, *_password;
double _timeZone = 0.0;
unsigned long _sleepTime;
rtcStore rtcMem;
strDateTime _actualTime;
SNTPtime NTPch("ch.pool.ntp.org");


/**************************************************************/
/*
    Constructors
*/
/**************************************************************/
ESPRelayTaskNTP::ESPRelayTaskNTP(int hoursTask1, int minutesTask1, int durationTask1, int hoursTask2, int minutesTask2, int durationTask2, double timeZone, char *mySSID, char *myPASSWORD, byte RESET_PIN) {
  _wakeupTask1.hour = hoursTask1;
  _wakeupTask1.minute = minutesTask1;
  _wakeupTask1.second = durationTask1;
  _wakeupTask2.hour = hoursTask2;
  _wakeupTask2.minute = minutesTask2;
  _wakeupTask2.second = durationTask2;
  _resetPin = RESET_PIN;
  _ssid = mySSID;
  _password = myPASSWORD;
  _timeZone = timeZone;
  if (_resetPin != 99) pinMode(_resetPin, INPUT_PULLUP);
}

ESPRelayTaskNTP::ESPRelayTaskNTP(int hoursTask1, int minutesTask1, int durationTask1, int hoursTask2, int minutesTask2, int durationTask2, double timeZone, char *mySSID, char *myPASSWORD) {

  byte RESET_PIN = 99;

}


/**************************************************************/
/*
    1.- Reinitialize the rtc memory if necessary
    2.- Checks whether the task execution is correct or not
    3.- Sets the amount of time the system is going to deep sleep
*/
/**************************************************************/
void ESPRelayTaskNTP::sleepOneDay() {

  int _secondsToWait;
  unsigned long entry = millis();

#ifdef WORK_WITH_E_PAPER
  displayInitialization();
#endif

  system_rtc_mem_read(65, &rtcMem, sizeof(rtcMem));
  if (/*(_resetPin!=99 && digitalRead(_resetPin)==0 )|| */rtcMem.markerFlag != 72) rtcMem.status = RESET;

  switch (rtcMem.status) {

    case RESET:
      rtcMem.markerFlag = 72;
      rtcMem.counter = 0;
      rtcMem.wifiConnectionFailed = false;
      _sleepTime = MIN_SLEEP_TIME;
      rtcMem.status = CHECK;
      system_rtc_mem_write(65, &rtcMem, sizeof(rtcMem));
      if (WithSerialDebug) {
        printRtcMem("RESET ");
      }

#ifdef WORK_WITH_E_PAPER
      writeLastStatusToDisplay (rtcMem.wakeuptime, statusDefNames[rtcMem.status], rtcMem.counter, _sleepTime / 1e6);
#endif

      ESP.deepSleep(_sleepTime, WAKE_RF_DEFAULT);
      break;

    case COUNTING:
      if (rtcMem.counter == 0) {
        _sleepTime = MIN_SLEEP_TIME;
        rtcMem.status = CHECK;
        system_rtc_mem_write(65, &rtcMem, sizeof(rtcMem));
        if (WithSerialDebug) {
          printRtcMem("COUNTING ZERO ");
        }

#ifdef WORK_WITH_E_PAPER
        writeLastStatusToDisplay (rtcMem.wakeuptime, statusDefNames[rtcMem.status], rtcMem.counter, _sleepTime / 1e6);
#endif

        ESP.deepSleep(_sleepTime, WAKE_RF_DEFAULT);
      }
      else {
        rtcMem.counter--;
        _sleepTime = ONE_HOUR;
        system_rtc_mem_write(65, &rtcMem, sizeof(rtcMem));
        if (WithSerialDebug) {
          printRtcMem("COUNTING DOWN ");
        }

#ifdef WORK_WITH_E_PAPER
        writeLastStatusToDisplay (rtcMem.wakeuptime, statusDefNames[rtcMem.status], rtcMem.counter, _sleepTime / 1e6);
#endif

        ESP.deepSleep(_sleepTime, WAKE_RF_DISABLED);
      }
      break;

    case CHECK:
      _secondsToWait = adjustTime();

      if (_secondsToWait > 10) {

        if (_secondsToWait > 3600) {
          rtcMem.counter = (int)(_secondsToWait / 3600);
          rtcMem.status = CHECK;
          _sleepTime = ONE_HOUR; // - (MAX_WIFI_WAIT_TIME * 1e3);
          system_rtc_mem_write(65, &rtcMem, sizeof(rtcMem));
          if (WithSerialDebug) {
            printRtcMem("CHECKED ");
          }

#ifdef WORK_WITH_E_PAPER
          writeLastStatusToDisplay (rtcMem.wakeuptime, statusDefNames[rtcMem.status], rtcMem.counter, _sleepTime / 1e6);
#endif

          ESP.deepSleep(_sleepTime, WAKE_RF_DISABLED);
        }
        else {
          rtcMem.status = CHECK;
          _sleepTime = _secondsToWait * ONE_SECOND;
          system_rtc_mem_write(65, &rtcMem, sizeof(rtcMem));
          if (WithSerialDebug) {
            printRtcMem("CHECKED AND LAST WAIT FOR WORK ");
          }

#ifdef WORK_WITH_E_PAPER
          writeLastStatusToDisplay (rtcMem.wakeuptime, "DESC.+WiFi", rtcMem.counter, _sleepTime / 1e6);
#endif
          ESP.deepSleep(_sleepTime, WAKE_RF_DEFAULT);
        }
      }
      else {
        rtcMem.status = WORK;
        _sleepTime = MIN_SLEEP_TIME;
        system_rtc_mem_write(65, &rtcMem, sizeof(rtcMem));
        if (WithSerialDebug) {
          printRtcMem("CHECKED 3 ");
        }

#ifdef WORK_WITH_E_PAPER
        writeLastStatusToDisplay (rtcMem.wakeuptime, statusDefNames[rtcMem.status], rtcMem.counter, _sleepTime / 1e6);
#endif

        ESP.deepSleep(_sleepTime, WAKE_RF_DEFAULT);
      }
      break;

    case WORK:
      rtcMem.status = CHECK;
      system_rtc_mem_write(65, &rtcMem, sizeof(rtcMem));
      break;

  }
}



/**************************************************************/
/*
    1.- Establish the wifi connection
    2.- Gets the unixtime from a NTP server and adjusts it to the time zone
    3.- Checks the time tasks and sets the state desired
*/
/**************************************************************/
int ESPRelayTaskNTP::adjustTime() {
  long _currentSecs, _wakeUpSecs;
  int _seconds;
  unsigned long entry = millis();

  WiFi.begin (_ssid, _password);
  while (WiFi.status() != WL_CONNECTED && millis() - entry < MAX_WIFI_WAIT_TIME) {
    if (WithSerialDebug) {
      Serial.print(".");
    }
    delay(500);
  }
  if (millis() - entry > MAX_WIFI_WAIT_TIME) {
    //rtcMem.wifiConnectionFailed = true;
    //system_rtc_mem_write(65, &rtcMem, sizeof(rtcMem));
    ESP.deepSleep(MIN_SLEEP_TIME, WAKE_RF_DEFAULT);  // no connection possible, try again
  }
  if (WithSerialDebug) {
    Serial.println("WiFi connected");
  }

  entry = millis();
  while (!NTPch.setSNTPtime() && millis() - entry < 10000) {
    if (WithSerialDebug) {
      Serial.println("x");
    }
  }

  if (millis() - entry < 10000) {
    rtcMem.wifiConnectionFailed = false;
    _actualTime = NTPch.getTime(_timeZone, true);

#ifdef WORK_WITH_E_PAPER
    rtcMem.wakeuptime = _actualTime;
#endif

    if (WithSerialDebug) {
      NTPch.printDateTime(_actualTime);
    }
    _currentSecs = ((_actualTime.hour * 60) + _actualTime.minute) * 60 + _actualTime.second;

    //Se calcula cuál de las tareas es la siguiente
    //Por defecto se ejecutará siempre sólo la Task1 si (Task2 < Task1)
    _wakeUpSecs = ((_wakeupTask1.hour * 60) + _wakeupTask1.minute) * 60;
    rtcMem.timeTaskOn = _wakeupTask1.second;
    rtcMem.nextTask = 1;
    if (_wakeUpSecs >= _currentSecs) {
    }
    else if (_wakeUpSecs + _wakeupTask1.second >= _currentSecs) {
      rtcMem.timeTaskOn = _wakeupTask1.second; // _wakeUpSecs + _wakeupTask1.second - _currentSecs;
      _wakeUpSecs = _currentSecs + 1;
      rtcMem.nextTask = 1;
    }
    else if (((_wakeupTask2.hour * 60) + _wakeupTask2.minute) * 60 >= _currentSecs) {
      _wakeUpSecs = ((_wakeupTask2.hour * 60) + _wakeupTask2.minute) * 60;
      rtcMem.timeTaskOn = _wakeupTask2.second;
      rtcMem.nextTask = 2;
    }
    else if (((_wakeupTask2.hour * 60) + _wakeupTask2.minute) * 60 + _wakeupTask2.second >= _currentSecs) {
      _wakeUpSecs = ((_wakeupTask2.hour * 60) + _wakeupTask2.minute) * 60;
      rtcMem.timeTaskOn = _wakeupTask2.second; // _wakeUpSecs + _wakeupTask2.second - _currentSecs;
      _wakeUpSecs = _currentSecs + 1;
      rtcMem.nextTask = 2;
    }

    _seconds = (_wakeUpSecs - _currentSecs > 0) ? (_wakeUpSecs - _currentSecs) : (_wakeUpSecs - _currentSecs + (24 * 3600));

    if (WithSerialDebug) {
      Serial.printf("_currentSecs: %3d\r\n", _currentSecs);
      Serial.printf("_wakeUpSecs: %3d\r\n", _wakeUpSecs);
      Serial.printf("_secondsToGo: %3d\r\n", _seconds);
    }
    return _seconds;
  }
  else {
    //rtcMem.wifiConnectionFailed = true;
    //system_rtc_mem_write(65, &rtcMem, sizeof(rtcMem));
    ESP.deepSleep(MIN_SLEEP_TIME, WAKE_RF_DEFAULT);  // Setting NTP time was not successful
  }
}



/**************************************************************/
/*
    1.- Serializes information about the system state
*/
/**************************************************************/
void ESPRelayTaskNTP::printRtcMem(String place) {
  Serial.print(place);
  Serial.print(" ");
  Serial.print("Status:");
  Serial.print(rtcMem.status);
  Serial.print(", markerFlag:");
  Serial.print(rtcMem.markerFlag);
  Serial.print(", counter:");
  Serial.print(rtcMem.counter);
  Serial.print(", sleepTime:");
  Serial.print(_sleepTime);
  Serial.print(", nextTask:");
  Serial.print(rtcMem.nextTask);
  Serial.println();
}


/**************************************************************/
/*
    1.- Restart the system
*/
/**************************************************************/
void ESPRelayTaskNTP::backToSleep() {
  //rtcMem.counter = 23;     //24 hours to sleep
  _sleepTime = MIN_SLEEP_TIME;
  rtcMem.status = CHECK;
  system_rtc_mem_write(65, &rtcMem, sizeof(rtcMem));
  if (WithSerialDebug) {
    printRtcMem("BACK TO SLEEP FOR CHECK ");
  }
  ESP.deepSleep(_sleepTime, WAKE_RF_DEFAULT);
}

unsigned int ESPRelayTaskNTP::readTimeTaskOn() {
  system_rtc_mem_read(65, &rtcMem, sizeof(rtcMem));
  return rtcMem.timeTaskOn;
}
