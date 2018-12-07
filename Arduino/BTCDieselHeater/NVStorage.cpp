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


CHeaterStorage::CHeaterStorage()
{
  calValues.Heater.Pmin = 14;
  calValues.Heater.Pmax = 40;
  calValues.Heater.Fmin = 1500;
  calValues.Heater.Fmax = 4500;
  calValues.Heater.ThermostatMode = 1;
  calValues.Heater.setTemperature = 22;
}

float
CHeaterStorage::getPmin()
{
  return float(calValues.Heater.Pmin) * 0.1f;
}

float
CHeaterStorage::getPmax()
{
  return float(calValues.Heater.Pmax) * 0.1f;
}

unsigned short
CHeaterStorage::getFmin()
{
  return calValues.Heater.Fmin;
}

unsigned short
CHeaterStorage::getFmax()
{
  return calValues.Heater.Fmax;
}

unsigned char
CHeaterStorage::getTemperature()
{
  return calValues.Heater.setTemperature;
}

unsigned char
CHeaterStorage::getThermostatMode()
{
  return calValues.Heater.ThermostatMode;
}

void
CHeaterStorage::setPmin(float val)
{
  uint8_t cVal = (uint8_t)(val * 10.f + 0.5f);
  calValues.Heater.Pmin = cVal;
}

void
CHeaterStorage::setPmax(float val)
{
  uint8_t cVal = (uint8_t)(val * 10.f + 0.5f);
  calValues.Heater.Pmax = cVal;
}

void
CHeaterStorage::setFmin(unsigned short val)
{
  calValues.Heater.Fmin = val;
}

void
CHeaterStorage::setFmax(unsigned short val)
{
  calValues.Heater.Fmax = val;
}

void
CHeaterStorage::setTemperature(unsigned char val)
{
  calValues.Heater.setTemperature = val;
}

void
CHeaterStorage::setThermostatMode(unsigned char val)
{
  calValues.Heater.ThermostatMode = val;
}

void 
CHeaterStorage::getTimerInfo(int idx, sTimer& timerInfo)
{
  if(idx >= 0 && idx <=1) {
    timerInfo = calValues.timer[idx];
  }
}

void 
CHeaterStorage::setTimerInfo(int idx, const sTimer& timerInfo)
{
  if(idx >= 0 && idx <=1) {
    calValues.timer[idx] = timerInfo;
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
  preferences.getBytes("calValues", &calValues, sizeof(sNVStore));
}

void CESP32HeaterStorage::save()
{
  preferences.putBytes("calValues", &calValues, sizeof(sNVStore));
}

#endif  // ESP32