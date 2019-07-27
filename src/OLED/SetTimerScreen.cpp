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


///////////////////////////////////////////////////////////////////////////
//
// CSetTimerScreen
//
// This screen allows the timers to be adjusted
//
///////////////////////////////////////////////////////////////////////////

#include "SetTimerScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "../../lib/RTClib/RTClib.h"
#include "../RTC/TimerManager.h"
#include "fonts/Arial.h"

const char* briefDOW[] = { "S", "M", "T", "W", "T", "F", "S" };

CSetTimerScreen::CSetTimerScreen(C128x64_OLED& display, CScreenManager& mgr, int instance) : CScreen(display, mgr) 
{
  _initUI();
  _ConflictTime = 0;
  _conflictID = 0;
  _timerID = instance;
}

void 
CSetTimerScreen::onSelect()
{
  CScreen::onSelect();
  _initUI();
  NVstore.getTimerInfo(_timerID, _timerInfo);
}

void 
CSetTimerScreen::_initUI()
{
  _rowSel = 0;
  _colSel = 0;
  _SaveTime = 0;
}

bool 
CSetTimerScreen::show()
{
  CScreen::show();

  _display.clearDisplay();

  char str[20];
  int xPos, yPos;

  if(_rowSel == 0) {
    NVstore.getTimerInfo(_timerID, _timerInfo);
  }
  sprintf(str, "Set Timer #%d", _timerID + 1);
  _showTitle(str);

  if(_SaveTime) {
    _showStoringMessage();
    long tDelta = millis() - _SaveTime;
    if(tDelta > 0) {
      _SaveTime = 0;
    }
  }
  else if(_ConflictTime) {
    long tDelta = millis() - _ConflictTime;
    if(tDelta > 0) 
      _ConflictTime = 0;
    sprintf(str, " with Timer %d ", _conflictID);
    _showConflict(str);
  }
  else {
    // start
    xPos = 18;
    yPos = 28;
    _printMenuText(xPos, yPos, "On", false, eRightJustify);
    _printMenuText(xPos+18, yPos, ":");
    xPos += 6;
    sprintf(str, "%02d", _timerInfo.start.hour);
    _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==0);
    xPos += 17;
    sprintf(str, "%02d", _timerInfo.start.min);
    _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==1);

    // stop
    xPos = 18;
    yPos = 41;
    _printMenuText(xPos, yPos, "Off", false, eRightJustify);
    _printMenuText(xPos+18, yPos, ":");
    xPos += 6;
    sprintf(str, "%02d", _timerInfo.stop.hour);
    _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==2);
    xPos += 17;
    sprintf(str, "%02d", _timerInfo.stop.min);
    _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==3);
    
    // control
    const char* msg;
    xPos = _display.width() - border;
    _printEnabledTimers();
    
    yPos = 41;
    if(_timerInfo.repeat)
      msg = "Repeat";
    else
      msg = "Once";
    if(_rowSel == 1) 
      _printMenuText(xPos, yPos, msg, _colSel==5, eRightJustify);
    else
      _printInverted(xPos, yPos, msg, _timerInfo.repeat, eRightJustify);

    // navigation line
    yPos = 53;
    xPos = _display.xCentre();
    if(_rowSel == 2) {
      _display.drawFastHLine(0, 53, 128, WHITE);
      _printMenuText(_display.xCentre(), 57, "\033\032 Sel         \030\031 Adj", false, eCentreJustify);
      _printMenuText(_display.xCentre(), 57, "Done", false, eCentreJustify);
    }
    else if(_rowSel == 1) {
      _display.drawFastHLine(0, 53, 128, WHITE);
      _printMenuText(_display.xCentre(), 57, "\033\032 Sel         \030\031 Adj", false, eCentreJustify);
      _printMenuText(_display.xCentre(), 57, "Save", false, eCentreJustify);
    }
    else {
      _printMenuText(xPos, yPos, " \021     Exit     \020 ", _rowSel==0, eCentreJustify);
    }
  }

  return true;
}


