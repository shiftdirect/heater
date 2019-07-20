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
#include "TimerManager.h"
#include <Wire.h>
//#include "DS3231Ex.h"
#include "../Utility/helpers.h"
#include "../Utility/NVStorage.h"
#include "../Utility/DebugPort.h"


// create ONE of the RTClib supported real time clock classes
#if RTC_USE_DS3231 == 1
RTC_DS3231Ex rtc;
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

  CTimerManager::createMap();
  
  update();
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
//    _checkTimers();
    // check timers upon minute rollovers
    if(_currentTime.minute() != _prevMinute) {
      CTimerManager::manageTime(_currentTime.hour(), _currentTime.minute(), _currentTime.dayOfTheWeek());
      _prevMinute = _currentTime.minute();
    }
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
CClock::saveData(uint8_t* pData, int len, int ofs)
{
  _rtc.writeData(pData, len, ofs);
}

void 
CClock::readData(uint8_t* pData, int len, int ofs)
{
  _rtc.readData(pData, len, ofs);
}

bool
CClock::lostPower()
{
  return _rtc.lostPower();
}

void
CClock::resetLostPower()
{
  _rtc.resetLostPower();
}

void setDateTime(const char* newTime)
{
  DebugPort.printf("setting time to: %s\r\n", newTime);
  int month,day,year,hour,minute,second;
  if(6 == sscanf(newTime, "%d/%d/%d %d:%d:%d", &day, &month, &year, &hour, &minute, &second)) {
    DateTime newDateTime(year, month, day, hour, minute, second);
    Clock.set(newDateTime);
  }
}

void setDate(const char* newDate)
{
  DebugPort.printf("setting date to: %s\r\n", newDate);
  int month,day,year;
  if(3 == sscanf(newDate, "%d/%d/%d", &day, &month, &year)) {
    DateTime currentDateTime = Clock.get();
    DateTime newDateTime(year, month, day, currentDateTime.hour(), currentDateTime.minute(), currentDateTime.second());
    Clock.set(newDateTime);
  }
}

void setTime(const char* newTime)
{
  DebugPort.printf("setting time to: %s\r\n", newTime);
  int hour,minute,second;
  if(3 == sscanf(newTime, "%d:%d:%d", &hour, &minute, &second)) {
    DateTime currentDateTime = Clock.get();
    DateTime newDateTime(currentDateTime.year(), currentDateTime.month(), currentDateTime.day(), hour, minute, second);
    Clock.set(newDateTime);
  }
}

#define _I2C_WRITE write
#define _I2C_READ  read

void 
RTC_DS3231Ex::writeData(uint8_t* pData, int len, int ofs) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire._I2C_WRITE((byte)(7+ofs)); // start at alarm bytes
  for(int i=0; i<len; i++) {
    Wire._I2C_WRITE(*pData++);
  }
  Wire.endTransmission();
}

void 
RTC_DS3231Ex::readData(uint8_t* pData, int len, int ofs) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire._I2C_WRITE((byte)(7+ofs)); // start at alarm bytes
  Wire.endTransmission();

  Wire.requestFrom(DS3231_ADDRESS, len);
  for(int i=0; i<len; i++) {
    *pData++ = Wire._I2C_READ();
  }
  Wire.endTransmission();
}

void 
RTC_DS3231Ex::resetLostPower() 
{
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire._I2C_WRITE(DS3231_STATUSREG);
  Wire.endTransmission();

  Wire.requestFrom(DS3231_ADDRESS, 1);
  uint8_t sts = Wire._I2C_READ();

  sts &= 0x7f;  // clear power loss flag

  Wire.beginTransmission(DS3231_ADDRESS);
  Wire._I2C_WRITE(DS3231_STATUSREG);
  Wire._I2C_WRITE(sts);
  Wire.endTransmission();
}

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
