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

#define DEBUG_HOURMETER


void 
CHourMeter::init(bool poweron)
{
  if(!INBOUNDS(RunTime.get(), 0, RTC_storageInterval)) {
    DebugPort.printf("CHourMeter::Init - resetting rogue run value %d\r\n", RunTime.get());
    RunTime.reset();
  }
  if(!INBOUNDS(GlowTime.get(), 0, RTC_storageInterval)) {
    DebugPort.printf("CHourMeter::Init - resetting rogue glow value %d\r\n", GlowTime.get());
    GlowTime.reset();
  }

  // power on reset or OTA update - cannot trust persistent values - they are likely un-initialised
  if(poweron) {       
    RunTime.reset();  
    GlowTime.reset();
  }

  // after all that, if there is a remnant time held, add it to the real NV stored value
  if(RunTime.get() || GlowTime.get() || RTC_Store.getRunTime() || RTC_Store.getGlowTime()) {
    store();
  }
}

void
CHourMeter::reset()
{
  RTC_Store.resetRunTime();
  RTC_Store.resetGlowTime();
  RunTime.reset();
  GlowTime.reset();
}

void
CHourMeter::resetHard()
{
  reset();
  sHourMeter NV = NVstore.getHourMeter();
  NV.RunTime = 0;
  NV.GlowTime = 0;
  NVstore.setHourMeter(NV);
}

void
CHourMeter::store()
{
  sHourMeter NV = NVstore.getHourMeter();
  NV.RunTime += RunTime.get() + RTC_storageInterval * RTC_Store.getRunTime();    // add any residual to the real NV stored value
  NV.GlowTime += GlowTime.get() + RTC_storageInterval * RTC_Store.getGlowTime();
  NVstore.setHourMeter(NV);  // stage new values, and setup for save (if changed)
  reset();                   // zero time tracked in this class
}

void 
CHourMeter::monitor(const CProtocol& frame)
{
  if(frame.getRunState() == 0) {
    // heater is stopped 
    if(RunTime.active()) {
      store();   // initial stop of heater - save residual times to NV
    }
    RunTime.stop();   // cancel time tracking
    GlowTime.stop();
  }
  else {
    // heater is running
    unsigned long now = millis();
    RunTime.recordTime(now);   // track run time of heater
    if(frame.getGlowPlug_Voltage() != 0) {
      GlowTime.recordTime(now);  // track on time of glow plug
    }
    else {
      GlowTime.stop();
    }
    // check for RAM counters time interval rollover
    sHourMeter NV = NVstore.getHourMeter();
    // rollover run time tracking?
    if(RunTime.get() > RTC_storageInterval) {
      RunTime.offset(-RTC_storageInterval);
      if(RTC_Store.incRunTime()) {       // returns true if RTC counter rolled back to zero
        // rolled RTC intermediate store - push into FLASH
        NV.RunTime += RTC_storageInterval * RTC_Store.getMaxRunTime();   // bump NV by our maximum storable time
      }
    }
    // rollover glow time tracking?
    if(GlowTime.get() > RTC_storageInterval) {
      GlowTime.offset(-RTC_storageInterval);
      if(RTC_Store.incGlowTime()) {       // returns true if rolled back to zero
        // rolled RTC intermediate store - push into FLASH
        NV.GlowTime += RTC_storageInterval * RTC_Store.getMaxGlowTime();  // bump NV by our maximum storable time
      }
    }
    NVstore.setHourMeter(NV);  // internally moderated, will only actually save if a value has changed
  }
}

uint32_t
CHourMeter::_getLclRunTime()
{
  uint32_t rt = RunTime.get();
#ifdef DEBUG_HOURMETER
  DebugPort.printf("HrMtr _GetLclRunTime(): %d %d\r\n", rt, RTC_Store.getRunTime());
#endif
  return  rt + RTC_storageInterval * RTC_Store.getRunTime();
}

uint32_t 
CHourMeter::getRunTime()
{
  uint32_t rt = _getLclRunTime();
#ifdef DEBUG_HOURMETER
  DebugPort.printf("HrMtr GetRunTime(): %d %d\r\n", rt, NVstore.getHourMeter().RunTime);
#endif
  return  rt + NVstore.getHourMeter().RunTime;
}

uint32_t
CHourMeter::_getLclGlowTime()
{
  uint32_t gt = GlowTime.get();
#ifdef DEBUG_HOURMETER
  DebugPort.printf("HrMtr _GetLclGlowTime(): %d %d\r\n", gt, RTC_Store.getGlowTime());
#endif
  return  gt + RTC_storageInterval * RTC_Store.getGlowTime();
}

uint32_t 
CHourMeter::getGlowTime()
{
  uint32_t gt = _getLclGlowTime();
#ifdef DEBUG_HOURMETER
  DebugPort.printf("HrMtr GetGlowTime(): %d %d\r\n", gt, NVstore.getHourMeter().GlowTime);
#endif
  return gt + NVstore.getHourMeter().GlowTime;
}

