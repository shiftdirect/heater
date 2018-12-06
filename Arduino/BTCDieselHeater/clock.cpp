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
#include "RTClib.h"

extern RTC_DS3231 rtc;


void showTime(C128x64_OLED& display)
{
  DateTime now = rtc.now();

  char msg[16];
  if(now.second() & 0x01)
    sprintf(msg, "%02d:%02d", now.hour(), now.minute());
  else
    sprintf(msg, "%02d %02d", now.hour(), now.minute());
  {
    CTransientFont AF(display, &franklinGothicMediumCond_8ptFontInfo);
    display.setCursor(display.xCentre(), 0);
    display.printCentreJustified(msg);
  }
}
