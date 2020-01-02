# ROUSGARDEN
## Autonomous Watering System


### MAIN GOALS
This system is designed for the **autonomous** and **unattended** irrigation of a small terrace or balcony garden. It can supply water up to 7\~8 plants with a maximum distance of 10\~12 meters. The main goal of the project is to have an unattended system for a complete emptying cycle of a water tank of about 50 liters in 20\~25 days.


### COMPONENTS
##### Controller
The main controller of the system is a Wemos D1 Mini board (ESP8266) that works in "deep sleep" mode for maximum energy savings. The board connects to the WiFi network and gets the current time each time the system wake up. The system acts accordingly by "sleeping" again or executing the irrigation task scheduled.

##### Energy Supply
There is a 4000 mAh Lithium battery that has two charging systems:
  - solar panels
  - external power supply
  
The case has three possible angular supports for maximum use of solar radiation depending on the season of the year. The maximum duration tested without external power supply has been 25 days. :grinning:

##### Pump & Pipes
The water pump is a 5V DC / 2.4W / 250L / 2m water lift and submersible. Pipes used in this project are 4 mm in diameter and there is an adaptor between the pump outlet and the pipe. For the correct water distribution it is advisable to make holes to the pipes following the next scheme:
```
         ├─────────────── 10~12m ───────────────┤
   [PUMP]════╦═══════╦═══════╦═...══════╦═══════╦
hole        #1      #2      #3 ...     #7      #8
 Ø         0.5mm   0.5mm    1mm       2.5mm   2.5mm
```
