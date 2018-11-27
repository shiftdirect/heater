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
#include "display.h"
#include "KeyPad.h"
#include "helpers.h"
#include "Screen5.h"
#include "BTCWifi.h"


CScreen5::CScreen5(C128x64_OLED& display, CScreenManager& mgr) : CScreen(display, mgr) 
{
  _bPWOK = false;
  _rowSel = 0;
  _colSel = 0;
  for(int i= 0; i < 4; i++) 
    _PWdig[i] = -1;
}


void 
CScreen5::show(const CProtocol& CtlFrame, const CProtocol& HtrFrame)
{
  CScreen::show(CtlFrame, HtrFrame);
  
  CRect extents;

  _display.setCursor(0, 16);
  _display.print("Fuel Settings:");

  if(_rowSel == 1) {
    _display.setCursor(0, 30);
    _display.print("Enter password...");
    _showPassword();
  }
  else {
    char str[16];
    int yPos = 28;
    const int border = 4;
    const int radius = 4;
    const int col2 = 90;
    const int col3 = _display.width() - border;
    CRect extents;
    _display.setCursor(0, yPos);
    _display.print("Pump (Hz)");
    _display.setCursor(col2, yPos);
    sprintf(str, "%.1f", CtlFrame.getPump_Min());
    _display.printRightJustified(str);
    if(_rowSel == 3 && _colSel == 0) {
      _display.getTextExtents(str, extents);
      extents.xPos = col2 - extents.width;
      extents.yPos = yPos;
      extents.Expand(border);
      _display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
    }
    _display.setCursor(col3, yPos);
    sprintf(str, "%.1f", CtlFrame.getPump_Max());
    _display.printRightJustified(str);
    if(_rowSel == 3 && _colSel == 1) {
      _display.getTextExtents(str, extents);
      extents.xPos = col3 - extents.width;
      extents.yPos = yPos;
      extents.Expand(border);
      _display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
    }
    yPos = 40;
    _display.setCursor(0, yPos);
    _display.print("Fan (RPM)");
    _display.setCursor(col2, yPos);
    sprintf(str, "%d", CtlFrame.getFan_Min());
    _display.printRightJustified(str);
    if(_rowSel == 2 && _colSel == 0) {
      _display.getTextExtents(str, extents);
      extents.xPos = col2 - extents.width;
      extents.yPos = yPos;
      extents.Expand(border);
      _display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
    }
    _display.setCursor(col3, yPos);
    sprintf(str, "%d", CtlFrame.getFan_Max());
    _display.printRightJustified(str);
    if(_rowSel == 2 && _colSel == 1) {
      _display.getTextExtents(str, extents);
      extents.xPos = col3 - extents.width;
      extents.yPos = yPos;
      extents.Expand(border);
      _display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
    }
  }
  
  _display.display();
}


void 
CScreen5::animate()
{
  // do nothing!!
};


void 
CScreen5::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      if(_rowSel == 1) {
        // match "1688"
        if((_PWdig[0] == 1) && 
           (_PWdig[1] == 6) && 
           (_PWdig[2] == 8) && 
           (_PWdig[3] == 8)) {
          _bPWOK = true;
          _rowSel = 2;
          _colSel = 0;
        }
        else {
          for(int i= 0; i < 4; i++) 
            _PWdig[i] = -1;
        }
      }
      else {
        _bPWOK = false;
        _rowSel = 0;
      }
      return;
    }
    // press LEFT 
    if(event & key_Left) {
      switch(_rowSel) {
        case 0:
          _Manager.prevScreen(); 
          break;
        case 1:
          _colSel--;
          LOWERLIMIT(_colSel, 0);
          break;
        case 2:
        case 3:
          _colSel = 0;
          break;
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      switch(_rowSel) {
        case 0:
          _Manager.nextScreen(); 
          break;
        case 1:
          _colSel++;
          UPPERLIMIT(_colSel, 3);
          break;
        case 2:
        case 3:
          _colSel = 1;
          break;
      }
    }
    // press UP 
    if(event & key_Up) {
      switch(_rowSel) {
        case 0:
          _rowSel = 1;
          _colSel = 0;
          break;
        case 1:
          _PWdig[_colSel]++; 
          ROLLUPPERLIMIT(_PWdig[_colSel], 9, 0);
          break;
        case 2:
          _rowSel = 3;
          break;
      }
    }
    // press DOWN
    if(event & key_Down) {
      switch(_rowSel) {
        case 1:
          _PWdig[_colSel]--; 
          ROLLLOWERLIMIT(_PWdig[_colSel], 0, 9);
          break;
        case 2:
          _rowSel--;   // force skip over line 1 (password)
          // deliberately not breaking
        case 3:
          _rowSel--;
          break;
      }
    }
    _Manager.reqUpdate();
  }

  // escape PW entry by holding centre button
  if(event & keyRepeat) {
    if(event & key_Centre) {
      _rowSel == 0;
    }
  }
}

void
CScreen5::_showPassword()
{
  // determine metrics of character sizing
  CRect extents;
  _display.getTextExtents("X", extents);
  int charWidth = extents.width;
  _display.getTextExtents(" ", extents);
  int spaceWidth = extents.width;

  const int border = 3;
  const int radius = 4;

  for(int i =0 ; i < 4; i++) {

    int xPos = _display.xCentre() - (2 - i) * (charWidth + spaceWidth);
//    Serial.print(" xPos="); Serial.print(xPos);
    char str[8];

    if(_PWdig[i] < 0) {
      strcpy(str, "-");
    }
    else {
      sprintf(str, "%d", _PWdig[i]);
    }
    _display.getTextExtents(str, extents);
    extents.xPos = xPos;
    extents.yPos = 46;
    if(_rowSel == 1 && _colSel == i) {
      // draw selection box
      extents.Expand(border);
      _display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
      // // draw white background
      // extents.Expand(1);
      // _display.fillRect(extents.xPos, extents.yPos, extents.width, extents.height, WHITE);
      extents.Expand(-border);
    }
    _display.setCursor(extents.xPos, extents.yPos);
    _display.print(str);
  }
//  Serial.println("");
}
