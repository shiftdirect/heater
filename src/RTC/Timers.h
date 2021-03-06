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

#include "../Utility/NVCore.h"
#include "../Utility/macros.h"


struct sHourMin {
  int8_t hour;
  int8_t min;
  sHourMin() {
    hour = 0;
    min = 0;
  }
  sHourMin& operator=(const sHourMin& rhs) {
    hour = rhs.hour;
    min = rhs.min;
    return *this;
  }
  bool operator!=(const sHourMin& rhs) {
    return (hour != rhs.hour) || (min != rhs.min);
  }
};

struct sTimer : public CESP32_NVStorage {
  sHourMin start;      // start time
  sHourMin stop;       // stop time
  uint8_t enabled;     // timer enabled - each bit is a day of week flag
  uint8_t repeat;      // repeating timer
  uint8_t temperature;
  uint8_t timerID;     // numeric ID
  sTimer() {
    enabled = 0;     
    repeat = false;
    temperature = 22;
    timerID = 0;
  }
  sTimer& operator=(const sTimer& rhs) {
    start = rhs.start;
    stop = rhs.stop;
    enabled = rhs.enabled;
    repeat = rhs.repeat;
    temperature = rhs.temperature;
    timerID = rhs.timerID;
    return *this;
  }
  void init(int idx) {
    start.hour = 0;
    start.min = 0;
    stop.hour = 0;
    stop.min = 0;
    enabled = 0;
    repeat = 0;
    temperature = 22;
    timerID = idx;
  }
  bool valid() {
    bool retval = true;
    retval &= INBOUNDS(start.hour, 0, 23);
    retval &= INBOUNDS(start.min, 0, 59);
    retval &= INBOUNDS(stop.hour, 0, 23);
    retval &= INBOUNDS(stop.min, 0, 59);
    retval &= repeat <= 2;
    retval &= INBOUNDS(temperature, 8, 35);
    return retval;
  }
  void load();
  void save();
};

const char* getTimerJSONStr(int timer, int param);
void decodeJSONTimerDays(const char* str);
void decodeJSONTimerTime(int stop, const char*);
void decodeJSONTimerNumeric(int repeat, const char*);

#endif
