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
#include "NVStorage.h"
#include "DebugPort.h"

bool u8inBounds(uint8_t test, uint8_t minLim, uint8_t maxLim);
bool s8inBounds(int8_t test, int8_t minLim, int8_t maxLim);
bool u8Match2(uint8_t test, uint8_t test1, uint8_t test2);
bool u16inBounds(uint16_t test, uint16_t minLim, uint16_t maxLim);
bool s32inBounds(long test, long minLim, long maxLim);
bool thermoMethodinBounds(uint8_t test, uint8_t minLim, uint8_t maxLim);

bool 
sNVStore::valid()
{
  bool retval = true;
  retval &= Options.valid();
  for(int i=0; i<2; i++) {
    retval &= timer[i].valid();
  }
  retval &= Heater.valid();
  return retval;  
}

void 
sNVStore::init()
{
  for(int i=0; i<2; i++) {
    timer[i].init();
  }
  Options.init();
  Heater.init();
}

CHeaterStorage::CHeaterStorage()
{
  _calValues.Heater.init();
}

float
CHeaterStorage::getPmin()
{
  return float(_calValues.Heater.Pmin) * 0.1f;
}

float
CHeaterStorage::getPmax()
{
  return float(_calValues.Heater.Pmax) * 0.1f;
}

unsigned short
CHeaterStorage::getFmin()
{
  return _calValues.Heater.Fmin;
}

unsigned short
CHeaterStorage::getFmax()
{
  return _calValues.Heater.Fmax;
}

unsigned char
CHeaterStorage::getDesiredTemperature()
{
  return _calValues.Heater.setTemperature;
}

unsigned char
CHeaterStorage::getThermostatMode()
{
  return _calValues.Heater.ThermostatMode;
}

unsigned char 
CHeaterStorage::getThermostatMethodMode()
{
  return _calValues.Options.ThermostatMethod & 0x03;
}

float
CHeaterStorage::getThermostatMethodWindow()
{
  return float((_calValues.Options.ThermostatMethod >> 2) & 0x3f) * 0.1f;  // top 5 bits / 10, then / 2
}

void
CHeaterStorage::setPmin(float val)
{
  uint8_t cVal = (uint8_t)(val * 10.f + 0.5f);
  _calValues.Heater.Pmin = cVal;
}

void
CHeaterStorage::setPmax(float val)
{
  uint8_t cVal = (uint8_t)(val * 10.f + 0.5f);
  _calValues.Heater.Pmax = cVal;
}

void
CHeaterStorage::setFmin(unsigned short val)
{
  _calValues.Heater.Fmin = val;
}

void
CHeaterStorage::setFmax(unsigned short val)
{
  _calValues.Heater.Fmax = val;
}

void
CHeaterStorage::setDesiredTemperature(unsigned char val)
{
  _calValues.Heater.setTemperature = val;
}

void
CHeaterStorage::setThermostatMode(unsigned char val)
{
  _calValues.Heater.ThermostatMode = val;
}

void
CHeaterStorage::setThermostatMethodMode(unsigned char val)
{
  _calValues.Options.ThermostatMethod &= ~0x03;
  _calValues.Options.ThermostatMethod |= (val & 0x03);
}

void
CHeaterStorage::setThermostatMethodWindow(float val)
{
  _calValues.Options.ThermostatMethod &= 0x03;
  int nVal = int(val * 10 + 0.5);
  _calValues.Options.ThermostatMethod |= ((nVal & 0x3F) << 2);
}


void 
CHeaterStorage::setSystemVoltage(float fVal)
{
  int val = int(fVal * 10.0);
  if(val == 120 || val == 240) {
    _calValues.Heater.sysVoltage = val;
  }
}

unsigned char
CHeaterStorage::getSysVoltage()
{
  return _calValues.Heater.sysVoltage;
}

void
CHeaterStorage::setFanSensor(unsigned char val)
{
  if(val == 2)
    _calValues.Heater.fanSensor = 2;
  else 
    _calValues.Heater.fanSensor = 1;
}

unsigned char
CHeaterStorage::getFanSensor()
{
  return _calValues.Heater.fanSensor;
}

void
CHeaterStorage::setGlowDrive(unsigned char val)
{
  if(val >=1 && val <= 6)
    _calValues.Heater.glowDrive = val;
  else 
    _calValues.Heater.glowDrive = 5;
}

