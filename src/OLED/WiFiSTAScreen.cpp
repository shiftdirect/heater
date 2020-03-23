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

#include "WiFiSTAScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "../Utility/NVStorage.h"
#include "../WiFi/BTCWifi.h"
#include "fonts/Arial.h"
#include "fonts/Icons.h"

///////////////////////////////////////////////////////////////////////////
//
// CWiFiSTAScreen
//
// This screen presents information about the STA connection
//
///////////////////////////////////////////////////////////////////////////


CWiFiSTAScreen::CWiFiSTAScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreen(display, mgr) 
{
}


bool 
CWiFiSTAScreen::show()
{
  CScreen::show();
  
  _display.clearDisplay();
  _showTitle("WiFi STA status");

  int yPos = 15;
    
  if(NVstore.getUserSettings().wifiMode == 0 || !isWifiSTA()) {
    if(NVstore.getUserSettings().wifiMode == 0)
      _printMenuText(border, yPos, "DISABLED");   
    else
      _printMenuText(border, yPos, "NOT CONNECTED");   
  }
  else {
    _printMenuText(0, yPos, "ADDR:");
    _printMenuText(31, yPos, getWifiSTAAddrStr());

    yPos += _display.textHeight() + 3;
    _printMenuText(0, yPos, "  GW:");
    _printMenuText(31, yPos, getWifiGatewayAddrStr());
    
    // yPos += _display.textHeight() + 2;
    // _printMenuText(0, yPos, " MAC:");
    // _printMenuText(31, yPos, getWifiSTAMACStr());
    
    yPos += _display.textHeight() + 3;
    _printMenuText(0, yPos, "RSSI:");
    int8_t RSSI = getWifiRSSI();
    char RSSIstr[16];
    sprintf(RSSIstr, "%ddBm", RSSI);
    _printMenuText(31, yPos, RSSIstr);

    int xPos = 90;
    _drawBitmap(xPos, yPos, WifiIconInfo, WHITE, BLACK);  // wide icon erases annotations!
    if(RSSI < -70) {
      _display.fillRect(xPos, yPos, WifiIconInfo.width, 6, BLACK);
    }
    else if(RSSI < -55) {
      _display.fillRect(xPos, yPos, WifiWideIconInfo.width, 3, BLACK);
      _display.fillRect(xPos, yPos, 1, 4, BLACK);
      _display.fillRect(xPos+WifiIconInfo.width-1, yPos, 1, 4, BLACK);
    }
  }
    
  _printMenuText(_display.xCentre(), 53, "\021                  \020", true, eCentreJustify);
  _printMenuText(_display.xCentre(), 53, "Exit", false, eCentreJustify);
  return true;
}


bool 
CWiFiSTAScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // press LEFT 
    if(event & key_Left) {
      _ScreenManager.prevMenu(); 
    }
    // press RIGHT 
    if(event & key_Right) {
      _ScreenManager.nextMenu(); 
    }
    // press UP
    if(event & key_Up) {
    }
    // press DOWN
    if(event & key_Down) {
    }
    if(event & key_Centre) {
      _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
    }
    _ScreenManager.reqUpdate();
  }

  return true;
}

