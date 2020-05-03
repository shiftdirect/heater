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

#ifndef __UTIL_CLASSES_H__
#define __UTIL_CLASSES_H__

//#include <string.h>
#include "DebugPort.h"
#include "../cfg/BTCConfig.h"

class CProtocol;

// a class to track the blue wire receive / transmit states

#define COMMSTATES_CALLBACK_SIGNATURE std::function<void(char*)> CScallback

class CommStates {
public:
  // comms states
  enum eCS { 
    Idle, OEMCtrlRx, OEMCtrlValidate, HeaterRx1, HeaterValidate1, TxStart, TxInterval, HeaterRx2, HeaterValidate2, ExchangeComplete
  };

private:
  eCS _State;
  int _Count;
  unsigned long _delay;
  bool _report;
  std::function<void(const char*)> _callback;
public:
  CommStates() {
    _State = Idle;
    _Count = 0;
    _delay = millis();
    _report = false;
    _callback = NULL;
  }
  void set(eCS eState);
  eCS get() {
    return _State;
  }
  bool is(eCS eState) {
    return _State == eState;
  }
  bool collectData(CProtocol& Frame, uint8_t val, int limit = 24);
  bool checkValidStart(uint8_t val);
  void setDelay(int ms);
  bool delayExpired();
  bool toggleReporting() { 
    _report = !_report; 
    return isReporting();
  };
  bool isReporting() {
    return _report != 0;
  };
  void setCallback(std::function<void(const char*)> fn) { _callback = fn; };
};


// a class to collect a new data byte from the blue wire
class sRxData {
  bool newData;
  int  Value;
public:
  sRxData() {
    reset();
  }
  void reset() {
    newData = false;
    Value = 0;
  }
  void setValue(int value) {
    newData = true;
    Value = value;
  }
  bool available() {
    return newData;
  }
  int getValue() {
    return Value;
  }
};

// a class to collect rx bytes into a string, typ. until a line terminator (handled elsewhere)
struct sRxLine {
  char Line[1024];
  int  Len;
  sRxLine() {
    clear();
  }
  bool append(char val) {
    if(Len < (sizeof(Line) - 1)) {
      Line[Len++] = val;
      Line[Len] = 0;
      return true;
    }
    return false;
  }
  void clear() {
    Line[0] = 0;
    Len = 0;
  }
};


// a class to generate time stamps depending if a heater or otherwise frame header is presented
class CContextTimeStamp {
  unsigned long prevTime;
  unsigned long refTime;
public:
  CContextTimeStamp() {
    refTime = 0; 
    prevTime = 0;
  };
  void setRefTime() { 
    refTime = millis(); 
  };
  void report(bool isDelta, char* msg=NULL) {
    if(isDelta) {
      long delta = millis() - prevTime;
      if(msg)
        sprintf(msg, "%+8ldms ", delta);
      else
        DebugPort.printf("%+8ldms ", delta);
    }
    else {
      prevTime = millis();
      if(msg)
        sprintf(msg, "%8ldms ", prevTime - refTime);
      else
        DebugPort.printf("%8ldms ", prevTime - refTime);
    }
  };
  void report(char* msg = NULL) {
    prevTime = millis();
    if(msg) {
      sprintf(msg, "%8ldms ", prevTime - refTime);  
    }
    else 
      DebugPort.printf("%8ldms ", prevTime - refTime);
  };
};


struct CRect {
  // types match with getTextBounds in Adafruit_GFX
  int16_t xPos, yPos;
  uint16_t width, height;
  CRect() {
    xPos = yPos = width = height = 0;
  }
  CRect(const CRect& a) {
    xPos = a.xPos;
    yPos = a.yPos;
    width = a.width;
    height = a.height;
  }
  void Expand(int val) {
    xPos -= val;
    yPos -= val;
    width += 2 * val;
    height += 2 * val;
  }
};

class CProfile {
  unsigned long tStart;
public:
  CProfile();
  unsigned long elapsed(bool reset = false);
};

enum eOTAmodes { 
  eOTAnormal, eOTAbrowser, eOTAWWW
};

void setHoldoff(unsigned long& holdoff, unsigned long period);
void hexDump(uint8_t* pData, int len, int wrap=16);

#endif // __UTIL_CLASSES_H__
