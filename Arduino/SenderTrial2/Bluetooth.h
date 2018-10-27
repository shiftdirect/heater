#include <Arduino.h>

class CProtocol;

void Bluetooth_Init();
void Bluetooth_SendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm=true);
void Bluetooth_Check();
void Bluetooth_SendACK();

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



