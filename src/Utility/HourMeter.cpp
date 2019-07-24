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

#include "HourMeter.h"
#include "NVStorage.h"
#include "../RTC/RTCStore.h"
#include "../RTC/Clock.h"
#include "../Protocol/Protocol.h"
#include "../Utility/NVStorage.h"

const int baseSeconds = 60 * 15;  // 15 minutes


void 
CHourMeter::powerOnInit()
{
  _RunTime = 0;
  _GlowTime = 0;
}

void 
CHourMeter::monitor(const CProtocol& frame)
{
//  long now = Clock.get().secondstime();
  unsigned long now = millis();
  if(frame.getRunState() != 0) {
    if(_lastRunTime != 0) {
      float tDelta = float(now - _lastRunTime) * 0.001;
      _RunTime += tDelta;
      if(frame.getGlowPlug_Voltage() != 0) {
        if(_lastGlowTime != 0) {
          _GlowTime += tDelta;
        }    
        _lastGlowTime = now;
      }
      else {
        _lastGlowTime = 0;
      }
    }
    _lastRunTime = now;
    sHourMeter FLASHmem = NVstore.getHourMeter();
    if(_RunTime > baseSeconds) {
      if(RTC_Store.incRunTime())
        FLASHmem.RunTime++;
      _RunTime -= baseSeconds;
    }
    if(_GlowTime > baseSeconds) {
      if(RTC_Store.incGlowTime())
        FLASHmem.GlowTime++;
      _GlowTime -= baseSeconds;
    }
    NVstore.setHourMeter(FLASHmem);
  }
  else {
    _lastRunTime = 0;
    _lastGlowTime = 0;
  }
    DebugPort.printf("CHourMeter %f %f\r\n", _RunTime, _GlowTime);
}

uint32_t 
CHourMeter::getRunTime()
{
  uint32_t rt = (uint32_t)_RunTime;
  DebugPort.printf("HrMtr: %d %d %d\r\n", rt, RTC_Store.getRunTime(), NVstore.getHourMeter().RunTime);

  return  rt + baseSeconds * RTC_Store.getRunTime() + baseSeconds * RTC_Store.getMaxRunTime() * NVstore.getHourMeter().RunTime;
}

uint32_t 
CHourMeter::getGlowTime()
{
  uint32_t gt = (uint32_t)_GlowTime;
  return gt +  baseSeconds * RTC_Store.getGlowTime()  +  baseSeconds * RTC_Store.getMaxGlowTime() * NVstore.getHourMeter().GlowTime;
}

