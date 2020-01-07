/* ROUSGarden

   Autonomous and Unattended Watering System, ESP8266 controlled with two daily tasks

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

#include <ESP8266WiFi.h>
#include <CayenneMQTTESP8266.h>
#include "credentials.h"
#include "ESPRelayTaskNTP.h"

extern "C" {
#include "user_interface.h" // this is for the RTC memory read/write functions
}

// WEMOS_D1_MINI used pins to control relay
#define RESET_PIN 12  //It corresponds to WEMOS D1 MINI - pin D6
#define RELAY_ON_PIN 5  //It corresponds to WEMOS D1 MINI - pin D1

//Working with Cayenne
#define CAYENNE_PRINT Serial

//On & Off enumeration
enum OnOffOption {
  Off,
  On
};

// Debugging through serial port
const OnOffOption WithSerialDebug = Off;
// Debugging through Cayenne IoT Project Builder
const OnOffOption WithCayenneDebug = On;

// This is the instatiation of the main class that controls the relay
// Usage:   relayTask(Task #1 Hour, Task #1 Minute, Task #1 Time On (seg), Task #2 Hour, Task #2 Minute, Task #2 Time On (seg), Timezone, WiFi SSID, WiFi PASSWORD, Reset Pin);
// Only Task 1 will be executed if the hour and minute of Task 2 is less than that of Task 1.
ESPRelayTaskNTP relayTask(0, 0, 0, 0, 0, 0, 1.0, my_SSID, my_PASSWORD, RESET_PIN);

void setup() {
  unsigned int timeTaskOn;

  if (WithSerialDebug) {
    Serial.begin(115200);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB
    }
    Serial.println();   Serial.println("Start");   Serial.println();
  }

  //Relay task schedule configuration
  relayTask._wakeupTask1.hour = 8; //Hour of the day
  relayTask._wakeupTask1.minute = 00; //Minute of the day
  relayTask._wakeupTask1.second = 90; //Duration in sec
  relayTask._wakeupTask2.hour = 8; //Hour of the day
  relayTask._wakeupTask2.minute = 00; //Minute of the day
  relayTask._wakeupTask2.second = 90; //Duration in sec

  relayTask.sleepOneDay();

  // ------------------ put the code for the tasks here -------------------------------

  if (WithCayenneDebug) {
    // Initialize Cayenne connection also activating the wifi because Wi-Fi is not
    // started by default when the job is activated.
    Cayenne.begin(username, password, clientID, my_SSID, my_PASSWORD);
    // Inform Cayenne the ON state
    Cayenne.digitalSensorWrite(0, true);
  }

  timeTaskOn = relayTask.readTimeTaskOn();

  if (WithSerialDebug) {
    Serial.println("............ W O R K I N G ............");
    Serial.print("Task in progress for ");
    Serial.print(timeTaskOn);
    Serial.println("s");
  }

  //Relay switch ON
  pinMode(RELAY_ON_PIN, OUTPUT);
  digitalWrite(RELAY_ON_PIN, true);

  delay(timeTaskOn * 1e3);

  digitalWrite(RELAY_ON_PIN, false);

  if (WithCayenneDebug) {
    // Inform Cayenne the OFF state
    Cayenne.digitalSensorWrite(0, false);
    delay(1000);
  }
  
  relayTask.backToSleep();
}

void loop() {
  // loop never reached
}
