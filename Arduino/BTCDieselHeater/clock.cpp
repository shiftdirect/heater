/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2018  Ray Jones <ray@mrjones.id.au>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */

#include "time.h"
#include <sys/time.h>                   /* gettimeofday(), settimeofday() */
#include "stdint.h"
#include "128x64OLED.h"
#include "Screen.h"
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
  {
    CTransientFont AF(display, &franklinGothicMediumCond_8ptFontInfo);
    display.setCursor(display.xCentre(), 0);
    display.printCentreJustified(msg);
  }
}
