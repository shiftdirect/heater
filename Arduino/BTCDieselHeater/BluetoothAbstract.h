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
#include "UtilClasses.h"
#include "Debugport.h"

class CProtocol;

extern void Command_Interpret(const char* pLine);   // decodes received command lines, implemented in main .ino file!


class CBluetoothAbstract {
protected:
  sRxLine _rxLine;
  CContextTimeStamp _timeStamp;
public:
  virtual void init() {};
  virtual void setRefTime() { 
    _timeStamp.setRefTime(); 
  };
  virtual void sendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm=true) {
    _timeStamp.report(pHdr);
    DebugReportFrame(pHdr, Frame, lineterm ? "\r\n" : "   ");
  };
  virtual void check() {};
  virtual void collectRxData(char rxVal) {
    // provide common behviour for bytes received from a bluetooth client
    if(isControl(rxVal)) {    // "End of Line"
      Command_Interpret(_rxLine.Line);
      _rxLine.clear();
    }
    else {
      _rxLine.append(rxVal);   // append new char to our Rx buffer
    }
  };
  virtual bool isConnected() { return false; };
};

extern CBluetoothAbstract& getBluetoothClient();

#endif // __BLUETOOTHABSTRACT_H__
