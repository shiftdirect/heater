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
  m_State = eState;
  m_Count = 0;
#if SHOW_STATE_MACHINE_TRANSITIONS == 1
  DebugPort.print("State");DebugPort.println(m_State);
#endif
}

bool 
CommStates::collectData(CProtocol& Frame, unsigned char val, int limit) {   // returns true when buffer filled
  Frame.Data[m_Count++] = val;
  return m_Count >= limit;
}

bool 
CommStates::collectDataEx(CProtocol& Frame, unsigned char val, int limit) {   // returns true when buffer filled
  // guarding against rogue rx kernel buffer stutters....
  if((m_Count == 0) && (val != 0x76)) {
    DebugPort.println("First heater byte not 0x76 - SKIPPING");
    return false;
  }
  Frame.Data[m_Count++] = val;
  return m_Count >= limit;
}

bool 
CommStates::checkValidStart(unsigned char val)
{
  if(m_Count) 
    return true;
  else 
    return val == 0x76;
}

