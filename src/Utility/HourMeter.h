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
  int& _RunTime;
  int& _GlowTime;
public:
  CHourMeter(int &runtime, int& glowtime) : 
    _RunTime(runtime), 
    _GlowTime(glowtime) 
  {};
  void associate(int &runtime, int& glowtime) {
    _RunTime = runtime; 
    _GlowTime = glowtime; 
  };
  void powerOnInit();
  void monitor(const CProtocol& frame);
  unsigned long getRunTime();
  unsigned long getGlowTime();
};

extern CHourMeter* pHourMeter;