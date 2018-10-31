#include <Arduino.h>

class CProtocol;

extern void Command_Interpret(const char* pLine);   // decodes received command lines, implemented in main .ino file!

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


class CBluetoothAbstract {
protected:
  sRxLine _rxLine;
public:
  virtual void init() {};
  virtual void sendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm=true) {
    DebugPort.print(millis());
    DebugPort.print("ms ");
    DebugReportFrame(pHdr, Frame, lineterm ? "\r\n" : "   ");
  };
  virtual void check() {};
  virtual void collectRxData(char rxVal) {
    if(isControl(rxVal)) {    // "End of Line"
      Command_Interpret(_rxLine.Line);
      _rxLine.clear();
    }
    else {
      _rxLine.append(rxVal);   // append new char to our Rx buffer
    }
  };
};