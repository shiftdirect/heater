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
    _checkTimers();
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

void 
CClock::_checkTimers()
{
  _checkTimer(0, _currentTime);   // test timer 1
  _checkTimer(1, _currentTime);   // test timer 2
}

void 
CClock::_checkTimer(int timer, const DateTime& now)
{
  sTimer Info;
  NVstore.getTimerInfo(timer, Info);
  int DOW = now.dayOfTheWeek();
  int timeNow = now.hour() * 60 + now.minute();
  int timeStart = Info.start.hour * 60 + Info.start.min;
  int timeStop = Info.stop.hour * 60 + Info.stop.min;

  // ensure DOW tracks expected start day should timer straddle midnight
  if(timeStop < timeStart) {   // true if stop is next morning
    if(timeNow <= timeStop) {  // current time has passed midnight - enable flag is based upon prior day
      DOW--;
      ROLLLOWERLIMIT(DOW, 0, 6);   // fixup for saturday night!
    }
  }
  // DOW of week is now correct for the day this timer started
  int maskDOW = 0x01 << DOW;

  if(Info.enabled & (maskDOW | 0x80) ) {  // specific day of week, or next day
    
    if(timeNow == timeStart && now.second() < 3) {  // check start, within 2 seconds of the minute rollover
      requestOn();
    }
    
    if(timeNow == timeStop) {            // check stop
      requestOff();
      if(!Info.repeat) {            // cancel timer if non repeating
        if(Info.enabled & 0x80)     // next day start flag set?
          Info.enabled = 0;         // outright cancel
        else {
          Info.enabled &= ~maskDOW; // otherwise clear specific day
        }
        NVstore.setTimerInfo(timer, Info);
        NVstore.save();
      }
    }
  }
}

void setDateTime(const char* newTime)
{
  DebugPort.print("setting time to: ");  DebugPort.println(newTime);
  int month,day,year,hour,minute,second;
  if(6 == sscanf(newTime, "%d/%d/%d %d:%d:%d", &day, &month, &year, &hour, &minute, &second)) {
    DateTime newDateTime(year, month, day, hour, minute, second);
    Clock.set(newDateTime);
  }
}

void setDate(const char* newDate)
{
  DebugPort.print("setting date to: ");  DebugPort.println(newDate);
  int month,day,year;
  if(3 == sscanf(newDate, "%d/%d/%d", &day, &month, &year)) {
    DateTime currentDateTime = Clock.get();
    DateTime newDateTime(year, month, day, currentDateTime.hour(), currentDateTime.minute(), currentDateTime.second());
    Clock.set(newDateTime);
  }
}

void setTime(const char* newTime)
{
  DebugPort.print("setting time to: ");  DebugPort.println(newTime);
  int hour,minute,second;
  if(3 == sscanf(newTime, "%d:%d:%d", &hour, &minute, &second)) {
    DateTime currentDateTime = Clock.get();
    DateTime newDateTime(currentDateTime.year(), currentDateTime.month(), currentDateTime.day(), hour, minute, second);
    Clock.set(newDateTime);
  }
}

