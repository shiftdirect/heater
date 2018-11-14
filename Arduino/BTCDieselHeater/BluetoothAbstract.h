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

#endif // __BLUETOOTHABSTRACT_H__
