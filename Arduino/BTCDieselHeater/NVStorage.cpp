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

#include "Arduino.h"
#include "NVStorage.h"
#include "DebugPort.h"

bool 
sNVStore::valid()
{
  bool retval = true;
  for(int i=0; i<2; i++) {
    retval &= (timer[i].start.hour >= 0 && timer[i].start.hour < 24);
    retval &= (timer[i].start.min >= 0 && timer[i].start.min < 60);
    retval &= (timer[i].stop.hour >= 0 && timer[i].stop.hour < 24);
    retval &= (timer[i].stop.min >= 0 && timer[i].stop.min < 60);
    retval &= timer[i].repeat < 2;
  }
  retval &= Heater.Pmin < 100;
  retval &= Heater.Pmax < 150;
  retval &= Heater.Fmin < 5000;
  retval &= Heater.Fmax < 6000;
  retval &= Heater.ThermostatMode < 2;
  retval &= Heater.setTemperature < 40;
  return retval;  
}

void 
sNVStore::init()
{
  for(int i=0; i<2; i++) {
    timer[i].start.hour = 0;
    timer[i].start.min = 0;
    timer[i].stop.hour = 0;
    timer[i].stop.min = 0;
    timer[i].enabled = 0;
    timer[i].repeat = 0;
  }
  Heater.Pmin = 14;
  Heater.Pmax = 45;
  Heater.Fmin = 1500;
  Heater.Fmax = 4500;
  Heater.ThermostatMode = 1;
  Heater.setTemperature = 23;
}

CHeaterStorage::CHeaterStorage()
{
  _calValues.Heater.Pmin = 14;
  _calValues.Heater.Pmax = 40;
  _calValues.Heater.Fmin = 1500;
  _calValues.Heater.Fmax = 4500;
  _calValues.Heater.ThermostatMode = 1;
  _calValues.Heater.setTemperature = 22;
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
CHeaterStorage::getTemperature()
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
CHeaterStorage::setTemperature(unsigned char val)
{
  _calValues.Heater.setTemperature = val;
}

void
CHeaterStorage::setThermostatMode(unsigned char val)
{
  _calValues.Heater.ThermostatMode = val;
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



///////////////////////////////////////////////////////////////////////////////////////
//          ESP32
//
#ifdef ESP32

CESP32HeaterStorage::CESP32HeaterStorage()
{
}

CESP32HeaterStorage::~CESP32HeaterStorage()
{
  preferences.end();
}

void
CESP32HeaterStorage::init()
{
  preferences.begin("dieselheater", false);
}

void CESP32HeaterStorage::load()
{
  DebugPort.println("Reading from NV storage");
  preferences.getBytes("calValues", &_calValues, sizeof(sNVStore));
  if(!_calValues.valid()) {
    DebugPort.println("Invalid NV storage, initialising");
    _calValues.init();
    save();
  }
}

void CESP32HeaterStorage::save()
{
  DebugPort.println("Saving to NV storage");
  preferences.putBytes("calValues", &_calValues, sizeof(sNVStore));
}

#endif  // ESP32