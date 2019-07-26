/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2019  Ray Jones <ray@mrjones.id.au>
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

#include <stdint.h>
#include "../RTC/RTCStore.h"
#include "NVStorage.h"

#define DEBUG_HOURMETER


class CProtocol;

class sRunTime {
  float& persistentVal;
  unsigned long lastSampleTime;
public:
  sRunTime(float& refVal) : persistentVal(refVal) { 
    lastSampleTime = 0; 
  };
  void reset() {
    persistentVal = 0;
  };
  uint32_t get() const {
    return (uint32_t)persistentVal;
  }
  bool active() const {
    return lastSampleTime != 0;
  }
  void stop() {
    lastSampleTime = 0;
  }
  float recordTime(unsigned long now) {
    float rVal = 0;
    if(lastSampleTime)
      rVal = float((unsigned long)(now - lastSampleTime)) * 0.001;
    lastSampleTime = now;
    persistentVal += rVal;
    return rVal;
  } 
  void offset(float ofs) {
    persistentVal += ofs;
  }

};

class CHourMeter {
  const int RTC_storageInterval = 60 * 10;  // 10 minutes

  sRunTime RunTime;
  sRunTime GlowTime;
  uint32_t _getLclRunTime();    // volatile persistent variable + RTC stored rollovers
  uint32_t _getLclGlowTime();   // volatile persistent variable + RTC stored rollovers
public:
  CHourMeter(float &runtime, float& glowtime) : 
    RunTime(runtime),
    GlowTime(glowtime) 
  {
#ifdef DEBUG_HOURMETER
    DebugPort.printf("CHourMeter %d %d\r\n", RunTime.get(), GlowTime.get());
#endif
  };
  void init(bool poweron);
  void reset();
  void store();                // transfer current state to permanent NV storage
  void monitor(const CProtocol& frame);
  uint32_t getRunTime();       // total time, local tracked + last NV stored value
  uint32_t getGlowTime();      // total time, local tracked + last NV stored value
  void resetHard();
};

extern CHourMeter* pHourMeter;