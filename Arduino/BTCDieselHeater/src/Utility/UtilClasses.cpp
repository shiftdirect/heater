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
#include "../Protocol/Protocol.h"
#include "UtilClasses.h"


// a class to track the blue wire receive / transmit states
// class CommStates 

void 
CommStates::set(eCS eState) 
{
  _State = eState;
  _Count = 0;
  if(_report) {
   static const char* stateNames[] = { 
     "Idle", "OEMCtrlRx", "OEMCtrlValidate", "HeaterRx1", "HeaterValidate1", "HeaterReport1", 
     "BTC_Tx", "HeaterRx2", "HeaterValidate2", "HeaterReport2", "TemperatureRead" 
    };
    if(_State == Idle) DebugPort.println("");  // clear screen
    DebugPort.printf("State: %s\r\n", stateNames[_State]);
  }
}

bool 
CommStates::collectData(CProtocol& Frame, unsigned char val, int limit) {   // returns true when buffer filled
  Frame.Data[_Count++] = val;
  return _Count >= limit;
}

bool 
CommStates::collectDataEx(CProtocol& Frame, unsigned char val, int limit) {   // returns true when buffer filled
  // guarding against rogue rx kernel buffer stutters....
  if((_Count == 0) && (val != 0x76)) {
    DebugPort.println("First heater byte not 0x76 - SKIPPING");
    return false;
  }
  Frame.Data[_Count++] = val;
  return _Count >= limit;
}

bool 
CommStates::checkValidStart(unsigned char val)
{
  if(_Count) 
    return true;
  else 
    return val == 0x76;
}

void
CommStates::setDelay(int ms)
{
  _delay = millis() + ms;
}

bool 
CommStates::delayExpired()
{
  long test = millis() - _delay;
  return(test >= 0);
}

CProfile::CProfile()
{
  tStart = millis();
}

unsigned long 
CProfile::elapsed(bool reset/* = false*/)
{
  unsigned long now = millis();
  unsigned long retval = now - tStart;
  if(reset)
    tStart = now;

  return retval;
}
