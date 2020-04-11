/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2020  Ray Jones <ray@mrjones.id.au>
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


///////////////////////////////////////////////////////////////////////////
//
// CDemandManager
//
// This provides management of the heater temperature or pump demands
//
///////////////////////////////////////////////////////////////////////////

#ifndef __DEMANDMANAGER_H__
#define __DEMANDMANAGER_H__

#include <stdint.h>

struct sTimer;

class CDemandManager {

private:
  static uint8_t _setDegC;
  static uint8_t _setPumpHz;

public:
  enum eStartCode { eStartOK=0, 
                    eStartTooWarm=-1,
                    eStartSuspend=-2,
                    eStartLVC=-3,
                    eStartLowFuel=-4
  };
  static uint8_t getDegC();          // absolute degC value to use
  static uint8_t getPumpHz();        // absolute Pump Hz value to use - scaled to fit temp range
  static uint8_t getDemand();        // can be pump Hz or degC, depending upon thermostat mode
  static void  setDegC(uint8_t newDegC);     // set and save absolute degC value to use
  static void  setPumpHz(uint8_t newDemand); // set and save absolute scaled pump Hz value to use 
  static bool  setDemand(uint8_t newDemand); // set and save demand, according to thermostat mode
  static bool  deltaDemand(int delta);       // nudge demand
  static void  setFromTimer(uint8_t newDemand);  // set voltile degC and PumpHz, for timer starts
  static void  reload();                     // reload stored RTC values
  static eStartCode checkStart();            // test if start allowed
  static bool  toggleThermostat();
  static bool  setThermostatMode(uint8_t val, bool save = true);
  static void  setDegFMode(bool state);
  static bool  isThermostat();
  static bool  isExtThermostatMode();  
  static bool  isExtThermostatOn();
  static const char* getExtThermostatHoldTime();
};

#endif //__DEMANDMANAGER_H__