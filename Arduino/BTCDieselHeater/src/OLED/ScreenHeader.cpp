/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2018  Ray Jones <ray@mrjones.id.au>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */

#include <Arduino.h>
#include "ScreenHeader.h"
#include "../Protocol/Protocol.h"
#include "../Protocol/helpers.h"
#include "../Wifi/BTCWifi.h"
#include "../Bluetooth/BluetoothAbstract.h" 
#include "../Utility/NVStorage.h"
#include "../RTC/Clock.h"
#include "fonts/Arial.h"
#include "fonts/Icons.h"
#include "fonts/MiniFont.h"
#include "../RTC/TimerManager.h"


#define MINIFONT miniFontInfo

#define X_BT_ICON      10
#define Y_BT_ICON       0
#define X_WIFI_ICON    19
#define Y_WIFI_ICON     0
#define X_CLOCK        50  
#define Y_CLOCK         0
#define X_TIMER_ICON   83
#define Y_TIMER_ICON    0
#define X_BATT_ICON   103
#define Y_BATT_ICON     0

/*#define X_BT_ICON      20
#define Y_BT_ICON       0
#define X_WIFI_ICON    29
#define Y_WIFI_ICON     0
#define X_GPIO_ICON     9
#define X_CLOCK        56  
#define Y_CLOCK         0
#define X_TIMER_ICON   84
#define Y_TIMER_ICON    0
#define X_BATT_ICON   103
#define Y_BATT_ICON     0*/


CScreenHeader::CScreenHeader(C128x64_OLED& disp, CScreenManager& mgr) : CScreen(disp, mgr)
{
  _clearUpAnimation = false;
  _clearDnAnimation = false;
  _colon = false;
}

bool 
CScreenHeader::show()
{
  _display.clearDisplay();

  // standard header items
  // Bluetooth
  showBTicon();

  // WiFi
  showWifiIcon();

  // battery
  showBatteryIcon(getHeaterInfo().getBattVoltage());

  // timers
  int numTimers = showTimers();

//  // GPIO
//  showGPIO();

  // clock
  showTime(numTimers);

  return true;
}

// Animate IN/OUT arrows against the WiFi icon, according to actual web server traffic:
//   an IN (down) arrow is drawn if incoming data has been detected.
//   an OUT (up) arrow is drawn if outgoing data has been sent.
//
// Each arrow is drawn for one animation interval with a minimum of one clear interval 
// creating a clean flash on the display.
// Both arrows may appear in the same interval.
// The following is a typical sequence, relative to animation ticks, note the gap
// that always appears in the animation interval between either arrow shown:
//   
//    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
// _________^^^^^________________________________________^^^^^_________________________
// ______________vvvvv_____vvvvv_______________vvvvv_____vvvvv_____vvvvv_______________

bool 
CScreenHeader::animate()
{
  bool retval = false;
  if((isWifiConnected() || isWifiAP()) && isWebClientConnected()) {

    int xPos = X_WIFI_ICON + W_WIFI_ICON;
    if(isWifiAP()) {
      xPos += 4;
    }
    
    // UP arrow animation
    //
    int yPos = 0;
    if(_clearUpAnimation) { 
      // arrow was drawn in the prior iteration, now erase it 
      _display.fillRect(xPos, yPos, W_WIFIIN_ICON, H_WIFIIN_ICON, BLACK);
      retval = true;
      _clearUpAnimation = false;
    }
    else if(hasWebServerSpoken(true)) {
      // we have emitted data to the web client, show an UP arrow
      _display.drawBitmap(xPos, yPos, wifiOutIcon, W_WIFIIN_ICON, H_WIFIIN_ICON, WHITE);
      _clearUpAnimation = true;  // clear arrow upon next iteration
      retval = true;
    }
    
    // DOWN arrow animation
    //
    yPos = H_WIFI_ICON - H_WIFIIN_ICON + 1;
    if(_clearDnAnimation) { 
      // arrow was drawn in the prior iteration, now erase it 
      _display.fillRect(xPos, yPos, W_WIFIOUT_ICON, H_WIFIOUT_ICON, BLACK);
      retval = true;
      _clearDnAnimation = false;
    }
    else if(hasWebClientSpoken(true)) {
      // we have receievd data from the web client, show an DOWN arrow
      _display.drawBitmap(xPos, yPos, wifiInIcon, W_WIFIOUT_ICON, H_WIFIOUT_ICON, WHITE);
      _clearDnAnimation = true;  // clear arrow upon next iteration
      retval = true;
    }
  }
  return retval;                 // true if we need to update the physical display
}

void 
CScreenHeader::showBTicon()
{
  if(getBluetoothClient().isConnected()) {
    _display.drawBitmap(X_BT_ICON, Y_BT_ICON, BTicon, W_BT_ICON, H_BT_ICON, WHITE);
  }
}

