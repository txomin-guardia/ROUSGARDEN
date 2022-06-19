/*
  ESPrelayTask library for ESP8266


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

#pragma once

#include <Arduino.h>
#include <SNTPtime.h>


//#define WORK_WITH_E_PAPER //Uncomment if you want to use an e-paper display
#ifdef WORK_WITH_E_PAPER
  #include "writeInePaper.h"
#endif


class ESPRelayTaskNTP {

  public:
    strDateTime _wakeupTask1, _wakeupTask2;
    ESPRelayTaskNTP(int hoursTask1, int minutesTask1, int durationTask1, int hoursTask2, int minutesTask2, int durationTask2, double timeZone, char *mySSID, char *myPASSWORD);
    ESPRelayTaskNTP(int hoursTask1, int minutesTask1, int durationTask1, int hoursTask2, int minutesTask2, int durationTask2, double timeZone, char *mySSID, char *myPASSWORD, byte RESET_PIN);

    void sleepOneDay();
    void backToSleep();
    unsigned int readTimeTaskOn();

  private:
    void printRtcMem(String place);
    int adjustTime();
};
