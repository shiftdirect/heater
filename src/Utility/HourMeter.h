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

class CProtocol;

class CHourMeter {
  const int baseSeconds = 60 * 15;  // 15 minutes

  float& _RunTime;
  float& _GlowTime;
  unsigned long _lastRunTime;
  unsigned long _lastGlowTime;
  uint32_t _getLclRunTime();    // volatile persistent variable + RTC stored rollovers
  uint32_t _getLclGlowTime();   // volatile persistent variable + RTC stored rollovers
public:
  CHourMeter(float &runtime, float& glowtime) : 
    _RunTime(runtime), 
    _GlowTime(glowtime) 
  {
    _lastRunTime = 0;
    _lastGlowTime = 0;
    DebugPort.printf("CHourMeter %f %f\r\n", _RunTime, _GlowTime);
  };
  void associate(float &runtime, float& glowtime) {
    _RunTime = runtime; 
    _GlowTime = glowtime; 
  };
  void init(bool poweron);
  void reset();
  void store();                // transfer current state to permanent NV storage
  void monitor(const CProtocol& frame);
  uint32_t getRunTime();       // total time, local tracked + last NV stored value
  uint32_t getGlowTime();      // total time, local tracked + last NV stored value
};

extern CHourMeter* pHourMeter;