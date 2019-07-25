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
#include "../Utility/macros.h"
#include "BTCDateTime.h"


void decodeJSONTimerDays(const char* ipStr)
{
  char dayInfo[32];
  int timerIdx;
  if(2 == sscanf(ipStr, "%d %31s", &timerIdx, dayInfo)) {
    dayInfo[31] = 0;
    timerIdx--;
    if(INBOUNDS(timerIdx, 0, 13)) {
      sTimer timer;
      NVstore.getTimerInfo(timerIdx, timer);
      uint8_t days = 0;
      if(strstr(dayInfo, "Next"))  {
        days = 0x80;
      }
      else {
        for(int i=0; i< 7; i++) {
          int mask = 0x01 << i;
          if(strstr(dayInfo, daysOfTheWeek[i]))  
            days |= mask;
        }
      }
      timer.enabled = days;
      NVstore.setTimerInfo(timerIdx, timer);
    }
  }
}


void decodeJSONTimerTime(int stop, const char* ipStr)
{
  int hour, min;
  int timerIdx;
  if(3 == sscanf(ipStr, "%d %d:%d", &timerIdx, &hour, &min)) {
    timerIdx--;
    if(INBOUNDS(timerIdx, 0, 13)) {
      sTimer timer;
      NVstore.getTimerInfo(timerIdx, timer);
      if(stop) {
        timer.stop.hour = hour;
        timer.stop.min = min;
      }
      else {
        timer.start.hour = hour;
        timer.start.min = min;
      }
      NVstore.setTimerInfo(timerIdx, timer);
    }
  }
}

void decodeJSONTimerNumeric(int valID, const char* ipStr)
{
  int value;
  int timerIdx;
  if(2 == sscanf(ipStr, "%d %d", &timerIdx, &value)) {
    timerIdx--;
    if(INBOUNDS(timerIdx, 0, 13)) {
      sTimer timer;
      NVstore.getTimerInfo(timerIdx, timer);
      switch(valID) {
        case 0: timer.repeat = value; break;
        case 1: timer.temperature = value; break;
      }
      NVstore.setTimerInfo(timerIdx, timer);
    }
  }
}

void decodeTimerTemp(const char* ipStr)
{
  int degC;
  int timerIdx;
  if(2 == sscanf(ipStr, "%d %d", &timerIdx, &degC)) {
    timerIdx--;
    if(INBOUNDS(timerIdx, 0, 13)) {
      sTimer timer;
      NVstore.getTimerInfo(timerIdx, timer);
      timer.temperature = degC;
      NVstore.setTimerInfo(timerIdx, timer);
    }
  }
}


const char* getTimerJSONStr(int timer, int param)
{
  sTimer timerInfo;
  // due to how ArduinoJSON builds the JSON string, we need to create and store each string individually here.
  static char StartStr[10];
  static char StopStr[10];
  static char DayStr[32];
  static char RptStr[8];
  static char TmpStr[8];

  NVstore.getTimerInfo(timer, timerInfo);
  int i = 0;
  int comma = 0;

  switch(param) {
    case 0:
      sprintf(StartStr, "%d %02d:%02d", timer+1, timerInfo.start.hour, timerInfo.start.min);
      return StartStr;
    case 1:
      sprintf(StopStr, "%d %02d:%02d", timer+1, timerInfo.stop.hour, timerInfo.stop.min);
      return StopStr;
    case 2:
      if(timerInfo.enabled == 0) {                   // timer disabled
        sprintf(DayStr, "%d None", timer+1);
      }
      else if(timerInfo.enabled & 0x80) {            // only for next occurrence
        sprintf(DayStr, "%d Next", timer+1);
      }
      else {
        comma = 0;
        sprintf(DayStr, "%d ", timer+1); 
        for(i=0; i<7; i++) {
          if(timerInfo.enabled & (0x01<<i)) {
            if(comma)
              strcat(DayStr, ",");
            strcat(DayStr, daysOfTheWeek[i]);        // show active days
            comma = 1;
          }
        }
      }
      return DayStr;
    case 3:
      sprintf(RptStr, "%d %s", timer+1, timerInfo.repeat ? "1" : "0");
      return RptStr;
    case 4:
      sprintf(TmpStr, "%d %d", timer+1, timerInfo.temperature);
      return TmpStr;
    default:
      return "BadParam";
  }
}
