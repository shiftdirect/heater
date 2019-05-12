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
#include "Clock.h"
#include "BTCDateTime.h"
#include "TimerManager.h"
#include <Wire.h>
#include <RTClib.h>
#include "../Protocol/helpers.h"
#include "../Utility/NVStorage.h"
#include "../Utility/DebugPort.h"


// create ONE of the RTClib supported real time clock classes
#if RTC_USE_DS3231 == 1
RTC_DS3231 rtc;
#elif RTC_USE_DS1307 == 1
RTC_DS1307 rtc;
#elif RTC_USE_PCF8523 == 1
RTC_PCF8523 rtc;
#else
RTC_Millis rtc;
#endif

CClock Clock(rtc);


void 
CClock::begin()
{
  // announce which sort of RTC is being used
#if RTC_USE_DS3231 == 1
DebugPort.println("Using DS3231 Real Time Clock");
#elif RTC_USE_DS1307 == 1
DebugPort.println("Using DS1307 Real Time Clock");
#elif RTC_USE_PCF8523 == 1
DebugPort.println("Using PCF8523 Real Time Clock");
#else
#define SW_RTC    // enable different begin() call for the millis() based RTC
DebugPort.println("Using millis() based psuedo \"Real Time Clock\"");
#endif

#ifdef SW_RTC
  DateTime zero(2019, 1, 1);   // can be pushed along as seen fit!
  _rtc.begin(zero);
#else
  _rtc.begin();
#endif

  _nextRTCfetch = millis();

  CTimerManager::createMap();
  
  update();
}

const BTCDateTime& 
CClock::update()
{
  long deltaT = millis() - _nextRTCfetch;
  if(deltaT >= 0) {
    uint32_t origClock = Wire.getClock();
    Wire.setClock(400000);
    _currentTime = _rtc.now();             // moderate I2C accesses
    Wire.setClock(origClock);
    _nextRTCfetch = millis() + 500;
//    _checkTimers();
    // check timers upon minute rollovers
    if(_currentTime.minute() != _prevMinute) {
      CTimerManager::manageTime(_currentTime.hour(), _currentTime.minute(), _currentTime.dayOfTheWeek());
      _prevMinute = _currentTime.minute();
    }
  }
  return _currentTime;
}

const BTCDateTime& 
CClock::get() const
{
  return _currentTime;
}

void 
CClock::set(const DateTime& newTimeDate)
{
  _rtc.adjust(newTimeDate);
}



void setDateTime(const char* newTime)
{
  DebugPort.printf("setting time to: %s\r\n", newTime);
  int month,day,year,hour,minute,second;
  if(6 == sscanf(newTime, "%d/%d/%d %d:%d:%d", &day, &month, &year, &hour, &minute, &second)) {
    DateTime newDateTime(year, month, day, hour, minute, second);
    Clock.set(newDateTime);
  }
}

void setDate(const char* newDate)
{
  DebugPort.printf("setting date to: %s\r\n", newDate);
  int month,day,year;
  if(3 == sscanf(newDate, "%d/%d/%d", &day, &month, &year)) {
    DateTime currentDateTime = Clock.get();
    DateTime newDateTime(year, month, day, currentDateTime.hour(), currentDateTime.minute(), currentDateTime.second());
    Clock.set(newDateTime);
  }
}

void setTime(const char* newTime)
{
  DebugPort.printf("setting time to: %s\r\n", newTime);
  int hour,minute,second;
  if(3 == sscanf(newTime, "%d:%d:%d", &hour, &minute, &second)) {
    DateTime currentDateTime = Clock.get();
    DateTime newDateTime(currentDateTime.year(), currentDateTime.month(), currentDateTime.day(), hour, minute, second);
    Clock.set(newDateTime);
  }
}

