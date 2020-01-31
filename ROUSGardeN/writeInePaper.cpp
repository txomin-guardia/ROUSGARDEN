#include "writeInePaper.h"

void displayInitialization()
{
  display.init(115200);
}

void writeLastDateUpdateToDisplay(const char *text)
{
  const GFXfont* f = &FreeMonoBold9pt7b;
  display.fillScreen(GxEPD_WHITE);
#if defined(HAS_RED_COLOR)
  display.setTextColor(GxEPD_RED);
#else
  display.setTextColor(GxEPD_BLACK);
#endif
  display.setFont(f);
  display.setCursor(0, 0);
  display.println();
  display.println(text);
  display.update();
}

void writeLastStatusToDisplay(const strDateTime wakeuptime ,const char *text ,const byte count, const float sleeptime)
{
  char _dummyString[200]="";
  const GFXfont* f = &FreeMonoBold9pt7b;
  display.fillScreen(GxEPD_WHITE);
  display.setFont(f);
  display.setCursor(0, 0);
  display.println();
  //Estado
  display.setTextColor(GxEPD_BLACK);
  display.print("Estado: ");
  display.setTextColor(GxEPD_RED);
  sprintf (_dummyString, "%s", text);
  display.println(_dummyString);
  //Contaje
  display.setTextColor(GxEPD_BLACK);
  display.print("Faltan: ");
  display.setTextColor(GxEPD_RED);
  sprintf (_dummyString, "%02u horas",count);
  display.println(_dummyString);
  //Tiempo durmiendo
  display.setTextColor(GxEPD_BLACK);
  display.print("Durmiendo:");
  display.setTextColor(GxEPD_RED);
  sprintf (_dummyString, "%5.1f s",sleeptime);
  display.println(_dummyString);
  //Fecha última actualización
  display.setTextColor(GxEPD_BLACK);
  display.println("Ult. Actu. Fecha:");
  display.setTextColor(GxEPD_RED);
  sprintf (_dummyString, "%02u-%02u-%04u\r\n%02u:%02u:%02u",wakeuptime.day,wakeuptime.month,wakeuptime.year,wakeuptime.hour,wakeuptime.minute,wakeuptime.second);
  display.println(_dummyString);
  display.update();
}
