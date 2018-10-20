#include <Arduino.h>

class CProtocol;

void Bluetooth_Init();
void Bluetooth_SendFrame(const char* pHdr, const CProtocol& Frame);
void Bluetooth_Check();

extern void Command_Interpret(String line);   // decodes received command lines, implemented in main .ino file!



