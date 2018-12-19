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

#include <Arduino.h>
#include <string.h>
#include "../Protocol/Protocol.h"
#include "DebugPort.h"


// a class to track the blue wire receive / transmit states
class CommStates {
public:
  // comms states
  enum eCS { 
    Idle, OEMCtrlRx, OEMCtrlReport, HeaterRx1, HeaterReport1, BTC_Tx, HeaterRx2, HeaterReport2, TemperatureRead
  };

  CommStates() {
    m_State = Idle;
    m_Count = 0;
  }
  void set(eCS eState) {
    m_State = eState;
    m_Count = 0;
#if SHOW_STATE_MACHINE_TRANSITIONS == 1
    DebugPort.print("State");DebugPort.println(m_State);
#endif
  }
  eCS get() {
    return m_State;
  }
  bool is(eCS eState) {
    return m_State == eState;
  }
  bool collectData(CProtocol& Frame, unsigned char val, int limit = 24) {   // returns true when buffer filled
    Frame.Data[m_Count++] = val;
    return m_Count >= limit;
  }
  bool collectDataEx(CProtocol& Frame, unsigned char val, int limit = 24) {   // returns true when buffer filled
    // guarding against rogue rx kernel buffer stutters....
    if((m_Count == 0) && (val != 0x76)) {
      DebugPort.println("First heater byte not 0x76 - SKIPPING");
      return false;
    }
    Frame.Data[m_Count++] = val;
    return m_Count >= limit;
  }
  bool checkValidStart(unsigned char val)
  {
    if(m_Count) 
      return true;
    else 
      return val == 0x76;
  }

private:
  eCS m_State;
  int m_Count;
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
  char Line[64];
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
  void report(const char* pHdr) {
    char msg[32];
    if(strncmp(pHdr, "[HTR]", 5) == 0) {
      long delta = millis() - prevTime;
      sprintf(msg, "%+8ldms ", delta);
    }
    else {
      prevTime = millis();
      sprintf(msg, "%8dms ", prevTime - refTime);
    }
    DebugPort.print(msg);
  };
};


struct CRect {
  // types match with getTextBounds in Adafruit_GFX
  int16_t xPos, yPos;
  uint16_t width, height;
  CRect() {
    xPos = yPos = width = height = 0;
  }
  void Expand(int val) {
    xPos -= val;
    yPos -= val;
    width += 2 * val;
    height += 2 * val;
  }
};

#endif // __UTIL_CLASSES_H__
