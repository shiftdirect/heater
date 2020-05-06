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

#include <Arduino.h>
#include "DemandManager.h"
#include "NVStorage.h"
#include "helpers.h"
#include "../RTC/RTCStore.h"
#include "../Protocol/Protocol.h"

// Concept of timer operation:
//
// The volatile variables _setDegC & _setPumpHz are altered when a timer runs.
// The timer installs the programmed timer value as the new set point.
// But if the timer value is zero, no change takes place.
//
// _setPumpHz will match _setDegC.
// _setPumpHz is converted into Hz based using the interploation that is 
// performed inside the heater ECU according to Max/min temp & pump settings.
//
// When the timer interval expires, the volatile values revert back to the stored 
// RTC memory values.
//
// Exception. 
// If a timer is running and the user alters the setting, according to the current 
// thermostat state _setDegC or _setPumpHz are updated with the new demand and the 
// associated RTC memory value is also updated accordingly. 
// When the timer completes, it continues with the same value.
// The programmed timer temperature is not altered by the user adjusting the running 
// setting and the same setpoint will recur in the future when that timer runs again.

uint8_t CDemandManager::_setDegC = 22;
uint8_t CDemandManager::_setPumpHz = 22;


uint8_t
CDemandManager::getDegC() 
{
  return _setDegC;
}

uint8_t
CDemandManager::getPumpHz() 
{
  return _setPumpHz;   // value lies in the typical range of 8-35 - interpolated by heater to real Hz
}


// set a new temperature setpoint, also saving it to non volatile RTC memory (battery backed RAM)
void  
CDemandManager::setDegC(uint8_t newDegC) 
{
  BOUNDSLIMIT(newDegC, NVstore.getHeaterTuning().Tmin, NVstore.getHeaterTuning().Tmax);

  _setDegC = newDegC;
  RTC_Store.setDesiredTemp(newDegC);
}

// set a new pump setpoint, also saving it to non volatile RTC memory (battery backed RAM)
void 
CDemandManager::setPumpHz(uint8_t newDemand)
{
  // Pump demands use the same range as temperature demands :-)
  BOUNDSLIMIT(newDemand, NVstore.getHeaterTuning().Tmin, NVstore.getHeaterTuning().Tmax);

  _setPumpHz = newDemand;
  RTC_Store.setDesiredPump(newDemand);
}

// set a transient setpoint for use by prgrammed timer starts
// setpoints only change if timer temperature is actually defined
void
CDemandManager::setFromTimer(uint8_t timerDemand)
{
  if(timerDemand) {
    _setPumpHz = timerDemand;
    _setDegC = timerDemand;
  }
}

// revert setpoints to stored RTC memory values
void
CDemandManager::reload()
{
  _setDegC = RTC_Store.getDesiredTemp();
  _setPumpHz = RTC_Store.getDesiredPump();
}


// test the ambient temperature and check if it satisfies a start condition when running
// in thermostat mode
// If running in Fixed Hz, the start is not denied, but cyclic suspend may be engaged
CDemandManager::eStartCode 
CDemandManager::checkStart()
{
  // create a deny start temperature margin
  int stopDeltaT = 0;

  // determine temperature error vs desired thermostat value
  float deltaT = getTemperatureSensor() - getDegC();

  int cyclicstop = NVstore.getUserSettings().cyclic.Stop;
  if(cyclicstop) {                // cyclic mode enabled
    // if cyclic mode, raise the margin by the cyclic stop range
    stopDeltaT = cyclicstop + 1;  // bump up by 1 degree - no point invoking cyclic at 1 deg over!

    // alows honour cyclic stop threshold - immediate suspend transition
    if(deltaT > stopDeltaT) {
      return eStartSuspend;
    }
  }

  if(!isExtThermostatMode()) {
    if(deltaT > stopDeltaT) {
      // temperature exceeded the allowed margin
      // only deny start if actually using inbuilt thermostat mode
      if(isThermostat()) {
        return eStartTooWarm;  // too warm - deny start
      }
    }
  }

  return eStartOK;  // allow start
}


// generic method adjust the active heter demand.
// thi may be Pump Hz or desired temeperature, dependent upon if thermostat mode is active
bool 
CDemandManager::setDemand(uint8_t newDemand)
{
  if(hasOEMcontroller())
    return false;

  // bounds operate over the same range for either mode
  BOUNDSLIMIT(newDemand, NVstore.getHeaterTuning().Tmin, NVstore.getHeaterTuning().Tmax);
  
  // set and save the demand to NV storage
  // note that we now maintain fixed Hz and Thermostat set points seperately
  if(isThermostat()) {
    setDegC(newDemand);
  }
  else {
    setPumpHz(newDemand);
  }

  return true;
}

bool 
CDemandManager::deltaDemand(int delta)
{
  if(hasOEMcontroller())
    return false;

  uint8_t newDemand;
  if(isThermostat()) {
    newDemand = getDegC() + delta;
    setDegC(newDemand);
  }
  else {
    newDemand = getPumpHz() + delta;
    setPumpHz(newDemand);
  }

  return true;
}

bool 
CDemandManager::toggleThermostat()
{
  return setThermostatMode(isThermostat() ? 0 : 1);
}

bool 
CDemandManager::setThermostatMode(uint8_t val, bool save)
{
  if(hasOEMcontroller())
    return false;

  sUserSettings settings = NVstore.getUserSettings();
  settings.useThermostat = val ? 0x01 : 0x00;
  NVstore.setUserSettings(settings);
  if(save)
    NVstore.save();
  return true;
}

// set system to show degF or degC
void 
CDemandManager::setDegFMode(bool state)
{
  sUserSettings settings = NVstore.getUserSettings();
  settings.degF = state ? 0x01 : 0x00;
  NVstore.setUserSettings(settings);
  NVstore.save();
}

// return tru is using a thermostat mode
bool 
CDemandManager::isThermostat()
{
  if(hasOEMcontroller()) {
    return getHeaterInfo().isThermostat();
  }
  else {
    return NVstore.getUserSettings().useThermostat != 0;
  }
}

// generic get demand for Pump Hz or degC, as would be used in the value sent to the heater
uint8_t
CDemandManager::getDemand()
{
  if(hasOEMcontroller()) {
    return getHeaterInfo().getHeaterDemand();
  }
  else {
    if(isThermostat()) {
      return getDegC();
    }
    else {
      return getPumpHz();  // timer manager will return pump Hz, as demand value, not real Hz
    }
  }
}

// return true if external thermostat mode is active
bool 
CDemandManager::isExtThermostatMode()
{
#if USE_JTAG == 0
  return GPIOin.usesExternalThermostat() && (NVstore.getUserSettings().ThermostatMethod == 3);
#else
  //CANNOT USE GPIO WITH JTAG DEBUG
  return false;
#endif
}

// return true if external thermosat is closed
bool 
CDemandManager::isExtThermostatOn()
{
#if USE_JTAG == 0
  return GPIOin.getState(1);
#else
  //CANNOT USE GPIO WITH JTAG DEBUG
  return false;
#endif
}

const char* 
CDemandManager::getExtThermostatHoldTime()
{
#if USE_JTAG == 0
  return GPIOin.getExtThermHoldTime();
#else
  //CANNOT USE GPIO WITH JTAG DEBUG
  return "00:00";
#endif
}