unsigned char
CHeaterStorage::getGlowDrive()
{
  return _calValues.Heater.glowDrive;
}

void 
CHeaterStorage::getTimerInfo(int idx, sTimer& timerInfo)
{
  if(idx >= 0 && idx < 14) {
    timerInfo = _calValues.timer[idx];
  }
}

void 
CHeaterStorage::setTimerInfo(int idx, const sTimer& timerInfo)
{
  if(idx >= 0 && idx < 14) {
    _calValues.timer[idx] = timerInfo;
  }
}

unsigned long 
CHeaterStorage::getDimTime()
{
  return _calValues.Options.DimTime;
}

void 
CHeaterStorage::setDimTime(unsigned long val)
{
  _calValues.Options.DimTime = val;
}

unsigned char 
CHeaterStorage::getDegFMode()
{
  return _calValues.Options.degF;
}

void 
CHeaterStorage::setDegFMode(unsigned char val)
{
  _calValues.Options.degF = val;
  save();
}

unsigned char 
CHeaterStorage::getWifiEnabled()
{
  return _calValues.Options.enableWifi;
}

void 
CHeaterStorage::setWifiEnabled(unsigned char val)
{
  _calValues.Options.enableWifi = val;
  save();
}

unsigned char 
CHeaterStorage::getOTAEnabled()
{
  return _calValues.Options.enableOTA;
}

void 
CHeaterStorage::setOTAEnabled(unsigned char val)
{
  _calValues.Options.enableOTA = val;
  save();
}

unsigned char
CHeaterStorage::getCyclicMode()
{
  return _calValues.Options.cyclicMode;
}

void 
CHeaterStorage::setCyclicMode(unsigned char val)
{
  _calValues.Options.cyclicMode = val;
  save();
}

GPIOinModes
CHeaterStorage::getGPIOinMode()
{
  GPIOinModes inMode = GPIOinNone;
  switch(_calValues.Options.GPIOinMode) {
    case 0: inMode = GPIOinNone; break;
    case 1: inMode = GPIOinOn1Off2; break;
    case 2: inMode = GPIOinOnHold1; break;
    case 3: inMode = GPIOinOn1Off1; break;
  }
  return inMode;
}

void 
CHeaterStorage::setGPIOinMode(unsigned char val)
{
  _calValues.Options.GPIOinMode = val;
}

GPIOoutModes
CHeaterStorage::getGPIOoutMode()
{
  GPIOoutModes outMode = GPIOoutNone;
  switch(_calValues.Options.GPIOoutMode) {
    case 0: outMode = GPIOoutNone; break;
    case 1: outMode = GPIOoutStatus; break;
    case 2: outMode = GPIOoutUser; break;
  }
  return outMode;
}

void 
CHeaterStorage::setGPIOoutMode(unsigned char val)
{
  _calValues.Options.GPIOoutMode = val;
}

unsigned char
CHeaterStorage::getGPIOalgMode()
{
  return _calValues.Options.GPIOalgMode;
}

void 
CHeaterStorage::setGPIOalgMode(unsigned char val)
{
  _calValues.Options.GPIOalgMode = val;
}

///////////////////////////////////////////////////////////////////////////////////////
//          ESP32
//
#ifdef ESP32

CESP32HeaterStorage::CESP32HeaterStorage()
{
}

CESP32HeaterStorage::~CESP32HeaterStorage()
{
}

void
CESP32HeaterStorage::init()
{
}

void 
CESP32HeaterStorage::load()
{
  DebugPort.println("Reading from NV storage");
  loadHeater();
  for(int i=0; i<14; i++) {
    loadTimer(i);
  }
  loadUI();
}

void 
CESP32HeaterStorage::save()
{
  DebugPort.println("Saving to NV storage");
  saveHeater();
  for(int i=0; i<14; i++) {
    saveTimer(i);
  }
  saveUI();
}

void 
CESP32HeaterStorage::loadHeater()
{
  // section for heater calibration params
  preferences.begin("Calibration", false);
  validatedLoad("minPump", _calValues.Heater.Pmin, 14, u8inBounds, 4, 100);
  validatedLoad("maxPump", _calValues.Heater.Pmax, 45, u8inBounds, 4, 150);
  validatedLoad("minFan", _calValues.Heater.Fmin, 1500, u16inBounds, 100, 5000);
  validatedLoad("maxFan", _calValues.Heater.Fmax, 4500, u16inBounds, 100, 6000);
  validatedLoad("thermostat", _calValues.Heater.ThermostatMode, 1, u8inBounds, 0, 1);
  validatedLoad("setTemperature", _calValues.Heater.setTemperature, 22, u8inBounds, 0, 40);
  validatedLoad("systemVoltage", _calValues.Heater.sysVoltage, 120, u8Match2, 120, 240);
  validatedLoad("fanSensor", _calValues.Heater.fanSensor, 1, u8inBounds, 1, 2);
  validatedLoad("glowDrive", _calValues.Heater.glowDrive, 5, u8inBounds, 1, 6);
  preferences.end();    
}

