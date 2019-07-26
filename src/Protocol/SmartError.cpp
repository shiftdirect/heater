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
  _prevRunState = 0;
  _Error = 0;
  _bInhibit = false;
}

// we use inhibit when we manually command the heater off during preheat
// otherwise we'll register an ignition fail event
void
CSmartError::inhibit()
{
  _bInhibit = true;
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
  if((_prevRunState == 0) && newRunState) {
    // reset the smart error
    _Error = 0;
    _bInhibit = false;
  }

  if(!_bInhibit) {
    if(_prevRunState == 2) {   // preheat state (S2)
      if(newRunState == 4) {
        // transitioned from preheat to ignited
        // - all good!
        _Error = 0;
      }
      else if(newRunState > 5) {
        // transitioned from preheat to post glow
        // - second ignition attempt failed, heater is shutting down
        _Error = 11;   // +1 over displayed error code
      }
      else if(newRunState == 3) {
        // transitioned from preheat to retry 
        // - first ignition attempt failed, heater will retry
        _Error = 12;   // +1 over displayed error code
      }
    }
  }

  if(_prevRunState != newRunState) {
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

  _prevRunState = newRunState;
}

/////////////////////////////////////////////////////////////////////////////
// checkVolts(float ipVolts, float glowI, bool throwfault)
//
// Performs low voltage cutout function / testing
// Input voltage drop is compensated here by using the glow plug current, 0.1V/A
//
// returns:
//    0 - OK
//    1 - Warning: Above LVC, but less than 12/24 (or .5V over LVC for higher LVC levels)
//    2 - Error:   LVC tripped
int
CSmartError::checkVolts(float ipVolts, float glowI, bool throwfault)
{
  // check for low voltage
  // Native NV values here are x10 integers

  // loom compenstaion, allow 1V drop for 10A current (bit naive but better than no compensation)
  float cableComp = glowI * 0.1;             
  float threshLVC = NVstore.getHeaterTuning().getLVC() - cableComp;  // NVstore
  
  // test low voltage cutout
  if(ipVolts < threshLVC) {
    if(throwfault) {     // only throw faults if directed to do so
      _Error = 2;        // internals error codes are +1 over displayed error code
      requestOff();      // shut heater down
    }
    return 2;            // Low voltage return value = 2
  }

  // warning threshold
  float threshWarn = threshLVC + 0.5;   // nominally create a warning threshold, 0.5V over LVC threhsold
  float alertlimit;                     // but always introduce it if below system voltage
  if(NVstore.getHeaterTuning().sysVoltage == 120) {
    alertlimit = 12.0 - cableComp;
  }
  else {
    alertlimit = 24.0 - cableComp;
  }
  if(threshWarn < alertlimit) {
    threshWarn = alertlimit;
  }

  if(ipVolts < threshWarn) {
    return 1;  // LVC OK, but below warning threshold, return code = 1
  }

  return 0;  // all good, return code = 0
}


// return our smart error, if it exists, as the registered code
uint8_t 
CSmartError::getError()
{
  return _Error;
}