bool 
CSetTimerScreen::keyHandler(uint8_t event)
{
  static bool bHeld = false;
  // handle initial key press
  if(event & keyPressed) {
    bHeld = false;
    // press CENTRE
    if(event & key_Centre) {
      if(_rowSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // exit: return to clock screen
      }
      else if(_rowSel == 2) {   // exit from per day settings
        _rowSel = 1;
        _colSel = 4;
      }
      else {  // in config fields, save new settings
        // test if the setting conflict with an already defined timer
        _conflictID = CTimerManager::conflictTest(_timerInfo);  
        if(_conflictID) {
          _timerInfo.enabled = 0;   // cancel enabled status
          _ConflictTime = millis() + 1500;
          _ScreenManager.reqUpdate();
          _rowSel = 1;
          _colSel = 4;   // select enable/disable 
        }
        else {
          _SaveTime = millis() + 1500;
          _ScreenManager.reqUpdate();
          _rowSel = 0;
          _colSel = 0;
        }
        CTimerManager::setTimer(_timerInfo);
      }
    }
    // press LEFT - navigate fields, or screens
    if(event & key_Left) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.prevMenu(); 
          break;
        case 1:
          // select previous field
          _colSel--;
          WRAPLOWERLIMIT(_colSel, 0, 5);
          break;
        case 2:
          // select previous day
          _colSel--;
          WRAPLOWERLIMIT(_colSel, 0, 6);
          break;
      }
    }
    // press RIGHT - navigate fields, or screens
    if(event & key_Right) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.nextMenu(); 
          break;
        case 1:
          // select next field
          _colSel++;
          WRAPUPPERLIMIT(_colSel, 5, 0);
          break;
        case 2:
          // select next day
          _colSel++;
          WRAPUPPERLIMIT(_colSel, 6, 0);
          break;
      }
    }
  }

  // handle held down keys
  if(event & keyRepeat) {
    bHeld = true;
    if(_rowSel == 1) {
      if(_colSel < 4) {
        // fast repeat of hour/minute adjustments by holding up or down keys
        if(event & key_Down) _adjust(-1);
        if(event & key_Up) _adjust(+1);
      }
      else if(_colSel == 4) {
        if(event & (key_Up | key_Down)) {
          // enable per day programming by holding up or down
          _timerInfo.enabled &= 0x7f;   // strip next day flag
          _rowSel = 2;
          _colSel = 0;
        }
      }
    }
  }

  if(event & keyReleased) {
    if(!bHeld) {
      int maskDOW = 0x01 << _colSel;

      // released DOWN - can only leave adjustment by using OK (centre button)
      if(event & key_Down) {
        // adjust selected item
        switch(_rowSel) {
          case 1:
            if(!(_colSel == 4 && (_timerInfo.enabled & 0x7F) != 0)) {
              _adjust(-1);
            }
            else {
              // bump into per day setup
              _rowSel = 2;
              _colSel = 0;
            }
            break;
          case 2:
            // adjust selected item
            _timerInfo.enabled ^= maskDOW;
            _timerInfo.enabled &= 0x7f;
            break;
        }
      }
      // released UP 
      if(event & key_Up) {
        switch(_rowSel) {
          case 0:
            _rowSel = 1;
            _colSel = 0;
            break;
          case 1:
            // prevent accidentally losing per day settings
            if(!(_colSel == 4 && (_timerInfo.enabled & 0x7F) != 0)) {
              _adjust(+1);   // adjust selected item, unless in per day mode
            }
            else {
              // bump into per day setup
              _rowSel = 2;   
              _colSel = 0;
            }
            break;
          case 2:
            // adjust selected item
            _timerInfo.enabled ^= maskDOW;
            _timerInfo.enabled &= 0x7f;
            break;
        }
      }
    }
  }

  _ScreenManager.reqUpdate();
  return true;
}


void 
CSetTimerScreen::_adjust(int dir)
{
  int maskDOW = 0x01 << _colSel;  // if doing Day of Week - (_rowSel == 2)  

  switch(_colSel) {
    case 0:
      _timerInfo.start.hour += dir;
      WRAPLIMITS(_timerInfo.start.hour, 0, 23);
      break;
    case 1:
      _timerInfo.start.min += dir;
      WRAPLIMITS(_timerInfo.start.min, 0, 59);
      break;
    case 2:
      _timerInfo.stop.hour += dir;
      WRAPLIMITS(_timerInfo.stop.hour, 0, 23);
      break;
    case 3:
      _timerInfo.stop.min += dir;
      WRAPLIMITS(_timerInfo.stop.min, 0, 59);
      break;
    case 4:
      if(_rowSel == 1) {
        _timerInfo.enabled &= 0x80;      // ensure specific day flags are cleared
        _timerInfo.enabled ^= 0x80;      // toggle next day flag
      }
      if(_rowSel == 2) {
        _timerInfo.enabled &= 0x7f;      // ensure next day flag is cleared
        _timerInfo.enabled ^= maskDOW;   // toggle flag for day of week
      }
      break;
    case 5:
      _timerInfo.repeat = !_timerInfo.repeat;
      break;
  }
}

void
CSetTimerScreen::_printEnabledTimers()
{
  const int dayWidth = 8;
  int xPos = _display.width() - border;
  int yPos = 28;

  if(_timerInfo.enabled == 0x00 && _rowSel != 2) {
    _printMenuText(xPos, yPos, "Disabled", _colSel==4, eRightJustify);
  }
  else if(_timerInfo.enabled & 0x80) {
    if(_rowSel==1 && _colSel==4)
      _printMenuText(xPos, yPos, "Enabled", true, eRightJustify);
    else 
      _printInverted(xPos, yPos, "Enabled", true, eRightJustify);
  }
  else {
    if(_rowSel==1 && _colSel==4) {
      CRect extents;
      extents.width = 7 * dayWidth + 2;
      extents.height = 8;
      extents.xPos = xPos - extents.width;
      extents.yPos = yPos;
      extents.Expand(border);
      _display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
    }
    xPos -= 7 * dayWidth;  // back step 7 day entries
    int xSel = xPos + _colSel * dayWidth;  // note location of selection now (xPos gets changed)
    // print days, inverse if enabled
    for(int i=0; i<7; i++) {
      int dayMask = 0x01 << i;
      _printInverted(xPos, yPos, briefDOW[i], _timerInfo.enabled & dayMask);
      xPos += dayWidth;
    }
    // draw selection loop afterwards - writing text otherwise obliterates it
    if(_rowSel==2) {
      CRect extents;
      extents.xPos = xSel;
      extents.yPos = yPos;
      _drawMenuSelection(extents, briefDOW[_colSel]);
    }
  }
}

void
CSetTimerScreen::_showConflict(const char* str)      
{
  CTransientFont AF(_display, &arial_8ptBoldFontInfo);
  _display.fillRect(19, 22, 90, 36, WHITE);
  _printInverted(_display.xCentre(), 39, str, true, eCentreJustify);
  _printInverted(_display.xCentre(), 28, "Conflicts", true, eCentreJustify);
}  
 