#include <Arduino.h>


void Bluetooth_Init();
void Bluetooth_Report(const char* pHdr, const unsigned char Data[24]);
void Bluetooth_Check();

void Bluetooth_Interpret(String line);



