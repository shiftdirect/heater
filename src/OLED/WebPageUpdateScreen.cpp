/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2020  Ray Jones <ray@mrjones.id.au>
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

#include "128x64OLED.h"
#include "WebPageUpdateScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "fonts/Arial.h"
#include "../WiFi/BTCWifi.h"
#include "../WiFi/BTCWebServer.h"


CWebPageUpdateScreen::CWebPageUpdateScreen(C128x64_OLED& display, CScreenManager& mgr) : CUIEditScreen(display, mgr) 
{
  _holdoff = 0;
}

bool
CWebPageUpdateScreen::onSelect() 
{
  CScreen::onSelect();
  return true;
}


bool 
CWebPageUpdateScreen::show()
{
  
  _display.clearDisplay();

  // standard version information screens,
  // animation of update available via animate() if firmware update is available on web server
  _showTitle("Web Content Update");
  
  int col = _display.xCentre();
  if(_rowSel == 0) {
    if(isWifiSTAConnected()) {
      _printMenuText(col, 16, "Press Up to update", false, eCentreJustify);
      _printMenuText(col, 26, "web page content  ", false, eCentreJustify);
      _printMenuText(col, 36, "stored in SPIFFS. ", false, eCentreJustify);
    }
    else {
      _printMenuText(col, 16, "WiFi STA connection", false, eCentreJustify);
      _printMenuText(col, 26, "must be active to  ", false, eCentreJustify);
      _printMenuText(col, 36, "update web content!", false, eCentreJustify);
    }

    _printMenuText(_display.xCentre(), 53, " \021                \020 ", true, eCentreJustify);     // " <              > "
    _printMenuText(_display.xCentre(), 53, "Exit", false, eCentreJustify);     // " <     Exit     > "
  }
  else if(_rowSel == 1) {
    _display.writeFillRect(12, 21, 104, 26, WHITE);
    CTransientFont AF(_display, &arial_8ptBoldFontInfo);
    _printInverted(col, 24, "Press Center to", true, eCentreJustify);
    _printInverted(col, 34, "confirm update ", true, eCentreJustify);
  }
  else if(_rowSel == 2) {
    _printMenuText(col, 22, "Getting:", false, eCentreJustify);
    const char* filename = _getFileName();
    _printMenuText(col, 34, filename, false, eCentreJustify);
  }

  return true;
}

bool
CWebPageUpdateScreen::animate()
{
  int col = _display.xCentre();
  if(_rowSel == 2) {
    _printMenuText(col, 22, "Getting:", false, eCentreJustify);
    const char* filename = _getFileName();
    _display.fillRect(0,34,_display.width(), 10, BLACK);
    _printMenuText(col, 34, filename, false, eCentreJustify);
    return true;
  }  
  if(_rowSel == 3) {
    _display.writeFillRect(12, 21, 104, 26, WHITE);
    CTransientFont AF(_display, &arial_8ptBoldFontInfo);
    _printInverted(col, 29, "ERROR OCCURED", true, eCentreJustify);

    long tDelta = millis() - _holdoff;
    if(tDelta > 0) {
      _holdoff = 0;
      _rowSel = 0;
    }
    return true;
  }
  return false;
}

bool 
CWebPageUpdateScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // UP press
    if(event & key_Up) {
      if(_rowSel == 0) {
        if(isWifiSTAConnected()) 
          _rowSel = 1;
      }
      else {
        _rowSel = 0;
      }
    }
    // DOWN press
    if(event & key_Down) {
      _rowSel = 0;
    }
    // LEFT press
    if(event & key_Left) {
      if(_rowSel == 0) 
        _ScreenManager.prevMenu();
      _rowSel = 0;
    }
    // RIGHT press
    if(event & key_Right) {
      if(_rowSel == 0) 
        _ScreenManager.nextMenu();
      _rowSel = 0;
    }
    // CENTRE press
    if(event & key_Centre) {
      if(_rowSel == 1) {
        getWebContent(true);
        _rowSel = 2;
      }
      else {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
      }
    }
  }

  _ScreenManager.reqUpdate();

  return true;
}

const char* 
CWebPageUpdateScreen::_getFileName()
{
  const char* livename = getWebContent(false);

  if(strcmp(livename, "DONE") == 0) {
    _rowSel = 0;
  }
  if(strcmp(livename, "ERROR") == 0) {
    _holdoff = (millis() + 2000) | 1;
    _rowSel = 3;
  }

  return livename;
}
