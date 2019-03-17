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

#include "WiFiScreen.h"
#include "KeyPad.h"
#include "../Protocol/helpers.h"
#include "../Wifi/BTCWifi.h"
#include "../Utility/NVstorage.h"

///////////////////////////////////////////////////////////////////////////
//
// CWiFiScreen
//
// This screen presents sundry information
// eg WiFi status
//
///////////////////////////////////////////////////////////////////////////

#define STA_HOLD_TIME 10

static const int LIMIT_AWAY = 0;
static const int LIMIT_LEFT = 1;
static const int LIMIT_RIGHT = 2;

CWiFiScreen::CWiFiScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _initUI();
}

void
CWiFiScreen::onSelect()
{
  _initUI();
}

void
CWiFiScreen::_initUI()
{
  _rowSel = 0;
  _bShowMAC = false;

  if(isWifiAP()) {
    if(isWifiConfigPortal()) {
      _colSel = 0;  // " WiFi: CFG AP only "
      _colLimit = LIMIT_LEFT;   // left most selection
    }
    else {
      _colSel = 1;  //  " WiFi: AP only ";
      _colLimit = LIMIT_RIGHT;   // right most selection
    }
  }
  else {
    if(isWifiConfigPortal()) {
      _colSel = 2;  // " WiFi: CFG STA+AP "
      _colLimit = LIMIT_AWAY;   // away from menu limits
    }
    else {
      _colSel = 3;  //  " WiFi: STA+AP ";
      _colLimit = LIMIT_RIGHT;   // right most selection
    }
  }
}

bool 
CWiFiScreen::show()
{
  CScreenHeader::show();
  
  int yPos = 18;
  if(isWifiConnected() || isWifiAP()) {
    
    const char* pTitle = NULL;
    switch(_colSel) {
      case 0:
        pTitle = " WiFi: CFG AP only ";
        break;
      case 1:
        pTitle = " WiFi: AP only ";
        break;
      case 2:
        pTitle = " WiFi: CFG STA+AP ";
        break;
      case 3:
        pTitle = " WiFi: STA+AP ";
        break;
    }
    
    if(_rowSel == 0) 
      _printInverted(3, yPos, pTitle, true);   // inverted title bar
    if(_rowSel == 1) 
      _printMenuText(3, yPos, pTitle, true);   // selection box
    yPos += 3;

    // only show STA IP address if available!
    if(isWifiSTA() && _repeatCount <= STA_HOLD_TIME) {
      yPos += _display.textHeight() + 2;
      _printMenuText(0, yPos, "STA:");
      if(_bShowMAC)
        _printMenuText(25, yPos, getWifiSTAMACStr());
      else
        _printMenuText(25, yPos, getWifiSTAAddrStr());
    }
    // show AP IP address
    yPos += _display.textHeight() + 2;
    _printMenuText(0, yPos, " AP:");
    if(_bShowMAC)
      _printMenuText(25, yPos, getWifiAPMACStr());
    else
      _printMenuText(25, yPos, getWifiAPAddrStr());
  }
  else {
    _printInverted(0, yPos, " WiFi Inactive ", true);
  }

  return true;
}

bool
CWiFiScreen::animate()
{
  bool retval = false;
  // show next/prev menu navigation line
  if(_rowSel == 0) {
    _printMenuText(_display.xCentre(), 53, " \021               \020 ", true, eCentreJustify);
    if(_bShowMAC)
      _printMenuText(_display.xCentre(), 53, "\030Sel  \031IP", false, eCentreJustify);
    else
      _printMenuText(_display.xCentre(), 53, "\030Sel  \031MAC", false, eCentreJustify);
  }
  if(_rowSel == 1) {
    _display.drawFastHLine(0, 52, 128, WHITE);
    const char* pMsg = NULL;
    switch(_colLimit) {
      case LIMIT_AWAY:
        pMsg = "\031 ESC   Set   \033\032 Sel";  // both Sel arrows
        break;
      case LIMIT_LEFT:
        pMsg = "\031 ESC   Set    \032 Sel";  // only right Sel arrow
        break;
      case LIMIT_RIGHT:
        pMsg = "\031 ESC   Set    \033 Sel";  // only left Sel arrow
        break;
    }
    if(pMsg)
      _printMenuText(_display.xCentre(), 56, pMsg, false, eCentreJustify);
  }
  CScreen::animate();
  return true;
}

bool 
CWiFiScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    _repeatCount = 0;
    // press CENTRE
    if(event & key_Centre) {
    }
    // press LEFT 
    if(event & key_Left) {
      if(_rowSel == 0) {
        _ScreenManager.prevMenu(); 
      }
      else {
        if(isWifiAP()) {
          _colSel = 0;
          _colLimit = LIMIT_LEFT;
        }
        else {
          _colSel--;
          LOWERLIMIT(_colSel, 0);
          _colLimit = (_colSel == 0) ? LIMIT_LEFT : LIMIT_AWAY;
        }
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      if(_rowSel == 0) {
        _ScreenManager.nextMenu(); 
      }
      else {
        if(isWifiAP()) {
          _colSel = 1;
          _colLimit = LIMIT_RIGHT;
        }
        else {
          _colSel++;
          UPPERLIMIT(_colSel, 3);
          _colLimit = (_colSel == 3) ? LIMIT_RIGHT : LIMIT_AWAY;
        }
      }
    }
    // press UP
    if(event & key_Up) {
      _rowSel = 1;
    }
    // press DOWN
    if(event & key_Down) {
      if(_rowSel == 0) {
        _bShowMAC = !_bShowMAC;   // toogle MAC/IP address if on navigation row
      }
      _rowSel = 0;
    }
    _ScreenManager.reqUpdate();
  }

  if(event & keyRepeat) {    // track key hold time
    if(event & key_Centre) {
      _repeatCount++;
    }
  }

  if(event & keyReleased) {
    if(event & key_Centre) {
      if(_rowSel) {

        switch(_colSel) {
          case 0:
            wifiEnterConfigPortal(true, true, 5000);    //  CFG AP: erase credentials, reboot into portal
            break;
          case 1:
            wifiEnterConfigPortal(false, true, 5000);   //  AP Only: erase credentials, reboot into webserver
            break;
          case 2:
            wifiEnterConfigPortal(true, false, 5000);   //  CFG STA+AP: keep credentials, reboot into portal
            break;
          case 3:
            wifiEnterConfigPortal(false, false, 5000);   //  STA+AP: keep credentials, reboot into webserver
            break;
        }
        _rowSel = 2;  // stop ticker display
      }
    }
    _repeatCount = 0;
  }
  return true;
}

