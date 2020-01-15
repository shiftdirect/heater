/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2018  Ray Jones <ray@mrjones.id.au>
 * Copyright (C) 2018  James Clark
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

#include "ABTelnetSpy.h"

ABTelnetSpy::ABTelnetSpy() : TelnetSpy() {
  _enabled = true;
  _CRtimeout = 0;
  _pending = 0;
}

size_t 
ABTelnetSpy::write(uint8_t val) {
  if(_enabled) {
    return TelnetSpy::write(val);
  }
  return 0;
}

void 
ABTelnetSpy::enable(bool state)
{
  _enabled = state;
}

//  typical problem we have with terminal software that may or may not send CR/LF
// when a user hits ENTER.
// This method will inject a LF if none was received within a period of time
bool
ABTelnetSpy::getch(char& rxVal)
{
  if(_CRtimeout) {
    // have received a CR, now in wait interval for a LF
    long diff = millis() - _CRtimeout;
    if(available()) {
      // a character is sitting in the debug input
      // grab it and check if it is a LF, if so do not retain and return a faked LF
      // otherwise retain for next pass, and return a faked LF now
      _pending = read();                       // read pending character
      if(_pending == '\n') {                   // if it's a LF, a normal CR/LF occured
        _pending = 0;                          // cancel retention
      }
      diff = 1;                                // force timeout to happen now
    }
    if(diff > 0) {
      // timed out waiting for a LF, inject one now
      _CRtimeout = 0;                          // reset timeout
      rxVal = '\n';                            // force a LF response
      return true;                             // rxVal is valid
    }
    return false;                              // no character ready
  }

  // dispose of any character that may have been received during CR/LF timeout
  if(_pending) {
    rxVal = _pending;
    _pending = 0;
    return true;
  }
  if(available()) {
    rxVal = read();                            // read incoming character
    if(rxVal == '\r') {                        // if it's a CR, start the timeout
      _CRtimeout = (millis() + _LFPERIOD) | 1; // ensure always non zero
    }
    return true;                               // rxVal is valid
  }
  return false;                                // no character ready
}

