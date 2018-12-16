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

#include "Screen5.h"
#include "KeyPad.h"
#include "../Protocol/helpers.h"
#include "../Wifi/BTCWifi.h"
#include "fonts/Arial.h"


CScreen5::CScreen5(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _rowSel = 0;
  _colSel = 0;
  for(int i= 0; i < 4; i++) 
    _PWdig[i] = -1;
}


void 
CScreen5::show()
{
  CScreenHeader::show();

  char str[16];
  int xPos, yPos;
  const int col2 = 90;
  const int col3 = _display.width() - border;

  _printInverted(0, 16, " Fuel Settings ", true);

  switch(_rowSel) {
    case 0:
      // show settings overview (initial screen entry)
      // pump max/min
      yPos = 28;
      _printMenuText(0, yPos, "Pump (Hz)");
      sprintf(str, "%.1f", getHeaterInfo().getPump_Min());
      _printMenuText(col2, yPos, str, false, eRightJustify);
      sprintf(str, "%.1f", getHeaterInfo().getPump_Max());
      _printMenuText(col3, yPos, str, false, eRightJustify);
      // fan max/min
      yPos = 40;
      _printMenuText(0, yPos, "Fan (RPM)");
      sprintf(str, "%d", getHeaterInfo().getFan_Min());
      _printMenuText(col2, yPos, str, false, eRightJustify);
      sprintf(str, "%d", getHeaterInfo().getFan_Max());
      _printMenuText(col3, yPos, str, false, eRightJustify);
      // navigation line
      yPos = 53;
      xPos = _display.xCentre();
      _printMenuText(xPos, yPos, "<-             ->", true, eCentreJustify);
      break;

    case 1:
      _printMenuText(_display.xCentre(), 34, "Enter password...", false, eCentreJustify);
      _showPassword();
      break;

    case 2:
    case 3:
    case 4:
    case 5:
      _display.clearDisplay();
      // Pump Minimum adjustment
      yPos = border + 36;
      _printMenuText(80, yPos, "Min", false, eRightJustify);
      sprintf(str, "%.1f", adjPump[0]); 
      _printMenuText(col3, yPos, str, _rowSel == 2, eRightJustify);
      // Pump Maximum adjustment
      yPos = border + 24;
      _printMenuText(80, yPos, "Pump Hz Max", false, eRightJustify);
      sprintf(str, "%.1f", adjPump[1]);
      _printMenuText(col3, yPos, str, _rowSel == 3, eRightJustify);
      // Fan Minimum adjustment
      yPos = border + 12;
      _printMenuText(80, yPos, "Min", false, eRightJustify);
      sprintf(str, "%d", adjFan[0]);
      _printMenuText(col3, yPos, str, _rowSel == 4, eRightJustify);
      // Fan Maximum adjustment
      yPos = border;
      _printMenuText(80, yPos, "Fan RPM Max", false, eRightJustify);
      sprintf(str, "%d", adjFan[1]);
      _printMenuText(col3, yPos, str, _rowSel == 5, eRightJustify);
      // navigation line
      yPos = 53;
      _printMenuText(_display.xCentre(), yPos, "<-             ->", false, eCentreJustify);
      break;

    case 6:
      _printMenuText(_display.xCentre(), 35, "Press UP to", false, eCentreJustify);
      _printMenuText(_display.xCentre(), 43, "confirm save", false, eCentreJustify);
      break;
  }
  
//  _display.display();
}


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
            adjPump[0] = getHeaterInfo().getPump_Min();
            adjPump[1] = getHeaterInfo().getPump_Max();
            adjFan[0] = getHeaterInfo().getFan_Min();
            adjFan[1] = getHeaterInfo().getFan_Max();
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
          _ScreenManager.prevScreen(); 
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
          _ScreenManager.nextScreen(); 
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
  _ScreenManager.reqUpdate();
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

  for(int idx =0 ; idx < 4; idx++) {

    extents.xPos = _display.xCentre() - (2 - idx) * (charWidth * 1.5); 
    extents.yPos = 50;

    char str[8];

    if(_PWdig[idx] < 0) {
      strcpy(str, "-");
    }
    else {
      sprintf(str, "%d", _PWdig[idx]);
    }
    _printMenuText(extents.xPos, extents.yPos, str, _colSel == idx);
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