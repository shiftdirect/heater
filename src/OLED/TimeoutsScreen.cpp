/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2019  Ray Jones <ray@mrjones.id.au>
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
#include "TimeoutsScreen.h"
#include "KeyPad.h"
#include "../Utility/NVStorage.h"
#include "fonts/Icons.h"



CTimeoutsScreen::CTimeoutsScreen(C128x64_OLED& display, CScreenManager& mgr) : CUIEditScreen(display, mgr) 
{
}

void 
CTimeoutsScreen::onSelect()
{
  CUIEditScreen::onSelect();
  _repeatCount = -1;
  _frameRate = NVstore.getUserSettings().FrameRate; 
  _dispTimeout = NVstore.getUserSettings().dimTime; 
  _menuTimeout = NVstore.getUserSettings().menuTimeout; 
}

bool 
CTimeoutsScreen::show()
{
  char msg[16];

  _display.clearDisplay();

  if(!CUIEditScreen::show()) {  // for showing "saving settings"

    _showTitle("Time Intervals");
    
    // data frame refresh rate
    _drawBitmap(15, 13, RefreshIconInfo);
    sprintf(msg, "%dms", _frameRate);
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
  return true;
}

bool 
CTimeoutsScreen::animate()
{
  if(_saveBusy()) {
    return false;
  }

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
      pMsg = "                    Define the polling rate of the bluewire communications.                    ";
      _scrollMessage(56, pMsg, _scrollChar);
      break;
  }
  return true;
}

bool 
CTimeoutsScreen::keyHandler(uint8_t event)
{
  if(CUIEditScreen::keyHandler(event)) {  // handle confirm save
    return true;
  }

  if(event & keyPressed) {
    _repeatCount = 0;

    // UP press
    if(event & key_Up) {
      _scrollChar = 0;
      _rowSel++;
      UPPERLIMIT(_rowSel, 3);
    }
    // UP press
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
        _confirmSave();   // enter save confirm mode
        _rowSel = 0;
      }
    }
  }

  if(event & keyRepeat) {
    if(keyRepeat >= 0) {
      _repeatCount++;
      if(_repeatCount > 4) {
        // LEFT or RIGHT hold
        if(event & (key_Right | key_Left)) {
          if(_rowSel == 2) {
            _repeatCount = -1;
            _dispTimeout = -_dispTimeout;
          }
        }
      }
    }
  }

  if(event & keyReleased) {
    if(_repeatCount == 0) {
      // LEFT short press
      if(event & key_Left) {
        if(_rowSel == 0)
          _ScreenManager.prevMenu();
        else 
          adjust(-1);
      }
      // RIGHT short press
      if(event & key_Right) {
        if(_rowSel == 0)
          _ScreenManager.nextMenu();
        else 
          adjust(+1);
      }
    }
  }

  _ScreenManager.reqUpdate();

  return true;
}

void
CTimeoutsScreen::adjust(int dir)
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
      _frameRate += dir * 50;
      LOWERLIMIT(_frameRate, 300);
      UPPERLIMIT(_frameRate, 1500);
      break;
  }
}

void
CTimeoutsScreen::_saveNV()
{
  sUserSettings settings = NVstore.getUserSettings();
  settings.dimTime = _dispTimeout;
  settings.menuTimeout = _menuTimeout;
  settings.FrameRate = _frameRate;
  NVstore.setUserSettings(settings);
  NVstore.save();
}
