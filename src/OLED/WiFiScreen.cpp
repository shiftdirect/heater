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
#include "../Utility/helpers.h"
#include "../WiFi/BTCWifi.h"
#include "../Utility/NVStorage.h"
#include "fonts/Arial.h"

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

CWiFiScreen::CWiFiScreen(C128x64_OLED& display, CScreenManager& mgr) : CUIEditScreen(display, mgr) 
{
  _initUI();
}

void
CWiFiScreen::onSelect()
{
  CScreen::onSelect();
  _initUI();
}

void
CWiFiScreen::_initUI()
{
  CUIEditScreen::_initUI();

  _OTAsel = NVstore.getUserSettings().enableOTA;
  _bShowMAC = false;

  if(NVstore.getUserSettings().wifiMode) {
    if(isWifiAP()) {
      if(isWifiConfigPortal()) {
        _colSel = 1;  // " WiFi: CFG AP only "
      }
      else {
        _colSel = 2;  //  " WiFi: AP only ";
      }
    }
    else {
      if(isWifiConfigPortal()) {
        _colSel = 3;  // " WiFi: CFG STA+AP "
      }
      else {
        _colSel = 4;  //  " WiFi: STA+AP ";
      }
    }
  }
}

bool 
CWiFiScreen::show()
{
//  CScreenHeader::show(false);
  CScreen::show();
  
  _display.clearDisplay();
  _showTitle("WiFi settings");

  int yPos = 18;
    
  const char* pTitle = NULL;
  switch(_colSel) {
    case 0:
      pTitle = "DISABLED";
      break;
    case 1:
      pTitle = "CFG AP only";
      break;
    case 2:
      pTitle = "AP only";
      break;
    case 3:
      pTitle = "CFG STA+AP";
      break;
    case 4:
      pTitle = "STA+AP";
      break;
  }
    
  _printMenuText(border, yPos, pTitle, _rowSel==1);   // selection box
  if(_OTAsel == 0)
    _printMenuText(128-border, yPos, "OTA: OFF", _rowSel==2, eRightJustify);   // selection box
  else
    _printMenuText(128-border, yPos, "OTA: ON ", _rowSel==2, eRightJustify);   // selection box
  yPos += 3;

  if(_colSel) {
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

  return true;
}

bool
CWiFiScreen::animate()
{
  // show next/prev menu navigation line
  if(_rowSel == 0) {
//    _printMenuText(_display.xCentre(), 53, "                    ", true, eCentreJustify);
    _printMenuText(_display.xCentre(), 53, "\021                  \020", true, eCentreJustify);
    if(_bShowMAC) {
      _printMenuText(_display.xCentre(), 53, "\030Mode        \031IP", false, eCentreJustify);
      _printMenuText(_display.xCentre(), 53, "  Exit", false, eCentreJustify);
    }
    else {
      _printMenuText(_display.xCentre(), 53, "\030Mode       \031MAC", false, eCentreJustify);
      _printMenuText(_display.xCentre(), 53, " Exit", false, eCentreJustify);
    }
  }
  if(_rowSel == 1) {
    _display.drawFastHLine(0, 52, 128, WHITE);
    const char* pMsg = NULL;
    pMsg = "\031ESC \030OTA  Set  \033\032Adj";  // both Sel arrows
    _printMenuText(_display.xCentre(), 56, pMsg, false, eCentreJustify);
  }
  if(_rowSel == 2) {
    _display.drawFastHLine(0, 52, 128, WHITE);
    const char* pMsg = NULL;
    pMsg = "\031Mode   Set   \033\032Adj";  
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
    // press LEFT 
    if(event & key_Left) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.prevMenu(); 
          break;
        case 1:
          if(isWifiAP()) {
            _colSel--;
            WRAPLOWERLIMIT(_colSel, 0, 2);
          }
          else {
            _colSel--;
            WRAPLOWERLIMIT(_colSel, 0, 4);
          }
          break;
        case 2:
          _OTAsel--;
          WRAPLOWERLIMIT(_OTAsel, 0, 1);
          break;
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.nextMenu(); 
          break;
        case 1:
          if(isWifiAP()) {
            _colSel++;
            WRAPUPPERLIMIT(_colSel, 2, 0);
          }
          else {
            _colSel++;
            WRAPUPPERLIMIT(_colSel, 4, 0);
          }
          break;
        case 2:
          _OTAsel++;
          WRAPUPPERLIMIT(_OTAsel, 1, 0);
          break;
      }
    }
    // press UP
    if(event & key_Up) {
      _rowSel++;
      UPPERLIMIT(_rowSel, 2);
    }
    // press DOWN
    if(event & key_Down) {
      if(_rowSel == 0) {
        _bShowMAC = !_bShowMAC;   // toogle MAC/IP address if on navigation row
      }
      _rowSel--;
      LOWERLIMIT(_rowSel, 0);
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
      if(_rowSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
      }
      if(_rowSel == 1) {

        switch(_colSel) {
          case 0:
            wifiDisable(5000);
            break;
          case 1:
            wifiEnterConfigPortal(true, true, 5000);    //  CFG AP: erase credentials, reboot into portal
            break;
          case 2:
            wifiEnterConfigPortal(false, true, 5000);   //  AP Only: erase credentials, reboot into webserver
            break;
          case 3:
            wifiEnterConfigPortal(true, false, 5000);   //  CFG STA+AP: keep credentials, reboot into portal
            break;
          case 4:
            wifiEnterConfigPortal(false, false, 5000);   //  STA+AP: keep credentials, reboot into webserver
            break;
        }
        _rowSel = 3;  // stop ticker display
      }
      if(_rowSel == 2) {
        sUserSettings settings = NVstore.getUserSettings();
        settings.enableOTA = _OTAsel;
        NVstore.setUserSettings(settings);
        NVstore.save();
        const char* content[2];
        if(_OTAsel)
          content[0] = "Enabling OTA";
        else
          content[0] = "Disabling OTA";
        content[1] = "";

        _ScreenManager.showRebootMsg(content, 5000);
      }
    }
    _repeatCount = 0;
  }
  return true;
}

