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

const int border = 4;
const int radius = 4;
const char* baseLabel = "<-             ->"; 

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
  char str[16];
  int yPos;
  const int col2 = 90;
  const int col3 = _display.width() - border;

  _display.setCursor(0, 16);
  _display.print("Fuel Settings:");

  switch(_rowSel) {
    case 0:
      yPos = 28;
      _display.setCursor(0, yPos);
      _display.print("Pump (Hz)");
      _display.setCursor(col2, yPos);
      sprintf(str, "%.1f", CtlFrame.getPump_Min());
      _display.printRightJustified(str);
      _display.setCursor(col3, yPos);
      sprintf(str, "%.1f", CtlFrame.getPump_Max());
      _display.printRightJustified(str);
      yPos = 40;
      _display.setCursor(0, yPos);
      _display.print("Fan (RPM)");
      _display.setCursor(col2, yPos);
      sprintf(str, "%d", CtlFrame.getFan_Min());
      _display.printRightJustified(str);
      _display.setCursor(col3, yPos);
      sprintf(str, "%d", CtlFrame.getFan_Max());
      _display.printRightJustified(str);
      break;
    case 1:
      _display.setCursor(_display.xCentre(), 30);
      _display.printCentreJustified("Enter password...");
      _showPassword();
      break;
    case 2:
    case 3:
    case 4:
    case 5:
      _display.clearDisplay();
      //50, 38, 26, 14
      yPos = border + 36;
      // Pump Minimum adjustment
      _display.setCursor(0, yPos);
      _display.print("Pump    Min");
      _display.setCursor(col3, yPos);
      sprintf(str, "%.1f", adjPump[0]); 
      _display.printRightJustified(str);
      if(_rowSel == 2) {
        _drawSelectionBox(col3, yPos, str);
      }
      // Pump Maximum adjustment
      yPos = border + 24;
      _display.setCursor(0, yPos);
      _display.print("Pump Hz Max");
      _display.setCursor(col3, yPos);
      sprintf(str, "%.1f", adjPump[1]);
      _display.printRightJustified(str);
      if(_rowSel == 3) {
        _drawSelectionBox(col3, yPos, str);
      }
      // Fan Minimum adjustment
      yPos = border + 12;
      _display.setCursor(0, yPos);
      _display.print("Fan     Min");
      _display.setCursor(col3, yPos);
      sprintf(str, "%d", adjFan[0]);
      _display.printRightJustified(str);
      if(_rowSel == 4) {
        _drawSelectionBox(col3, yPos, str);
      }
      // Fan Maximum adjustment
      yPos = border;
      _display.setCursor(0, yPos);
      _display.print("Fan RPM Max");
      _display.setCursor(col3, yPos);
      sprintf(str, "%d", adjFan[1]);
      _display.printRightJustified(str);
      if(_rowSel == 5) {
        _drawSelectionBox(col3, yPos, str);
      }

      yPos = 53;
      _display.setCursor(_display.xCentre(), yPos);
      _display.printCentreJustified(baseLabel);
      break;
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
          adjPump[0] = getPumpMin();
          adjPump[1] = getPumpMax();
          adjFan[0] = getFanMin();
          adjFan[1] = getFanMax();
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
        case 4:
        case 5:
          _adjustSetting(-1);
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
          UPPERLIMIT(_colSel, 5);
          break;
        case 2:
        case 3:
        case 4:
        case 5:
          _adjustSetting(+1);
          break;
      }
    }
    // press UP 
    if(event & key_Up) {
      switch(_rowSel) {
        case 0:
        case 2:
        case 3:
        case 4:
          _rowSel++;
          _colSel = 0;
          UPPERLIMIT(_rowSel, 5);
          break;
        case 1:
          _PWdig[_colSel]++; 
          ROLLUPPERLIMIT(_PWdig[_colSel], 9, 0);
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
          _rowSel = 0;   
          break;
        case 3:
        case 4:
        case 5:
          _rowSel--;
          _colSel = 0;
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
      extents.Expand(-border);
    }
    _display.setCursor(extents.xPos, extents.yPos);
    _display.print(str);
  }
}

void
CScreen5::_drawSelectionBox(int x, int y, const char* str, int border, int radius)
{
  CRect extents;
  _display.getTextExtents(str, extents);
  extents.xPos = x - extents.width;
  extents.yPos = y;
  extents.Expand(border);
  _display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
}

void
CScreen5::_adjustSetting(int dir)
{
  switch(_rowSel) {
    case 2:
      adjPump[0] += (float(dir) * 0.1f);
      LOWERLIMIT(adjPump[0], 0.5f);
      UPPERLIMIT(adjPump[0], 10.f);
      break;
    case 3:
      adjPump[1] += (float(dir) * 0.1f);
      LOWERLIMIT(adjPump[1], 0.5f);
      UPPERLIMIT(adjPump[1], 10.f);
      break;
    case 4:
      adjFan[0] += dir * 10;
      LOWERLIMIT(adjFan[0], 1000);
      UPPERLIMIT(adjFan[0], 5000);
      break;
    case 5:
      adjFan[1] += dir * 10;
      LOWERLIMIT(adjFan[1], 1000);
      UPPERLIMIT(adjFan[1], 5000);
      break;
  }
}