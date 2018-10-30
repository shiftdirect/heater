#include <Arduino.h>

class CProtocol;

void Bluetooth_Init();
void Bluetooth_SendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm=true);
void Bluetooth_Check();
void Bluetooth_SendACK();

extern void Command_Interpret(const char* pLine);   // decodes received command lines, implemented in main .ino file!

