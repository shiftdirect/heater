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

#ifndef __BLUETOOTHABSTRACT_H__
#define __BLUETOOTHABSTRACT_H__

#include <Arduino.h>
#include "../Utility/UtilClasses.h"
#include "../Utility/Debugport.h"
#include "../Protocol/helpers.h"

class CProtocol;

class CBluetoothAbstract {
protected:
  sRxLine _rxLine;
  CContextTimeStamp _timeStamp;
  virtual void foldbackDesiredTemp() {};
public:
  virtual void begin() {};
  virtual void setRefTime() { 
    _timeStamp.setRefTime(); 
  };
  virtual void send(const char* Str) {};
  virtual void sendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm=true) {
    _timeStamp.report(pHdr);
    DebugReportFrame(pHdr, Frame, lineterm ? "\r\n" : "   ");
  };
  virtual void check() {};
  virtual void collectRxData(char rxVal) {
    // provide common behviour for bytes received from a bluetooth client
    _rxLine.append(rxVal);   // append new char to our Rx buffer
    if(rxVal == '}') {    // "End of JSON Line"
      interpretJsonCommand(_rxLine.Line);
      _rxLine.clear();
      foldbackDesiredTemp();   // rapid foldback if desired temp changes
    }
  };
  virtual bool isConnected() { return false; };
};

extern CBluetoothAbstract& getBluetoothClient();

#endif // __BLUETOOTHABSTRACT_H__