void 
CScreenHeader::showWifiIcon()
{
  if(isWifiConnected() || isWifiAP()) {
    _display.drawBitmap(X_WIFI_ICON, Y_WIFI_ICON, wifiIcon, W_WIFI_ICON, H_WIFI_ICON, WHITE);
    if(isWifiButton()) {
      _display.fillRect(X_WIFI_ICON + 11, Y_WIFI_ICON + 5, 15, 7, BLACK);
      CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
      _display.setCursor(X_WIFI_ICON+12, Y_WIFI_ICON+6);
      switch(isWifiButton()) {
        case 1:  _display.print("CFG"); break;
        case 2:  _display.print("HTR"); break;
        case 3:  _display.print("ERS"); break;
      }
    }
    else if(isWifiConfigPortal()) {
      _display.fillRect(X_WIFI_ICON + 11, Y_WIFI_ICON + 5, 15, 7, BLACK);
      CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
      _display.setCursor(X_WIFI_ICON+12, Y_WIFI_ICON+6);
//      _display.print("PTL");
      _display.print("CFG");
    }
    else if(isWifiAP()) {
      _display.fillRect(X_WIFI_ICON + 11, Y_WIFI_ICON + 5, 10, 7, BLACK);
      CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
      _display.setCursor(X_WIFI_ICON+12, Y_WIFI_ICON+6);
      _display.print("AP");
    }
    if(NVstore.getOTAEnabled()) {
      _display.fillRect(X_WIFI_ICON +11, Y_WIFI_ICON, 14, 6, BLACK);
      CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
      _display.setCursor(X_WIFI_ICON+12, Y_WIFI_ICON);
      _display.print("OTA");
    }
  }
}

void
CScreenHeader::showBatteryIcon(float voltage)
{
  _display.drawBitmap(X_BATT_ICON, Y_BATT_ICON, BatteryIcon, W_BATT_ICON, H_BATT_ICON, WHITE);
  char msg[16];
  sprintf(msg, "%.1fV", voltage);
  CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
  _display.setCursor(X_BATT_ICON + W_BATT_ICON/2, 
                     Y_BATT_ICON + H_BATT_ICON + 2);
  _display.printCentreJustified(msg);

  // nominal 10.5 -> 13.5V bargraph
  int Capacity = (voltage - 10.7) * 4;
  if(Capacity < 0)   Capacity = 0;
  if(Capacity > 11)  Capacity = 11;
  _display.fillRect(X_BATT_ICON+2 + Capacity, Y_BATT_ICON+2, W_BATT_ICON-4-Capacity, 6, BLACK);
}

int
CScreenHeader::showTimers()
{
  int nextTimer = CTimerManager::getNextTimer();
  if(nextTimer) {
    int xPos = X_TIMER_ICON;   
    _display.drawBitmap(xPos, Y_TIMER_ICON, largeTimerIcon, W_TIMER_ICON, H_TIMER_ICON, WHITE);
    if(nextTimer & 0x80) 
      _display.drawBitmap(xPos-3, Y_TIMER_ICON, verticalRepeatIcon, verticalRepeatWidthPixels, verticalRepeatHeightPixels, WHITE);

    CTransientFont AF(_display, &miniFontInfo);  // temporarily use a mini font
    if((nextTimer & 0x0f) >= 10) 
      _display.setCursor(xPos+4, Y_TIMER_ICON+8);
    else
      _display.setCursor(xPos+6, Y_TIMER_ICON+8);
    _display.print(nextTimer & 0x0f);
    return 1;
  }
  return 0;
}



void 
CScreenHeader::showTime(int numTimers)
{
  const BTCDateTime& now = Clock.get();

  char msg[16];
  if(now.day() == 0xA5) {
    sprintf(msg, "No RTC");    
  }
  else {
    if(_colon)
      sprintf(msg, "%02d:%02d", now.hour(), now.minute());
    else
      sprintf(msg, "%02d %02d", now.hour(), now.minute());
    _colon = !_colon;
  }

  {
    CTransientFont AF(_display, &arial_8ptFontInfo);
    // determine centre position of remaining real estate
    int xPos = X_WIFI_ICON + W_WIFI_ICON + W_WIFIIN_ICON;  // rhs of wifi conglomeration
    if(isWifiAP())  xPos += 4;                             // add more if an Access Point
    
/*    switch(numTimers) {
      case 0: xPos = _display.xCentre(); break;
      case 1: xPos += (X_TIMER_ICON - xPos) / 2; break;
    }
    _printMenuText(xPos, Y_CLOCK, msg, false, eCentreJustify);*/
    _printMenuText(X_CLOCK, Y_CLOCK, msg);
  }
}

void 
CScreenHeader::showGPIO()
{
/*  int xPos = X_GPIO_ICON;   // both are enabled - draw icon 1 to the left, otherwise leave to the right
  CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
  _display.setCursor(xPos, 0);
  _display.print("1");
  _display.drawBitmap(xPos + 4, 0, getGPIO(0) ? TickIcon : CrossIcon, TickIconWidth, TickIconHeight, WHITE);
  _display.setCursor(xPos, 6);
  _display.print("2");
  _display.drawBitmap(xPos + 4, 6, getGPIO(1) ? TickIcon : CrossIcon, TickIconWidth, TickIconHeight, WHITE);*/
}
/*void 
CScreenHeader::showGPIO()
{
  int xPos = X_GPIO_ICON;   // both are enabled - draw icon 1 to the left, otherwise leave to the right
  _display.drawBitmap(xPos, 0, getGPIO(0) ? GPIO1ONIcon : GPIO1OFFIcon, GPIOIconWidthPixels, GPIOIconHeightPixels, WHITE);
  _display.drawBitmap(xPos, 8, getGPIO(1) ? GPIO2ONIcon : GPIO2OFFIcon, GPIOIconWidthPixels, GPIOIconHeightPixels, WHITE);
}*/
