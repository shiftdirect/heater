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
  int xPos, yPos;
  const int col2 = 90;
  const int col3 = _display.width() - border;

  _display.setCursor(0, 16);
  _display.print("Fuel Settings:");

  switch(_rowSel) {
    case 0:
      // show settings overview (initial screen entry)
      // pump max/min
      yPos = 28;
      _display.setCursor(0, yPos);
      _display.print("Pump (Hz)");
      sprintf(str, "%.1f", getPumpMin());
      _display.setCursor(col2, yPos);
      _display.printRightJustified(str);
      sprintf(str, "%.1f", getPumpMax());
      _display.setCursor(col3, yPos);
      _display.printRightJustified(str);
      // fan max/min
      yPos = 40;
      _display.setCursor(0, yPos);
      _display.print("Fan (RPM)");
      sprintf(str, "%d", getFanMin());
      _display.setCursor(col2, yPos);
      _display.printRightJustified(str);
      sprintf(str, "%d", getFanMax());
      _display.setCursor(col3, yPos);
      _display.printRightJustified(str);
      // navigation line
      yPos = 53;
      xPos = _display.xCentre();
      _drawMenuTextCentreJustified(xPos, yPos, true, baseLabel);
      break;

    case 1:
      _display.setCursor(_display.xCentre(), 34);
      _display.printCentreJustified("Enter password...");
      _showPassword();
      break;

    case 2:
    case 3:
    case 4:
    case 5:
      _display.clearDisplay();
      // Pump Minimum adjustment
      yPos = border + 36;
      _display.setCursor(0, yPos);
      _display.print("Pump    Min");
      sprintf(str, "%.1f", adjPump[0]); 
      _drawMenuTextRightJustified(col3, yPos, _rowSel == 2, str);
      // Pump Maximum adjustment
      yPos = border + 24;
      _display.setCursor(0, yPos);
      _display.print("Pump Hz Max");
      sprintf(str, "%.1f", adjPump[1]);
      _drawMenuTextRightJustified(col3, yPos, _rowSel == 3, str);
      // Fan Minimum adjustment
      yPos = border + 12;
      _display.setCursor(0, yPos);
      _display.print("Fan     Min");
      sprintf(str, "%d", adjFan[0]);
      _drawMenuTextRightJustified(col3, yPos, _rowSel == 4, str);
      // Fan Maximum adjustment
      yPos = border;
      _display.setCursor(0, yPos);
      _display.print("Fan RPM Max");
      sprintf(str, "%d", adjFan[1]);
      _drawMenuTextRightJustified(col3, yPos, _rowSel == 5, str);
      // navigation line
      yPos = 53;
      _display.setCursor(_display.xCentre(), yPos);
      _display.printCentreJustified(baseLabel);
      break;

    case 6:
      _display.setCursor(_display.xCentre(), 35);
      _display.printCentreJustified("Press UP to");
      _display.setCursor(_display.xCentre(), 43);
      _display.printCentreJustified("confirm save");
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
      switch(_rowSel) {
        case 1:
          // match "1688"
          if((_PWdig[0] == 1) && 
             (_PWdig[1] == 6) && 
             (_PWdig[2] == 8) && 
             (_PWdig[3] == 8)) {
            _rowSel = 2;
            _colSel = 0;
            // grab current settings upon entry to edit mode
            adjPump[0] = getPumpMin();
            adjPump[1] = getPumpMax();
            adjFan[0] = getFanMin();
            adjFan[1] = getFanMax();
          }
          // reset PW digits
          for(int i= 0; i < 4; i++) 
            _PWdig[i] = -1;
          break;
        case 2:
        case 3:
        case 4:
        case 5:
          _rowSel = 6;  // enter save confirm mode
          break;
        case 6:
          _rowSel = 0;
          break;
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
        case 6:
          _rowSel = 0;
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
        case 6:
          _rowSel = 0;
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
        case 1:  // password entry
          _PWdig[_colSel]++; 
          ROLLUPPERLIMIT(_PWdig[_colSel], 9, 0);
          break;
        case 6:
          setPumpMin(adjPump[0]);
          setPumpMax(adjPump[1]);
          setFanMin(adjFan[0]);
          setFanMax(adjFan[1]);
          saveNV();
          _rowSel = 0;
          break;
      }
    }
    // press DOWN
    if(event & key_Down) {
      switch(_rowSel) {
        case 1:  // password entry
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
        case 6:
          _rowSel = 0;
          break;
      }
    }
  }

  
  if(event & keyRepeat) {
    switch(_rowSel) {
      case 1:
        if(event & key_Centre)
          _rowSel = 0;    // escape PW entry by holding centre button
        break;
      case 2:
      case 3:
      case 4:
      case 5:
        int adj = 0;
        if(event & key_Right) adj = +1;
        if(event & key_Left) adj = -1;
        if(adj) {
          _adjustSetting(adj);
        }
        break;
    }
  }
  _Manager.reqUpdate();
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

  for(int i =0 ; i < 4; i++) {

    extents.xPos = _display.xCentre() - (2 - i) * (charWidth * 1.5); 
    extents.yPos = 50;

    char str[8];

    if(_PWdig[i] < 0) {
      strcpy(str, "-");
    }
    else {
      sprintf(str, "%d", _PWdig[i]);
    }
    if(_rowSel == 1 && _colSel == i) {
      // draw selection box
      _drawSelectionBox(extents.xPos, extents.yPos, str);
    }
    _display.setCursor(extents.xPos, extents.yPos);
    _display.print(str);
  }
}

void
CScreen5::_adjustSetting(int dir)
{
  switch(_rowSel) {
    case 2:
      adjPump[0] += (float(dir) * 0.1f);
      break;
    case 3:
      adjPump[1] += (float(dir) * 0.1f);
      break;
    case 4:
      adjFan[0] += dir * 10;
      break;
    case 5:
      adjFan[1] += dir * 10;
      break;
  }
  LOWERLIMIT(adjPump[0], 0.5f);
  UPPERLIMIT(adjPump[0], 10.f);
  LOWERLIMIT(adjFan[1], 1000);
  UPPERLIMIT(adjFan[1], 5000);
}