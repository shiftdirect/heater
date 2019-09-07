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

#include "128x64OLED.h"
#include "HomeMenuSelScreen.h"
#include "KeyPad.h"
#include "fonts/Icons.h"



CHomeMenuSelScreen::CHomeMenuSelScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
}

void 
CHomeMenuSelScreen::onSelect()
{
  CScreenHeader::onSelect();
  _rowSel = 0;
  _action = NVstore.getUserSettings().HomeMenu;
}

void
CHomeMenuSelScreen::_initUI()
{
}

bool 
CHomeMenuSelScreen::show()
{
  char msg[16];

  _display.clearDisplay();

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

    if(_rowSel == 4) {
      _showConfirmMessage();
    }
    else {
      _showTitle("Home Menu Actions");
      
      _drawBitmap(30, 14, TimeoutIconInfo);
      switch(_action.onTimeout) {
        case 0: strcpy(msg, "Default"); break;
        case 1: strcpy(msg, "Detailed"); break;
        case 2: strcpy(msg, "Basic"); break;
        case 3: strcpy(msg, "Clock"); break;
      }
      _printMenuText(50, 14, msg, _rowSel == 3);

      _drawBitmap(32, 26, StartIconInfo);
      switch(_action.onStart) {
        case 0: strcpy(msg, "Default"); break;
        case 1: strcpy(msg, "Detailed"); break;
        case 2: strcpy(msg, "Basic"); break;
        case 3: strcpy(msg, "Clock"); break;
      }
      _printMenuText(50, 26, msg, _rowSel == 2);

      _drawBitmap(31, 38, StopIconInfo);
      switch(_action.onStop) {
        case 0: strcpy(msg, "Default"); break;
        case 1: strcpy(msg, "Detailed"); break;
        case 2: strcpy(msg, "Basic"); break;
        case 3: strcpy(msg, "Clock"); break;
      }
      _printMenuText(50, 38, msg, _rowSel == 1);
    }
  }
  return true;
}

bool 
CHomeMenuSelScreen::animate()
{
  if(!CPasswordScreen::_busy()) {
    if(_rowSel != 4) {
      int yPos = 53;
      int xPos = _display.xCentre();
      const char* pMsg = NULL;
      switch(_rowSel) {
        case 0:
          _printMenuText(xPos, yPos, " \021  \030Edit  Exit   \020 ", true, eCentreJustify);
          break;
        case 1:
          _display.drawFastHLine(0, 52, 128, WHITE);
          pMsg = "                    Menu to switch to when the heater stops.                    ";
          _scrollMessage(56, pMsg, _scrollChar);
          break;
        case 2:
          _display.drawFastHLine(0, 52, 128, WHITE);
          pMsg = "                    Menu to switch to when the heater starts.                    ";
          _scrollMessage(56, pMsg, _scrollChar);
          break;
        case 3:
          _display.drawFastHLine(0, 52, 128, WHITE);
          pMsg = "                    Menu to return to after no keypad activity.                    ";
          _scrollMessage(56, pMsg, _scrollChar);
          break;
      }
      return true;
    }
  }
  return false;
}


bool 
CHomeMenuSelScreen::keyHandler(uint8_t event)
{
  sUserSettings us;
  if(event & keyPressed) {
    // UP press
    if(event & key_Up) {
      if(_rowSel == 4) {
        _enableStoringMessage();
        us = NVstore.getUserSettings();
        us.HomeMenu = _action;
        NVstore.setUserSettings(us);
        saveNV();
        _rowSel = 0;
      }
      else {
        _scrollChar = 0;
        _rowSel++;
        UPPERLIMIT(_rowSel, 3);
      }
    }
    // DOWN press
    if(event & key_Down) {
      _scrollChar = 0;
      _rowSel--;
      LOWERLIMIT(_rowSel, 0);
    }
    // CENTRE press
    if(event & key_Centre) {
      if(_rowSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
      }
      else {
        _rowSel = 4;
      }
    }
    // LEFT press
    if(event & key_Left) {
      if(_rowSel == 0)
        _ScreenManager.prevMenu();
      else 
        adjust(-1);
    }
    // RIGHT press
    if(event & key_Right) {
      if(_rowSel == 0)
        _ScreenManager.nextMenu();
      else 
        adjust(+1);
    }
  }

  _ScreenManager.reqUpdate();

  return true;
}

void
CHomeMenuSelScreen::adjust(int dir)
{
  switch(_rowSel) {
    case 1:
      _action.onStop += dir;
      WRAPLIMITS(_action.onStop, 0, 3);
      break;
    case 2:
      _action.onStart += dir;
      WRAPLIMITS(_action.onStart, 0, 3);
      break;
    case 3: 
      _action.onTimeout += dir;
      WRAPLIMITS(_action.onTimeout, 0, 3);
      break;
  }
}



CNoHeaterHomeMenuSelScreen::CNoHeaterHomeMenuSelScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
}

