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
// CScreen5
//
// This screen allows the fuel mixture endpoints to be adjusted
//
///////////////////////////////////////////////////////////////////////////

#include "128x64OLED.h"
#include "KeyPad.h"
#include "helpers.h"
#include "Screen7.h"
#include "NVStorage.h"
#include "RTClib.h"

const char* briefDOW[] = { "S", "M", "T", "W", "T", "F", "S" };

CScreen7::CScreen7(C128x64_OLED& display, CScreenManager& mgr, int instance) : CScreenHeader(display, mgr) 
{
  _rowSel = 0;
  _colSel = 0;
  _instance = instance;
  NVstore.getTimerInfo(_instance, _timer);
}


void 
CScreen7::show()
{
  CScreenHeader::show();

  char str[16];
  int xPos, yPos;

  sprintf(str, " Timer %d ", _instance + 1);
  _printInverted(0, 16, str, true);

  // start
  xPos = 18;
  yPos = 28;
  _printMenuText(xPos, yPos, "On", false, eRightJustify);
  _printMenuText(xPos+18, yPos, ":");
  xPos += 6;
  sprintf(str, "%02d", _timer.start.hour);
  _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==0);
  xPos += 17;
  sprintf(str, "%02d", _timer.start.min);
  _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==1);

  // stop
  xPos = 18;
  yPos = 40;
  _printMenuText(xPos, yPos, "Off", false, eRightJustify);
  _printMenuText(xPos+18, yPos, ":");
  xPos += 6;
  sprintf(str, "%02d", _timer.stop.hour);
  _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==2);
  xPos += 17;
  sprintf(str, "%02d", _timer.stop.min);
  _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==3);
  
  // control
  const char* msg;
  xPos = _display.width() - border;
  _printEnabledTimers();
  
  yPos = 40;
  if(_timer.repeat)
    msg = "Repeat";
  else
    msg = "Once";
  if(_rowSel == 1) 
    _printMenuText(xPos, yPos, msg, _colSel==5, eRightJustify);
  else
    _printInverted(xPos, yPos, msg, _timer.repeat, eRightJustify);

  // navigation line
  yPos = 53;
  xPos = _display.xCentre();
  _printMenuText(xPos, yPos, "<-             ->", _rowSel==0, eCentreJustify);
}


void 
CScreen7::keyHandler(uint8_t event)
{
  static bool bHeld = false;
  // handle initial key press
  if(event & keyPressed) {
    bHeld = false;
    // press CENTRE
    if(event & key_Centre) {
      if(_rowSel != 0) {
        NVstore.setTimerInfo(_instance, _timer);
        NVstore.save();
        _rowSel = 0;
      }
      return;
    }
    // press LEFT - navigate fields, or screens
    if(event & key_Left) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.prevScreen(); 
          break;
        case 1:
          _colSel--;
          ROLLLOWERLIMIT(_colSel, 0, 5);
          break;
        case 2:
          _colSel--;
          ROLLLOWERLIMIT(_colSel, 0, 6);
          break;
      }
    }
    // press RIGHT - navigate fields, or screens
    if(event & key_Right) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.nextScreen(); 
          break;
        case 1:
          _colSel++;
          ROLLUPPERLIMIT(_colSel, 5, 0);
          break;
        case 2:
          _colSel++;
          ROLLUPPERLIMIT(_colSel, 6, 0);
          break;
      }
    }
  }

  // handle held down keys
  if(event & keyRepeat) {
    bHeld = true;
    if(_rowSel == 1) {
      if(_colSel < 4) {
        if(event & key_Down) adjust(-1);
        if(event & key_Up) adjust(+1);
      }
      else if(_colSel == 4) {
        if(event & key_Up) {
          _timer.enabled &= 0x7f;   // strip next day flag
          _rowSel = 2;
          _colSel = 0;
        }
      }
    }
    if(_rowSel==2) {
      if(event & key_Down) {
        _rowSel = 1;
        _colSel = 4;
      }
    }
  }

  if(event & keyReleased) {
    if(!bHeld) {
      // released DOWN - can only leave adjustment by using OK (centre button)
      int maskDOW = 0x01 << _colSel;
      if(event & key_Down) {
        // adjust selected item
        if(_rowSel == 1) 
          adjust(-1);
        if(_rowSel == 2) {
          // adjust selected item
          _timer.enabled ^= maskDOW;
          _timer.enabled &= 0x7f;
        }
      }
      // released UP 
      if(event & key_Up) {
        switch(_rowSel) {
          case 0:
            // move from screen navigation to field select & adjust
            _rowSel = 1;
            _colSel = 0;
            break;
          case 1:
            // adjust selected item
            adjust(+1);
            break;
          case 2:
            // adjust selected item
            _timer.enabled ^= maskDOW;
            _timer.enabled &= 0x7f;
            break;
        }
      }
    }
  }

  _ScreenManager.reqUpdate();
}


