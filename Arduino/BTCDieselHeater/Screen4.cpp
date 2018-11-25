#include "128x64OLED.h"
#include "display.h"
#include "KeyPad.h"
#include "helpers.h"
#include "Screen4.h"
#include "BTCWifi.h"


CScreen4::CScreen4(C128x64_OLED& display, CScreenManager& mgr) : CScreen(display, mgr) 
{
}


void 
CScreen4::show(const CProtocol& CtlFrame, const CProtocol& HtrFrame)
{
  CScreen::show(CtlFrame, HtrFrame);
  
  CRect extents;

  _display.setCursor(0, 24);
  _display.print("IP addr:");
  _display.setCursor(_display.width(), 24);
  if(isWifiConnected()) {
    _display.printRightJustified(getWifiAddrStr());
  }
  else {
    _display.printRightJustified("Not active");
  }

}


void 
CScreen4::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      return;
    }
    // press LEFT 
    if(event & key_Left) {
      _Manager.prevScreen(); 
    }
    // press RIGHT 
    if(event & key_Right) {
      _Manager.nextScreen(); 
    }
  }
}

