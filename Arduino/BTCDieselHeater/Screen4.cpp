#include "128x64OLED.h"
#include "display.h"
#include "KeyPad.h"
#include "helpers.h"
#include "Screen4.h"
#include "BTCWifi.h"


void showScreen4(C128x64_OLED& display)
{
  CRect extents;

  display.setCursor(0, 24);
  display.print("IP addr:");
  display.setCursor(display.width(), 24);
  if(isWifiConnected()) {
    display.printRightJustified(getWifiAddrStr());
  }
  else {
    display.printRightJustified("Not active");
  }

}

void animateScreen4(C128x64_OLED& display)
{
  display.display();
}


void keyhandlerScreen4(uint8_t event)
{
  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      return;
    }
    // press LEFT 
    if(event & key_Left) {
      prevScreen(); 
    }
    // press RIGHT 
    if(event & key_Right) {
      nextScreen(); 
    }
  }
}

