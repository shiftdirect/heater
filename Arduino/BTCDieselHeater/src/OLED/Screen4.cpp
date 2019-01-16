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

#include "Screen4.h"
#include "KeyPad.h"
#include "../Protocol/helpers.h"
#include "../Wifi/BTCWifi.h"
// #include "fonts/Tahoma8.h"
// #include "fonts/FranklinGothic.h"
// #include "fonts/Arial.h"
#include "../Utility/NVstorage.h"

///////////////////////////////////////////////////////////////////////////
//
// CScreen4
//
// This screen presents sundry information
// eg WiFi status
//
///////////////////////////////////////////////////////////////////////////

#define STA_HOLD_TIME 10

CScreen4::CScreen4(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _rowSel = 0;
}


void 
CScreen4::show()
{
  CScreenHeader::show();
  
  int yPos = 16;
  if(isWifiConnected() || isWifiAP()) {
    
    const char* pTitle = NULL;
    if(isWifiAP()) 
      pTitle = isWifiConfigPortal() ? " WiFi: CFG AP only " : " WiFi: AP only ";
    else 
      pTitle = isWifiConfigPortal() ? " WiFi: CFG STA+AP " : " WiFi: STA+AP ";
    
    if(_rowSel == 1) 
      _printMenuText(3, yPos, pTitle, true);   // selection box
    else  
      _printInverted(3, yPos, pTitle, true);   // inverted title bar
    yPos += 3;

    // show AP IP address
    yPos += _display.textHeight() + 2;
    _printMenuText(0, yPos, " AP:");
    _printMenuText(25, yPos, getWifiAPAddrStr());
    // only show STA IP address if available!
    if(isWifiSTA() && _repeatCount <= STA_HOLD_TIME) {
      yPos += _display.textHeight() + 2;
      _printMenuText(0, yPos, "STA:");
      _printMenuText(25, yPos, getWifiSTAAddrStr());
    }
  }
  else {
    _printInverted(0, yPos, " WiFi Inactive ", true);
  }

/*  yPos += _display.textHeight() + 2;
  char msg[32];
  int mins = NVstore.getDimTime() / 60000;
  sprintf(msg, "Display Dim: %d min%c", mins, (mins > 1) ? 's' : ' ');
  _printMenuText(0, yPos, msg);*/
}


void 
CScreen4::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    _repeatCount = 0;
    // press CENTRE
    if(event & key_Centre) {
    }
    // press LEFT 
    if(event & key_Left) {
      _ScreenManager.prevScreen(); 
      _rowSel = 0;
    }
    // press RIGHT 
    if(event & key_Right) {
      _ScreenManager.nextScreen(); 
      _rowSel = 0;
    }
    // press UP
    if(event & key_Up) {
      _rowSel = 1;
      // _rowSel++;
      // UPPERLIMIT(_rowSel, (isWifiConfigPortal() ? 2 : 1));
    }
    // press DOWN
    if(event & key_Down) {
      _rowSel = 0;
      // _rowSel--;
      // UPPERLIMIT(_rowSel, 0);
    }
    _ScreenManager.reqUpdate();
  }

  if(event & keyRepeat) {    // track key hold time
    _repeatCount++;
  }

  if(event & keyReleased) {
    if(event & key_Centre) {
      if(_rowSel) {

        if(_repeatCount > STA_HOLD_TIME) {
          wifiEnterConfigPortal(true, _repeatCount > STA_HOLD_TIME, 5000);    // press - reboot into portal, long press - erase credentials
        }
        else {
          if(isWifiConfigPortal()) {
            wifiEnterConfigPortal(false, false, 5000);  // stop config portal, reboot
          }
          else {
            wifiEnterConfigPortal(true, false, 5000);  // stop config portal, reboot
          }
        }
      }
    }
    _repeatCount = 0;
  }
}

