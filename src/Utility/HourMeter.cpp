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



void 
CHourMeter::init(bool poweron)
{
  if(poweron) {
    _RunTime = 0;  // definitely untrustworthy after power on or OTA updates
    _GlowTime = 0;
  }
  if(_RunTime || _GlowTime || RTC_Store.getRunTime() || RTC_Store.getGlowTime()) {
    store();
  }
}

void
CHourMeter::reset()
{
  RTC_Store.resetRunTime();
  RTC_Store.resetGlowTime();
  _RunTime = 0;
  _GlowTime = 0;
}

void
CHourMeter::store()
{
  sHourMeter NV = NVstore.getHourMeter();
  NV.RunTime += _RunTime + baseSeconds * RTC_Store.getRunTime();    // add any residual to the real NV stored value
  NV.GlowTime += _GlowTime + baseSeconds * RTC_Store.getGlowTime();
  NVstore.setHourMeter(NV);
  reset();
}

void 
CHourMeter::monitor(const CProtocol& frame)
{
//  long now = Clock.get().secondstime();
  unsigned long now = millis();
  if(frame.getRunState() == 0) {
    // heater is stopped - save remnant times to flash
    if(_lastRunTime) {
      store();   // heater hass stopped - save remnant times to flash
    }
    _lastRunTime = 0;
    _lastGlowTime = 0;
  }
  else {
    // heater is running
    if(_lastRunTime != 0) {
      // first sample after start is ignored
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
    sHourMeter NV = NVstore.getHourMeter();
    // test for rollover of our local run time tracking
    if(_RunTime > baseSeconds) {
      _RunTime -= baseSeconds;
      if(RTC_Store.incRunTime()) {       // returns true if rolled back to zero
        // rolled RTC intermediate store - push into FLASH
        NV.RunTime += baseSeconds * RTC_Store.getMaxRunTime();   // bump by our maximum storable time
      }
    }
    // test for rollover of our local glow time tracking
    if(_GlowTime > baseSeconds) {
      _GlowTime -= baseSeconds;
      if(RTC_Store.incGlowTime()) {       // returns true if rolled back to zero
        // rolled RTC intermediate store - push into FLASH
        NV.GlowTime += baseSeconds * RTC_Store.getMaxGlowTime();  // bump by our maximum storable time
      }
    }
    NVstore.setHourMeter(NV);  // internally moderated, only actually saves if values have changed
  }
//    DebugPort.printf("CHourMeter %f %f\r\n", _RunTime, _GlowTime);
}

uint32_t
CHourMeter::_getLclRunTime()
{
  uint32_t rt = (uint32_t)_RunTime;
  DebugPort.printf("HrMtr _GetLclRunTime(): %d %d\r\n", rt, RTC_Store.getRunTime());
  return  rt + baseSeconds * RTC_Store.getRunTime();
}

uint32_t
CHourMeter::_getLclGlowTime()
{
  uint32_t gt = (uint32_t)_GlowTime;
  DebugPort.printf("HrMtr _GetLclGlowTime(): %d %d\r\n", gt, RTC_Store.getGlowTime());
  return  gt + baseSeconds * RTC_Store.getGlowTime();
}

uint32_t 
CHourMeter::getRunTime()
{
  uint32_t rt = _getLclRunTime();
  DebugPort.printf("HrMtr GetRunTime(): %d %d\r\n", rt, NVstore.getHourMeter().RunTime);
  return  rt + NVstore.getHourMeter().RunTime;
}

uint32_t 
CHourMeter::getGlowTime()
{
  uint32_t gt = _getLclGlowTime();
  DebugPort.printf("HrMtr GetGlowTime(): %d %d\r\n", gt, NVstore.getHourMeter().RunTime);
  return gt + NVstore.getHourMeter().GlowTime;
}

