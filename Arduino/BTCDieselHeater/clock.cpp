#include "time.h"
#include <sys/time.h>                   /* gettimeofday(), settimeofday() */
#include "stdint.h"
#include "128x64OLED.h"
#include "display.h"
#include "OCRfont.h"
#include "FranklinGothic.h"

time_t now()
{
struct timeval tv = { .tv_sec = 0, .tv_usec = 0 };   /* btw settimeofday() is helpfull here too*/
// uint64_t sec, us;
uint32_t sec, us;
         gettimeofday(&tv, NULL); 
         (sec) = tv.tv_sec;  
         (us) = tv.tv_usec; 

return sec;
}


void showTime(C128x64_OLED& display)
{
  return;   // till we sort out a proper RTC - the ESP32 SUCKS

  struct tm timeinfo;
  time_t timenow = time(NULL);
  gmtime_r(&timenow, &timeinfo);

  char msg[16];
  if(timeinfo.tm_sec & 0x01)
    sprintf(msg, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  else
    sprintf(msg, "%02d %02d", timeinfo.tm_hour, timeinfo.tm_min);
  display.setFontInfo(&franklinGothicMediumCond_8ptFontInfo);

  display.setCursor(display.xCentre(), 0);
  display.printCentreJustified(msg);
  display.setFontInfo(NULL);
}