void 
CESP32HeaterStorage::saveHeater()
{
  // section for heater calibration params
  preferences.begin("Calibration", false);
  preferences.putUChar("minPump", _calValues.Heater.Pmin);
  preferences.putUChar("maxPump", _calValues.Heater.Pmax);
  preferences.putUShort("minFan", _calValues.Heater.Fmin);
  preferences.putUShort("maxFan", _calValues.Heater.Fmax);
  preferences.putUChar("thermostat", _calValues.Heater.ThermostatMode);
  preferences.putUChar("setTemperature", _calValues.Heater.setTemperature);
  preferences.putUChar("systemVoltage", _calValues.Heater.sysVoltage);
  preferences.putUChar("fanSensor", _calValues.Heater.fanSensor);
  preferences.putUChar("glowDrive", _calValues.Heater.glowDrive);
  preferences.end();    
}

void 
CESP32HeaterStorage::loadTimer(int idx) 
{
  sTimer& timer = _calValues.timer[idx];
  timer.timerID = idx;
  char SectionName[16];
  sprintf(SectionName, "timer%d", idx+1);
  preferences.begin(SectionName, false);
  validatedLoad("startHour", timer.start.hour, 0, s8inBounds, 0, 23);
  validatedLoad("startMin", timer.start.min, 0, s8inBounds, 0, 59);
  validatedLoad("stopHour", timer.stop.hour, 0, s8inBounds, 0, 23);
  validatedLoad("stopMin", timer.stop.min, 0, s8inBounds, 0, 59);
  validatedLoad("enabled", timer.enabled, 0, u8inBounds, 0, 255);  // all 8 bits used!
  validatedLoad("repeat", timer.repeat, 0, u8inBounds, 0, 1);
  validatedLoad("temperature", timer.temperature, 22, u8inBounds, 8, 35);
  preferences.end();    
}

void 
CESP32HeaterStorage::saveTimer(int idx) 
{
  sTimer& timer = _calValues.timer[idx];
  char SectionName[16];
  sprintf(SectionName, "timer%d", idx+1);
  preferences.begin(SectionName, false);
  preferences.putChar("startHour", timer.start.hour);
  preferences.putChar("startMin", timer.start.min);
  preferences.putChar("stopHour", timer.stop.hour);
  preferences.putChar("stopMin", timer.stop.min);
  preferences.putUChar("enabled", timer.enabled);
  preferences.putUChar("repeat", timer.repeat);
  preferences.putUChar("temperature", timer.temperature);
  preferences.end();    
}

void 
CESP32HeaterStorage::loadUI()
{
  preferences.begin("user", false);
  validatedLoad("dimTime", _calValues.Options.DimTime, 60000, s32inBounds, 0, 600000);
  validatedLoad("degF", _calValues.Options.degF, 0, u8inBounds, 0, 1);
  validatedLoad("thermoMethod", _calValues.Options.ThermostatMethod, (10 << 2), u8inBounds, 0, 2, 0x03);
  validatedLoad("enableWifi", _calValues.Options.enableWifi, 1, u8inBounds, 0, 1);
  validatedLoad("enableOTA", _calValues.Options.enableOTA, 1, u8inBounds, 0, 1);
  validatedLoad("cyclicMode", _calValues.Options.cyclicMode, 0, u8inBounds, 0, 10);
  validatedLoad("GPIOinMode", _calValues.Options.GPIOinMode, 0, u8inBounds, 0, 3);
  validatedLoad("GPIOoutMode", _calValues.Options.GPIOoutMode, 0, u8inBounds, 0, 2);
  validatedLoad("GPIOalgMode", _calValues.Options.GPIOalgMode, 0, u8inBounds, 0, 2);
  preferences.end();    
}

