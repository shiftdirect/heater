#include "stdint.h"

class C128x64_OLED;
class CProtocol;

void showScreen1(C128x64_OLED& display, const CProtocol& CtlFrame, const CProtocol& HtrFrame);
void animateScreen1(C128x64_OLED& display);

void keyhandlerScreen1(uint8_t event);