void 
CScreen7::adjust(int dir)
{
  int days;
  int maskDOW = 0x01 << _colSel;  // if doing Day of Week - (_rowSel == 2)  

  switch(_colSel) {
    case 0:
      _timer.start.hour += dir;
      ROLLUPPERLIMIT(_timer.start.hour, 23, 0);
      ROLLLOWERLIMIT(_timer.start.hour, 0, 23);
      break;
    case 1:
      _timer.start.min += dir;
      ROLLUPPERLIMIT(_timer.start.min, 59, 0);
      ROLLLOWERLIMIT(_timer.start.min, 0, 59);
      break;
    case 2:
      _timer.stop.hour += dir;
      ROLLUPPERLIMIT(_timer.stop.hour, 23, 0);
      ROLLLOWERLIMIT(_timer.stop.hour, 0, 23);
      break;
    case 3:
      _timer.stop.min += dir;
      ROLLUPPERLIMIT(_timer.stop.min, 59, 0);
      ROLLLOWERLIMIT(_timer.stop.min, 0, 59);
      break;
    case 4:
      if(_rowSel == 1) {
        _timer.enabled &= 0x80;      // ensure specific day flags are cleared
        _timer.enabled ^= 0x80;      // toggle next day flag
      }
      if(_rowSel == 2) {
        _timer.enabled &= 0x7f;      // ensure next day flag is cleared
        _timer.enabled ^= maskDOW;   // toggle flag for day of week
      }
      break;
    case 5:
      _timer.repeat = !_timer.repeat;
      break;
  }
}

void
CScreen7::_printEnabledTimers()
{
  const int dayWidth = 8;
  int xPos = _display.width() - border;
  int yPos = 28;

  if(_timer.enabled == 0x00 && _rowSel != 2) {
    _printMenuText(xPos, yPos, "Disabled", _colSel==4, eRightJustify);
  }
  else if(_timer.enabled & 0x80) {
    if(_rowSel==1 && _colSel==4)
      _printMenuText(xPos, yPos, "Enabled", true, eRightJustify);
    else 
      _printInverted(xPos, yPos, "Enabled", true, eRightJustify);
  }
  else {
    if(_rowSel==1 && _colSel==4) {
      _printMenuText(xPos, yPos, "Hold UP", true, eRightJustify);
    }
    else {
      xPos -= 7 * dayWidth;  // back step 7 day entries
      int xSel = xPos + _colSel * dayWidth;
      for(int i=0; i<7; i++) {
        int dayMask = 0x01 << i;
        _printInverted(xPos, yPos, briefDOW[i], _timer.enabled & dayMask);
        xPos += dayWidth;
      }
      if(_rowSel == 2) {
        CRect extents;
        extents.xPos = xSel;
        extents.yPos = yPos;
        _drawMenuSelection(extents, briefDOW[_colSel]);
      }
    }
  }
}
      