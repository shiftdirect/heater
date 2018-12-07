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

CScreen7::CScreen7(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _rowSel = 0;
  _colSel = 0;
}


void 
CScreen7::show()
{
  CScreenHeader::show();

  char str[16];
  int xPos, yPos;

  _printInverted(0, 16, " Timer ", true);

  switch(_rowSel) {
    case 0:
    case 1:
      yPos = 28;
      xPos = 32;

      // start
      _printMenuText(xPos, yPos, "Start", false, eRightJustify);
      xPos += 6;
      sprintf(str, "%02d", _startHour);
      _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==0);
      xPos += 16;
      _printMenuText(xPos, yPos, ":");
      xPos += 8;
      sprintf(str, "%02d", _startMin);
      _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==1);

      // stop
      xPos = 32;
      yPos = 40;
      _printMenuText(xPos, yPos, "Stop", false, eRightJustify);
      xPos += 6;
      sprintf(str, "%02d", _stopHour);
      _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==2);
      xPos += 16;
      _printMenuText(xPos, yPos, ":");
      xPos += 8;
      sprintf(str, "%02d", _stopMin);
      _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==3);
      
      // control
      xPos = _display.width() - 8 * 6;
      yPos = 32;
      const char* msg;
      if(_bEnabled)
        msg = "Enabled";
      else 
        msg = "Disabled";
      if(_rowSel == 1) 
        _printMenuText(xPos, yPos, msg, _colSel==4, eRightJustify);
      else 
        _printInverted(xPos, yPos, msg, _bEnabled, eRightJustify);
      
      yPos = 40;
      if(_bRepeat)
        msg = "Repeat";
      else
        msg = "One time";
      if(_rowSel == 1) 
        _printMenuText(xPos, yPos, msg, _colSel==5, eRightJustify);
      else
        _printInverted(xPos, yPos, msg, _bRepeat, eRightJustify);

      // navigation line
      yPos = 53;
      xPos = _display.xCentre();
      _printMenuText(xPos, yPos, "<-             ->", _rowSel==0, eCentreJustify);
      break;

  }
}


void 
CScreen7::keyHandler(uint8_t event)
{
  // handle initial key press
  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      if(_rowSel == 1) {
        // TODO: save settings
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
      }
    }
    // press UP 
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
      }
    }
    // press DOWN - can only leave adjustment by using OK (centre button)
    if(event & key_Down) {
      // adjust selected item
      if(_rowSel == 1) adjust(-1);
    }
  }

  // handle held down keys
  if(event & keyRepeat) {
    if(_rowSel == 1) {
      if(event & key_Down) adjust(-1);
      if(event & key_Up) adjust(-1);
    }
  }

  _ScreenManager.reqUpdate();
}


void 
CScreen7::adjust(int dir)
{
  int days;
  switch(_colSel) {
    case 0:
      _startHour += dir;
      ROLLUPPERLIMIT(_startHour, 23, 0);
      ROLLLOWERLIMIT(_startHour, 0, 23);
      break;
    case 1:
      _startMin += dir;
      ROLLUPPERLIMIT(_startMin, 59, 0);
      ROLLLOWERLIMIT(_startMin, 0, 59);
      break;
    case 2:
      _stopHour += dir;
      ROLLUPPERLIMIT(_stopHour, 23, 0);
      ROLLLOWERLIMIT(_stopHour, 0, 23);
      break;
    case 3:
      _stopMin += dir;
      ROLLUPPERLIMIT(_stopMin, 59, 0);
      ROLLLOWERLIMIT(_stopMin, 0, 59);
      break;
    case 4:
      _bEnabled = !_bEnabled;
      break;
    case 5:
      _bRepeat = !_bRepeat;
      break;
  }
}
