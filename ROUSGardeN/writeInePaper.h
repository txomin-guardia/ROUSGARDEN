#ifndef _WRITE_IN_E_PAPER_H
  #define _WRITE_IN_E_PAPER_H

  // E-INK DISPLAY
  // include library, include base class, make path known
  #include <GxEPD.h>
  // the display class to use
  #include <GxGDEW0154Z04/GxGDEW0154Z04.h>  // 1.54" b/w/r 200x200
  // FreeFonts from Adafruit_GFX
  #include <Fonts/FreeMonoBold9pt7b.h>
  #include <Fonts/FreeMonoBold12pt7b.h>
  #include <Fonts/FreeMonoBold18pt7b.h>
  #include <Fonts/FreeMonoBold24pt7b.h>
  #include <GxIO/GxIO_SPI/GxIO_SPI.h>
  #include <GxIO/GxIO.h>
  #include "ESPRelayTaskNTP.h"
  #if defined(ESP8266)
    // for SPI pin definitions see e.g.:
    // C:\Users\xxx\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.4.2\variants\generic\common.h
    // BUSY -> D2, RST -> D4, DC -> D3, CS -> RX, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V
    // He tenido que usar el pin RX para la función CS porque el pin que viene por defecto para esa función (D8) me daba problemas al resetear el micro.
    // Parece que el D8 se usa en el arranque del micro para comprobar si se entra en modo programación, y la pantalla lo pone en ese momento a nivel
    // para que el micro se quede esperando.
    static GxIO_Class io(SPI, /*CS=RX*/ 3, /*DC=D3*/ 0, /*RST=D4*/ 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
    static GxEPD_Class display(io, /*RST=D4*/ 2, /*BUSY=D2*/ 4); // default selection of D4(=2), D2(=4)
    // Heltec E-Paper 1.54" b/w without RST, BUSY
    //GxEPD_Class display(io, /*RST=D4*/ -1, /*BUSY=D2*/ -1); // no RST, no BUSY
  #endif
 
  extern void displayInitialization(void);
  extern void writeLastDateUpdateToDisplay(const char *text);
  extern void writeLastStatusToDisplay(const strDateTime wakeuptime ,const char *text ,const byte count, const float sleeptime);

#endif // _WRITE_IN_E_PAPER_H