void 
CNoHeaterHomeMenuSelScreen::onSelect()
{
  CScreenHeader::onSelect();
  _rowSel = 0;
  _action = NVstore.getUserSettings().HomeMenu;
  _dispTimeout = NVstore.getUserSettings().dimTime; 
  _menuTimeout = NVstore.getUserSettings().menuTimeout; 
  if(_action.onTimeout == 0)
    _action.onTimeout = 2;
}

void
CNoHeaterHomeMenuSelScreen::_initUI()
{
}

bool 
CNoHeaterHomeMenuSelScreen::show()
{
  char msg[16];

  _display.clearDisplay();

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

    if(_rowSel == 4) {
      _showConfirmMessage();
    }
    else {
      _showTitle("Home Menu Actions");
      
      _drawBitmap(22, 14, TimeoutIconInfo);
      switch(_action.onTimeout) {
        case 2: strcpy(msg, "Temperature"); break;
        case 3: strcpy(msg, "Clock"); break;
      }
      _printMenuText(40, 14, msg, _rowSel == 3);

      // display timeout
      _drawBitmap(10, 26, DisplayTimeoutIconInfo);
      if(_dispTimeout) {
        float mins = float(abs(_dispTimeout)) / 60000.f;
        sprintf(msg, "%s %0.1f min%s",  (_dispTimeout < 0) ? "Blank" : "Dim", mins, mins < 2 ? "" : "s");
        _printMenuText(40, 26, msg, _rowSel == 2);
      }
      else 
        _printMenuText(40, 26, "Always on", _rowSel == 2);

      // menu timeout
      _drawBitmap(10, 38, MenuTimeoutIconInfo);
      if(_menuTimeout) {
        float mins = float(abs(_menuTimeout)) / 60000.f;
        sprintf(msg, "Home %0.1f min%s", mins, mins < 2 ? "" : "s");
        _printMenuText(40, 38, msg, _rowSel == 1);
      }
      else 
        _printMenuText(40, 38, "Disabled", _rowSel == 1);

    }
  }
  return true;
}

bool 
CNoHeaterHomeMenuSelScreen::animate()
{
  if(!CPasswordScreen::_busy()) {
    if(_rowSel != 4) {
      int yPos = 53;
      int xPos = _display.xCentre();
      const char* pMsg = NULL;
      switch(_rowSel) {
        case 0:
          _printMenuText(xPos, yPos, " \021  \030Edit  Exit   \020 ", true, eCentreJustify);
          break;
        case 1:
          _display.drawFastHLine(0, 52, 128, WHITE);
          pMsg = "                    No keypad activity returns to the home menu.                    ";
          _scrollMessage(56, pMsg, _scrollChar);
          break;
        case 2:
          _display.drawFastHLine(0, 52, 128, WHITE);
          pMsg = "                    No keypad activity either dims or blanks the display. Hold Left or Right to toggle Dim/Blank mode.                    ";
          _scrollMessage(56, pMsg, _scrollChar);
          break;
        case 3:
          _display.drawFastHLine(0, 52, 128, WHITE);
          pMsg = "                    Menu to return to after no keypad activity.                    ";
          _scrollMessage(56, pMsg, _scrollChar);
          break;
      }
      return true;
    }
  }
  return false;
}


bool 
CNoHeaterHomeMenuSelScreen::keyHandler(uint8_t event)
{
  sUserSettings us;
  if(event & keyPressed) {
    // UP press
    if(event & key_Up) {
      if(_rowSel == 4) {
        _enableStoringMessage();
        us = NVstore.getUserSettings();
        us.HomeMenu = _action;
        us.menuTimeout = _menuTimeout;
        us.dimTime = _dispTimeout;
        NVstore.setUserSettings(us);
        saveNV();
        _rowSel = 0;
      }
      else {
        _scrollChar = 0;
        _rowSel++;
        UPPERLIMIT(_rowSel, 3);
      }
    }
    // DOWN press
    if(event & key_Down) {
      _scrollChar = 0;
      _rowSel--;
      LOWERLIMIT(_rowSel, 0);
    }
    // CENTRE press
    if(event & key_Centre) {
      if(_rowSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
      }
      else {
        _rowSel = 4;
      }
    }
    // LEFT press
    if(event & key_Left) {
      if(_rowSel == 0)
        _ScreenManager.prevMenu();
      else 
        adjust(-1);
    }
    // RIGHT press
    if(event & key_Right) {
      if(_rowSel == 0)
        _ScreenManager.nextMenu();
      else 
        adjust(+1);
    }
  }

  _ScreenManager.reqUpdate();

  return true;
}

void
CNoHeaterHomeMenuSelScreen::adjust(int dir)
{
  switch(_rowSel) {
    case 1: 
      _menuTimeout += dir * 30000;
      LOWERLIMIT(_menuTimeout, 0);
      UPPERLIMIT(_menuTimeout, 300000);
      break;
    case 2: 
      _dispTimeout += dir * 30000;
      LOWERLIMIT(_dispTimeout, -600000);
      UPPERLIMIT(_dispTimeout, 600000);
      break;
    case 3: 
      _action.onTimeout = _action.onTimeout==3 ? 2 : 3;
      break;
  }
}
