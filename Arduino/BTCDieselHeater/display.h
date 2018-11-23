#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <Arduino.h>

class CProtocol;

void initOLED();
void updateOLED(const CProtocol& CtlFrame, const CProtocol& HtrFrame);
void animateOLED();

void nextScreen();
void prevScreen();


#endif // __DISPLAY_H__
