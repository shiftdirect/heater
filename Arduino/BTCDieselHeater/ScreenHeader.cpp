#include <Arduino.h>
#include "ScreenHeader.h"
#include "Protocol.h"
#include "128x64OLED.h"
#include "BTCWifi.h"
#include "BluetoothAbstract.h" 
#include "clock.h"
#include "Icons.h"
#include "MiniFont.h"
#include "helpers.h"

#define MINIFONT miniFontInfo

#define X_BATT_ICON    95
#define Y_BATT_ICON     0
#define X_WIFI_ICON    22
#define Y_WIFI_ICON     0
#define X_BT_ICON      12
#define Y_BT_ICON       0

#define MINI_BATTLABEL

#define DEMO_AP_MODE

CScreenHeader::CScreenHeader(C128x64_OLED& disp, CScreenManager& mgr) : CScreen(disp, mgr)
{
  _animateState = 0;
  _animationHold = 0;
  _needAnimationClear = false;
}

void 
CScreenHeader::show()
{
  _display.clearDisplay();

  // standard header items
  //Bluetooth
  if(getBluetoothClient().isConnected())
    showBTicon();
  // WiFi
  if(isWifiConnected()) {
    showWifiIcon();
  }
  // battery
  showBatteryIcon(getHeaterInfo().getBattVoltage());

  showTime(_display);
}

// Animate IN/OUT arrows against the WiFi icon, according to actual web server traffic
//   an IN arrow is drawn if incoming data has been detected
//   an OUT arrow is drawn if outgoing data has been sent
//
// Each arrow is drawn standalone, with a clear interval for a clean flash on the display
// The following is a typical sequence, relative to animation ticks, note the gap
// always appears in the animation interval between any arrow shown
//   
//    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
// ________^^^^^^____vvvvv_______________vvvvv__________^^^^^_____vvvvv_____________________

bool 
CScreenHeader::animate()
{
  bool retval = false;
  if(isWifiConnected() && isWebClientConnected()) {
    int xPos = X_WIFI_ICON + W_WIFI_ICON;
#ifdef DEMO_AP_MODE
    xPos += 4;
#endif
    int yPos = H_WIFI_ICON - H_WIFIIN_ICON + 1;
    // animation hold ensures our arrow indications always have a clear 
    // period between them, otherwise they just visibly mush together
    if(_animationHold) 
      _animationHold--;
    // as we control (moderate) the delivery, check for any data we have sent first 
    // An over enthusiastic client would otherwise block the out arrow animation!
    if(!_animationHold && hasWebServerSpoken(true)) {
      // we have emitted data to the web client, show an OUT arrow
//      _display.drawBitmap(xPos, yPos, wifiOutIcon, W_WIFIIN_ICON, H_WIFIIN_ICON, WHITE);
      _display.drawBitmap(xPos, 0, wifiOutIcon, W_WIFIIN_ICON, H_WIFIIN_ICON, WHITE);
      _needAnimationClear = true;  // clear arrow upon next iteration
      _animationHold = 2;          // prevent anotehr arrow appearing before previous arrow has been scrubbed
      retval = true;
    }
    else if(!_animationHold && hasWebClientSpoken(true)) {
      // we have receievd data from the web client, show an IN arrow
      _display.drawBitmap(xPos, yPos, wifiInIcon, W_WIFIIN_ICON, H_WIFIIN_ICON, WHITE);
      _needAnimationClear = true;  // clear arrow upon next iteration
      _animationHold = 2;          // prevent another arrow appearing before previous arrow has been scrubbed 
      retval = true;
    }
    else if(_needAnimationClear) { // an arrow was drawn in the prior iteration, now erase it 
      _display.fillRect(xPos, yPos, W_WIFIIN_ICON, H_WIFIIN_ICON, BLACK);
      _display.fillRect(xPos, 0, W_WIFIIN_ICON, H_WIFIIN_ICON, BLACK);
      retval = true;
      _needAnimationClear = false;
    }
  }
  return retval;                    // true if we have updated the display contents
}

void 
CScreenHeader::showBTicon()
{
  _display.drawBitmap(X_BT_ICON, Y_BT_ICON, BTicon, W_BT_ICON, H_BT_ICON, WHITE);
}

void 
CScreenHeader::showWifiIcon()
{
  _display.drawBitmap(X_WIFI_ICON, Y_WIFI_ICON, wifiIcon, W_WIFI_ICON, H_WIFI_ICON, WHITE);
#ifdef DEMO_AP_MODE
  _display.fillRect(X_WIFI_ICON + 8, Y_WIFI_ICON + 5, 10, 7, BLACK);
  CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
  _display.setCursor(X_WIFI_ICON+9, Y_WIFI_ICON+6);
  _display.print("AP");
#endif
}

void
CScreenHeader::showBatteryIcon(float voltage)
{
  _display.drawBitmap(X_BATT_ICON, Y_BATT_ICON, BatteryIcon, W_BATT_ICON, H_BATT_ICON, WHITE);
  char msg[16];
  sprintf(msg, "%.1fV", voltage);
#ifdef MINI_BATTLABEL
  CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
#endif
  _display.setCursor(X_BATT_ICON + W_BATT_ICON/2, 
                     Y_BATT_ICON + H_BATT_ICON + 2);
  _display.printCentreJustified(msg);

  // nominal 10.5 -> 13.5V bargraph
  int Capacity = (voltage - 10.7) * 4;
  if(Capacity < 0)   Capacity = 0;
  if(Capacity > 11)  Capacity = 11;
  _display.fillRect(X_BATT_ICON+2 + Capacity, Y_BATT_ICON+2, W_BATT_ICON-4-Capacity, 6, BLACK);
}


