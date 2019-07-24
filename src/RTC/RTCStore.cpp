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
#include "RTCStore.h"
#include "Clock.h"
#include <Wire.h>
#include "../Utility/DebugPort.h"



// RTC storage, using alarm registers as GP storage
// MAXIMUM OF 7 BYTES
//
// [0..3] float fuelGauge strokes
//    [4] uint8_t DesiredTemp (typ. 8-35)
//    [5] uint8_t DesiredPump (typ. 8-35)
//    [6] uint8_t spare
//
//           ____________________________________________________
//          |    b7         |  b6  | b5 | b4 | b3 | b2 | b1 | b0 |
//          |---------------|------|-----------------------------|
// Byte[4]: | CyclicEngaged | bit6 |     Desired Deg Celcius     |
//          |---------------|------|-----------------------------|
// Byte[5]: |               |      |      Desired Pump Speed     |
//           ----------------------------------------------------

CRTC_Store::CRTC_Store()
{
  _accessed[0] = false;
  _accessed[1] = false;
  _accessed[2] = false;
  _accessed[3] = false;
  _fuelgauge = 0;
  _demandDegC = 22;
  _demandPump = 22;
  _CyclicEngaged = false;
  _RunTime = 0;
  _GlowTime = 0;
}

void
CRTC_Store::begin()
{
  if(Clock.lostPower()) {
    // RTC lost power - reset internal NV values to defaults
    DebugPort.println("CRTC_Store::begin() RTC lost power, re-initialising NV aspect");
    _demandPump = _demandDegC = 22;
    _CyclicEngaged = false;
    setFuelGauge(0);
    setDesiredTemp(_demandDegC);
    setDesiredPump(_demandPump);
    Clock.resetLostPower();
  }
  getFuelGauge();
  getDesiredTemp();
  getDesiredPump();
  getRunTime();
}

void 
CRTC_Store::setFuelGauge(float val)
{
  _accessed[0] = true;
  _fuelgauge = val;
  Clock.saveData((uint8_t*)&val, 4, 0);
}

float  
CRTC_Store::getFuelGauge()
{
  if(!_accessed[0]) {
    float NVval;
    Clock.readData((uint8_t*)&NVval, 4, 0);
    _fuelgauge = NVval;
    _accessed[0] = true;
    DebugPort.printf("RTC_Store - read fuel gauge %.2f\r\n", _fuelgauge);
  }
  return _fuelgauge;
}

void 
CRTC_Store::setDesiredTemp(uint8_t val)
{
  _demandDegC = val;
  _PackAndSaveByte4();
}

uint8_t
CRTC_Store::getDesiredTemp()
{
  _ReadAndUnpackByte4();
  return _demandDegC;
}

bool  
CRTC_Store::getCyclicEngaged()
{
  _ReadAndUnpackByte4();
  return _CyclicEngaged;
}

void 
CRTC_Store::setCyclicEngaged(bool active)
{
  _CyclicEngaged = active;
  _PackAndSaveByte4();
}

void 
CRTC_Store::setDesiredPump(uint8_t val)
{
  _demandPump = val;
  _PackAndSaveByte5();
}

uint8_t
CRTC_Store::getDesiredPump()
{
  _ReadAndUnpackByte5();
  return _demandPump;
}


bool
CRTC_Store::incRunTime()
{
  _RunTime++;
  _RunTime &= 0x1f;
  _PackAndSaveByte6();
  return _RunTime == 0;
}
  
bool
CRTC_Store::incGlowTime()
{
  _GlowTime++;
  _GlowTime &= 0x07;
  _PackAndSaveByte6();
  return _GlowTime == 0;
}

int
CRTC_Store::getRunTime()
{
  _ReadAndUnpackByte6();
  return _RunTime;
}
  
int
CRTC_Store::getGlowTime()
{
  _ReadAndUnpackByte6();
  return _GlowTime;
}

void
CRTC_Store::_ReadAndUnpackByte4()
{
  if(!_accessed[1]) {
    uint8_t NVval = 0;
    Clock.readData((uint8_t*)&NVval, 1, 4);
    _demandDegC = NVval & 0x3f;
    _CyclicEngaged = (NVval & 0x80) != 0;
    _bit6 = (NVval & 0x40) != 0;
    _accessed[1] = true;
    DebugPort.printf("RTC_Store - read byte4: degC=%d, CyclicOn=%d, bit6=%d\r\n", _demandDegC, _CyclicEngaged, _bit6);
  }
}

void
CRTC_Store::_PackAndSaveByte4()
{
  uint8_t NVval = (_CyclicEngaged ? 0x80 : 0x00) 
                | (_bit6 ? 0x40 : 0x00)
                | (_demandDegC & 0x3f);
  Clock.saveData((uint8_t*)&NVval, 1, 4);
}

void
CRTC_Store::_ReadAndUnpackByte5()
{
  if(!_accessed[2]) {
    uint8_t NVval = 0;
    Clock.readData((uint8_t*)&NVval, 1, 5);
    _demandPump = NVval & 0x3f;
    _accessed[2] = true;
    DebugPort.printf("RTC_Store - read byte5: pump=%d\r\n", _demandPump);
  }
}

void
CRTC_Store::_PackAndSaveByte5()
{
  uint8_t NVval = (_demandPump & 0x3f);
  Clock.saveData((uint8_t*)&NVval, 1, 5);
}

void
CRTC_Store::_PackAndSaveByte6()
{
  uint8_t NVval = ((_GlowTime & 0x07)<<5) | (_RunTime & 0x1f);
  Clock.saveData((uint8_t*)&NVval, 1, 6);
}

void
CRTC_Store::_ReadAndUnpackByte6()
{
  if(!_accessed[3]) {
    uint8_t NVval = 0;
    Clock.readData((uint8_t*)&NVval, 1, 6);
    _GlowTime = (NVval >> 5) & 0x07;
    _RunTime = NVval & 0x1f;
    _accessed[3] = true;
    DebugPort.printf("RTC_Store - read byte6: glow=%d, run=%d\r\n", _GlowTime, _RunTime);
  }
}

