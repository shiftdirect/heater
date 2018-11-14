#ifndef __UTIL_CLASSES_H__
#define __UTIL_CLASSES_H__

#include <Arduino.h>
#include <string.h>
#include "Protocol.h"
#include "debugport.h"


// a class to track the blue wire receive / transmit states
class CommStates {
public:
  // comms states
  enum eCS { 
    Idle, OEMCtrlRx, OEMCtrlReport, HeaterRx1, HeaterReport1, BTC_Tx, HeaterRx2, HeaterReport2, TemperatureRead
  };

  CommStates() {
    set(Idle);
  }
  void set(eCS eState) {
    m_State = eState;
    m_Count = 0;
  }
  eCS get() {
    return m_State;
  }
  bool is(eCS eState) {
    return m_State == eState;
  }
  bool collectData(CProtocol& Frame, unsigned char val, int limit = 24) {   // returns true when buffer filled
    Frame.Data[m_Count++] = val;
    return m_Count == limit;
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
      unsigned delta = millis() - prevTime;
      sprintf(msg, "%+8dms ", delta);
    }
    else {
      prevTime = millis();
      sprintf(msg, "%8dms ", prevTime - refTime);
    }
    DebugPort.print(msg);
  };
};

#endif // __UTIL_CLASSES_H__
