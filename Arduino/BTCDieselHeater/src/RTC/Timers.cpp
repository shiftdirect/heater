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

#include <Arduino.h>
#include "Timers.h"
#include "../Utility/NVStorage.h"
#include "BTCDateTime.h"


void decodeTimerDays(int ID, const char* str)
{
  sTimer timer;
  NVstore.getTimerInfo(ID, timer);
  unsigned char days = 0;
  if(strstr(str, "Next"))  {
    days = 0x80;
  }
  else {
    for(int i=0; i< 7; i++) {
      int mask = 0x01 << i;
      if(strstr(str, daysOfTheWeek[i]))  
        days |= mask;
    }
  }
  timer.enabled = days;
  NVstore.setTimerInfo(ID, timer);
}


void decodeTimerTime(int ID, int stop, const char* str)
{
  sTimer timer;
  NVstore.getTimerInfo(ID, timer);
  int hour, minute;
  if(2 == sscanf(str, "%d:%d", &hour, &minute)) {
    if(stop) {
      timer.stop.hour = hour;
      timer.stop.min = minute;
    }
    else {
      timer.start.hour = hour;
      timer.start.min = minute;
    }
    NVstore.setTimerInfo(ID, timer);
  }
}


void decodeTimerRepeat(int ID, int state)
{
  sTimer timer;
  NVstore.getTimerInfo(ID, timer);
  timer.repeat = state;
  NVstore.setTimerInfo(ID, timer);
}


const char* getTimerStr(int timer, int param)
{
  sTimer timerInfo;
  // due to how ArduinoJSON builds the JSON string, we need to create and store each string individually here.
  static char StartStr[2][8];
  static char StopStr[2][8];
  static char DayStr[2][32];
  static char RptStr[2][2];

  NVstore.getTimerInfo(timer, timerInfo);
  int i = 0;
  int comma = 0;

  switch(param) {
    case 0:
      sprintf(StartStr[timer], "%02d:%02d", timerInfo.start.hour, timerInfo.start.min);
      return StartStr[timer];
    case 1:
      sprintf(StopStr[timer], "%02d:%02d", timerInfo.stop.hour, timerInfo.stop.min);
      return StopStr[timer];
    case 2:
      if(timerInfo.enabled == 0) {
        strcpy(DayStr[timer], "None");
      }
      else if(timerInfo.enabled & 0x80) {
        strcpy(DayStr[timer], "Next");
      }
      else {
        comma = 0;
        DayStr[timer][0] = 0;
        for(i=0; i<7; i++) {
          if(timerInfo.enabled & (0x01<<i)) {
            if(comma)
              strcat(DayStr[timer], ",");
            strcat(DayStr[timer], daysOfTheWeek[i]);
            comma = 1;
          }
        }
      }
      return DayStr[timer];
    case 3:
      strcpy(RptStr[timer], timerInfo.repeat ? "1" : "0");
      return RptStr[timer];
    default:
      return "BadParam";
  }
}
