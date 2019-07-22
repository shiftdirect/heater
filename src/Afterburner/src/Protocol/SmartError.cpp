/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2019  Ray Jones <ray@mrjones.id.au>
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

#include "SmartError.h"
#include "TxManage.h"
#include "../Utility/helpers.h"
#include "../Utility/macros.h"
#include "../Utility/NVStorage.h"
#include "../Utility/DataFilter.h"

CSmartError::CSmartError()
{
  reset();
}

void 
CSmartError::reset()
{
  m_prevRunState = 0;
  m_Error = 0;
  m_bInhibit = false;
}

// we use inhibit when we manually command the heater off during preheat
// otherwise we'll register an ignition fail event
void
CSmartError::inhibit()
{
  m_bInhibit = true;
//  m_Error = 0;
}

// accept a fresh heater frame
// inpsect the advertised run state, tracking when and how it transitions out
// of preheat especially.
// abnormal transitions are registered and becoem our smart m_Error
// In addition, the hetaer frame has the ErrState updated to track the 
// smart error, providing no heater error exists!
void
CSmartError::monitor(const CProtocol& heaterFrame)
{
  bool bSilent = true;
  if(heaterFrame.verifyCRC(bSilent)) {  // check but don't report dodgy frames to debug
    // only accept valid heater frames!
    monitor(heaterFrame.getRunState());
  }
}

// test the new run state value, comparing to previous
// detect abnormal transitions
void 
CSmartError::monitor(uint8_t newRunState)
{
  // check if moving away from heater Idle state (S0)
  // especially useful if an OEM controller exists
  if((m_prevRunState == 0) && newRunState) {
    // reset the smart error
    m_Error = 0;
    m_bInhibit = false;
  }

  if(!m_bInhibit) {
    if(m_prevRunState == 2) {   // preheat state (S2)
      if(newRunState == 4) {
        // transitioned from preheat to ignited
        // - all good!
        m_Error = 0;
      }
      else if(newRunState > 5) {
        // transitioned from preheat to post glow
        // - second ignition attempt failed, heater is shutting down
        m_Error = 11;   // +1 over displayed error code
      }
      else if(newRunState == 3) {
        // transitioned from preheat to retry 
        // - first ignition attempt failed, heater will retry
        m_Error = 12;   // +1 over displayed error code
      }
    }
  }

  if(m_prevRunState != newRunState) {
    // check for transition to startup 
    // - force cancellation of an on request if we generated it
    if(newRunState >= 2) {
      TxManage.queueOnRequest(false);  // ensure ON request is cancelled
    }
    // check for transition to shutdown 
    // - force cancellation of an off request if we generated it
    if(newRunState >= 7 || newRunState == 0) {
      TxManage.queueOffRequest(false);  // ensure OFF request is cancelled
    }
  }

   m_prevRunState = newRunState;
}

bool
CSmartError::checkVolts(float ipVolts, float glowI)
{
 // check for low voltage
 // values here are x10 integers
 if(NVstore.getHeaterTuning().lowVolts) {   // only if enabled
    float cableComp = glowI * 0.1;             // allow 1V drop for 10A current (bit naive but better than no compensation)
    float Thresh = NVstore.getHeaterTuning().getLVC() - cableComp;  // NVstore
    if(ipVolts < Thresh) {
      m_Error = 2;     // +1 over displayed error code
      requestOff();
      return false;
    }
  }
  return true;
}

// return our smart error, if it exists, as the registered code
uint8_t 
CSmartError::getError()
{
  if(m_Error) {
    return m_Error;
  }
  return 0;
}