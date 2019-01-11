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

bool inBounds(uint8_t test, uint8_t minLim, uint8_t maxLim);
bool inBounds(uint16_t test, uint16_t minLim, uint16_t maxLim);

bool 
sNVStore::valid()
{
  bool retval = true;
  retval &= (DimTime >= 0) && (DimTime < 300000);  // 5 mins
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
  DimTime = 60000;  // 1 minute
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
CHeaterStorage::getTimerInfo(int idx, sTimer& timerInfo)
{
  if(idx >= 0 && idx <=1) {
    timerInfo = _calValues.timer[idx];
  }
}

void 
CHeaterStorage::setTimerInfo(int idx, const sTimer& timerInfo)
{
  if(idx >= 0 && idx <=1) {
    _calValues.timer[idx] = timerInfo;
  }
}

unsigned long 
CHeaterStorage::getDimTime()
{
  return _calValues.DimTime;
}

void 
CHeaterStorage::setDimTime(unsigned long val)
{
  _calValues.DimTime = val;
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
  // section for heater calibration params
  loadHeater();
  if(!_calValues.Heater.valid()) {
    _calValues.Heater.init();
    saveHeater();
  }
  // section for timers
  for(int i=0; i<2; i++) {
    loadTimer(i+1, _calValues.timer[i]);
    if(!_calValues.timer[i].valid()) {
      _calValues.timer[i].init();
      saveTimer(i+1, _calValues.timer[i]);
    }
  }
  loadUI();
}

void 
CESP32HeaterStorage::save()
{
  DebugPort.println("Saving to NV storage");
  saveHeater();
  for(int i=0; i<2; i++) {
    saveTimer(i+1, _calValues.timer[i]);
  }
  saveUI();
}

void 
CESP32HeaterStorage::loadHeater()
{
  // section for heater calibration params
  preferences.begin("Calibration", false);
  _calValues.Heater.Pmin = preferences.getUChar("minPump", 1.4);
  _calValues.Heater.Pmax = preferences.getUChar("maxPump", 4.5);
  _calValues.Heater.Fmin = preferences.getUShort("minFan", 1500);
  _calValues.Heater.Fmax = preferences.getUShort("maxFan", 4500);
  _calValues.Heater.ThermostatMode = preferences.getUChar("thermostat", 1);
  _calValues.Heater.setTemperature = preferences.getUChar("setTemperature", 22);
  _calValues.Heater.sysVoltage = preferences.getUChar("systemVoltage", 120);
  _calValues.Heater.fanSensor = preferences.getUChar("fanSensor", 1);
  _calValues.Heater.glowPower = preferences.getUChar("glowPower", 5);
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
  preferences.putUChar("glowPower", _calValues.Heater.glowPower);
  preferences.end();    
}

void 
CESP32HeaterStorage::loadTimer(int idx, sTimer& timer) {
  char SectionName[16];
  sprintf(SectionName, "timer%d", idx);
  if(!preferences.begin(SectionName, false))
    DebugPort.println("Preferences::begin() failed");
  timer.start.hour = preferences.getUChar("startHour", 0);
  timer.start.min = preferences.getUChar("startMin", 0);
  timer.stop.hour = preferences.getUChar("stopHour", 0);
  timer.stop.min = preferences.getUChar("stopMin", 0);
  timer.enabled = preferences.getUChar("enabled", 0);
  timer.repeat = preferences.getUChar("repeat", 0);
  preferences.end();    
  DebugPort.println("LOADED idx start stop en rpt");
  DebugPort.print(idx);
  DebugPort.print(" ");
  DebugPort.print(timer.start.hour);
  DebugPort.print(":");
  DebugPort.print(timer.start.min);
  DebugPort.print(" ");
  DebugPort.print(timer.stop.hour);
  DebugPort.print(":");
  DebugPort.print(timer.stop.min);
  DebugPort.print(" ");
  DebugPort.print(timer.enabled);
  DebugPort.print(" ");
  DebugPort.println(timer.repeat);
}

void 
CESP32HeaterStorage::saveTimer(int idx, sTimer& timer) 
{
  char SectionName[16];
  sprintf(SectionName, "timer%d", idx);
  preferences.begin(SectionName, false);
  preferences.putUChar("startHour", timer.start.hour);
  preferences.putUChar("startMin", timer.start.min);
  preferences.putUChar("stopHour", timer.stop.hour);
  preferences.putUChar("stopMin", timer.stop.min);
  preferences.putUChar("enabled", timer.enabled);
  preferences.putUChar("repeat", timer.repeat);
  preferences.end();    
  DebugPort.println("SAVED idx start stop en rpt");
  DebugPort.print(idx);
  DebugPort.print(" ");
  DebugPort.print(timer.start.hour);
  DebugPort.print(":");
  DebugPort.print(timer.start.min);
  DebugPort.print(" ");
  DebugPort.print(timer.stop.hour);
  DebugPort.print(":");
  DebugPort.print(timer.stop.min);
  DebugPort.print(" ");
  DebugPort.print(timer.enabled);
  DebugPort.print(" ");
  DebugPort.println(timer.repeat);
}

void 
CESP32HeaterStorage::loadUI()
{
  preferences.begin("user", false);
  _calValues.DimTime = preferences.getUChar("dimTime", 60000);
  preferences.end();    
  if(!((_calValues.DimTime >= 0) && (_calValues.DimTime < 300000))) {   // 5 mins
    _calValues.DimTime = 60000;
    saveUI();
  }
}

void 
CESP32HeaterStorage::saveUI()
{
  preferences.begin("user", false);
  preferences.putUChar("dimTime", _calValues.DimTime);
  preferences.end();    
}

bool inBounds(uint8_t test, uint8_t minLim, uint8_t maxLim)
{
  return (test >= minLim) && (test <= maxLim);
}
bool inBounds(uint16_t test, uint16_t minLim, uint16_t maxLim)
{
  return (test >= minLim) && (test <= maxLim);
}

#endif  // ESP32