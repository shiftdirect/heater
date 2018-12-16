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

#ifndef __BTC_TIMERS_H__
#define __BTC_TIMERS_H__

#include "BTCDateTime.h"
#include "../cfg/BTCConfig.h"


class CClock {
  // allow use of ONE of the RTClib supported RTC chips
  // reference to the selected rtc stored here
#if RTC_USE_DS3231 == 1
  RTC_DS3231& _rtc;
#elif RTC_USE_DS1307 == 1
  RTC_DS1307& _rtc;
#elif RTC_USE_PCF8523 == 1
  RTC_PCF8523& _rtc;
#else
  RTC_Millis& _rtc;
#endif
  unsigned long _nextRTCfetch;
  BTCDateTime _currentTime;

  void _checkTimers();
  void _checkTimer(int timer, const DateTime& now);
public:
  // constructors for ONE of the RTClib supported RTC chips
#if RTC_USE_DS3231 == 1
  CClock(RTC_DS3231& rtc) : _rtc(rtc) {};
#elif RTC_USE_DS1307 == 1
  CClock(RTC_1307& rtc) : _rtc(rtc) {};
#elif RTC_USE_PCF8523 == 1
  CClock(RTC_PCF8523& rtc) : _rtc(rtc) {};
#else
  CClock(RTC_Millis& rtc) : _rtc(rtc) {};
#endif
  void begin();
  const BTCDateTime& update();
  const BTCDateTime& get() const;
  void set(const DateTime& newTime);
};

extern CClock Clock;

#endif // __BTC_TIMERS_H__
