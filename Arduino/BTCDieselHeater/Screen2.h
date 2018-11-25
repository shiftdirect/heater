#include "stdint.h"

class C128x64_OLED;
class CProtocol;

void showScreen2(C128x64_OLED& display, const CProtocol& CtlFrame, const CProtocol& HtrFrame);
void animateScreen2(C128x64_OLED& display);

void keyhandlerScreen2(uint8_t event);