void 
CESP32HeaterStorage::saveUI()
{
  preferences.begin("user", false);
  preferences.putULong("dimTime", _calValues.Options.DimTime);
  preferences.putUChar("degF", _calValues.Options.degF);
  preferences.putUChar("thermoMethod", _calValues.Options.ThermostatMethod);
  preferences.putUChar("enableWifi", _calValues.Options.enableWifi);
  preferences.putUChar("enableOTA", _calValues.Options.enableOTA);
  preferences.putUChar("cyclicMode", _calValues.Options.cyclicMode);
  preferences.putUChar("GPIOinMode", _calValues.Options.GPIOinMode);
  preferences.putUChar("GPIOoutMode", _calValues.Options.GPIOoutMode);
  preferences.putUChar("GPIOalgMode", _calValues.Options.GPIOalgMode);
  preferences.end();    
}

bool
CESP32HeaterStorage::validatedLoad(const char* key, uint8_t& val, int defVal, std::function<bool(uint8_t, uint8_t, uint8_t)> validator, int min, int max, uint8_t mask)
{
  val = preferences.getUChar(key, defVal);
  if(!validator(val & mask, min, max)) {

    DebugPort.print("CESP32HeaterStorage::validatedLoad<uint8_t> invalid read ");
    DebugPort.print(key); DebugPort.print("="); DebugPort.print(val);
    DebugPort.print(" validator("); DebugPort.print(min); DebugPort.print(","); DebugPort.print(max); DebugPort.print(") reset to ");
    DebugPort.println(defVal);

    val = defVal;
    preferences.putUChar(key, val);
    return false;
  }
  return true;
}

bool
CESP32HeaterStorage::validatedLoad(const char* key, int8_t& val, int defVal, std::function<bool(int8_t, int8_t, int8_t)> validator, int min, int max)
{
  val = preferences.getChar(key, defVal);
  if(!validator(val, min, max)) {

    DebugPort.print("CESP32HeaterStorage::validatedLoad<uint8_t> invalid read ");
    DebugPort.print(key); DebugPort.print("="); DebugPort.print(val);
    DebugPort.print(" validator("); DebugPort.print(min); DebugPort.print(","); DebugPort.print(max); DebugPort.print(") reset to ");
    DebugPort.println(defVal);

    val = defVal;
    preferences.putChar(key, val);
    return false;
  }
  return true;
}

bool
CESP32HeaterStorage::validatedLoad(const char* key, uint16_t& val, int defVal, std::function<bool(uint16_t, uint16_t, uint16_t)> validator, int min, int max)
{
  val = preferences.getUShort(key, defVal);
  if(!validator(val, min, max)) {

    DebugPort.print("CESP32HeaterStorage::validatedLoad<uint16_t> invalid read ");
    DebugPort.print(key); DebugPort.print("="); DebugPort.print(val);
    DebugPort.print(" validator("); DebugPort.print(min); DebugPort.print(","); DebugPort.print(max); DebugPort.print(") reset to ");
    DebugPort.println(defVal);

    val = defVal;
    preferences.putUShort(key, val);
    return false;
  }
  return true;
}

bool
CESP32HeaterStorage::validatedLoad(const char* key, long& val, long defVal, std::function<bool(long, long, long)> validator, long min, long max)
{
  val = preferences.getLong(key, defVal);
  if(!validator(val, min, max)) {

    DebugPort.print("CESP32HeaterStorage::validatedLoad<long> invalid read ");
    DebugPort.print(key); DebugPort.print("="); DebugPort.print(val);
    DebugPort.print(" validator("); DebugPort.print(min); DebugPort.print(","); DebugPort.print(max); DebugPort.print(") reset to ");
    DebugPort.println(defVal);

    val = defVal;
    preferences.putLong(key, val);
    return false;
  }
  return true;
}

bool u8inBounds(uint8_t test, uint8_t minLim, uint8_t maxLim)
{
  return (test >= minLim) && (test <= maxLim);
}

bool s8inBounds(int8_t test, int8_t minLim, int8_t maxLim)
{
  return (test >= minLim) && (test <= maxLim);
}

bool u8Match2(uint8_t test, uint8_t test1, uint8_t test2)
{
  return (test == test1) || (test == test2);
}

bool u16inBounds(uint16_t test, uint16_t minLim, uint16_t maxLim)
{
  return (test >= minLim) && (test <= maxLim);
}

bool s32inBounds(long test, long minLim, long maxLim)
{
  return (test >= minLim) && (test <= maxLim);
}


#endif  // ESP32