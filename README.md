# ROUSGARDEN
Autonomous Watering System

This system is designed for the autonomous irrigation of a small terrace or balcony garden. It can supply water up to 7\~8 plants with a maximum distance of 10\~12 meters. The main goal of the project is to have an unattended system for a complete emptying cycle of a water tank of about 50 liters in 20\~25 days.

The main controller of the system is a Wemos D1 Mini board (ESP8266) that works in "deep sleep" mode for maximum energy savings. The board connects to the WiFi network and gets the current time each time the system wake up. The system acts accordingly by "sleeping" again or executing the irrigation task scheduled.

The lithium battery has two charging systems: by solar panels and by external power supply. The case has three possible angular supports for maximum use of solar radiation depending on the season of the year. The maximum duration tested without external power supply has been 25 days.
