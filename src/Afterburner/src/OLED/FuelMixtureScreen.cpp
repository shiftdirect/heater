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
// CFuelMixtureScreen
//
// This screen allows the fuel mixture endpoints to be adjusted
//
///////////////////////////////////////////////////////////////////////////

#include "FuelMixtureScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "../WiFi/BTCWifi.h"
#include "../Utility/DebugPort.h"
#include "../Utility/macros.h"
#include "../Protocol/Protocol.h"


CFuelMixtureScreen::CFuelMixtureScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _initUI();
}

void
CFuelMixtureScreen::onSelect()
{
  CPasswordScreen::onSelect();
  _initUI();
  
  adjPump[0] = getHeaterInfo().getPump_Min();
  adjPump[1] = getHeaterInfo().getPump_Max();
  adjFan[0] = getHeaterInfo().getFan_Min();
  adjFan[1] = getHeaterInfo().getFan_Max();
}

void
CFuelMixtureScreen::_initUI()
{
  _rowSel = 0;
  _colSel = 0;
}

bool 
CFuelMixtureScreen::show()
{
  char str[16];
  int xPos, yPos;
  const int col3 = _display.width() - border;

  _display.clearDisplay();

  if(!CPasswordScreen::show()) {
    switch(_rowSel) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
        // Pump Minimum adjustment
        yPos = border + 36;
        _printMenuText(80, yPos, "Min", false, eRightJustify);
        sprintf(str, "%.1f", adjPump[0]); 
        _printMenuText(col3, yPos, str, _rowSel == 1, eRightJustify);
        // Pump Maximum adjustment
        yPos = border + 24;
        _printMenuText(80, yPos, "Pump Hz Max", false, eRightJustify);
        sprintf(str, "%.1f", adjPump[1]);
        _printMenuText(col3, yPos, str, _rowSel == 2, eRightJustify);
        // Fan Minimum adjustment
        yPos = border + 12;
        _printMenuText(80, yPos, "Min", false, eRightJustify);
        sprintf(str, "%d", adjFan[0]);
        _printMenuText(col3, yPos, str, _rowSel == 3, eRightJustify);
        // Fan Maximum adjustment
        yPos = border;
        _printMenuText(80, yPos, "Fan RPM Max", false, eRightJustify);
        sprintf(str, "%d", adjFan[1]);
        _printMenuText(col3, yPos, str, _rowSel == 4, eRightJustify);
        // navigation line
        yPos = 53;
        xPos = _display.xCentre();
        switch(_rowSel) {
          case 0:
            _printMenuText(xPos, yPos, " \021     Exit     \020 ", _rowSel == 0, eCentreJustify);     // " <     Exit     > "
            break;
          case 1:
          case 2:
            _display.drawFastHLine(0, 52, 128, WHITE);
            _printMenuText(xPos, 56, "\030\031Sel   Save  \033\032 \3600.1", false, eCentreJustify);  // "^vSel   Save  <> +-0.1"
            break;
          case 3:
          case 4:
            _display.drawFastHLine(0, 52, 128, WHITE);
            _printMenuText(xPos, 56, "\030\031Sel   Save   \033\032 \36010", false, eCentreJustify);  // "^vSel   Save   <> +-10"
            break;
        }

        break;

      case 5:
        _printInverted(_display.xCentre(), 0, " Save Fuel Settings ", true, eCentreJustify);
        _printMenuText(_display.xCentre(), 35, "Press UP to", false, eCentreJustify);
        _printMenuText(_display.xCentre(), 43, "confirm save", false, eCentreJustify);
        break;
    }
  }
  
  return true;
}


bool 
CFuelMixtureScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);
          break;
        case 1:
        case 2:
        case 3:
        case 4:
          _rowSel = 5;  // enter save confirm mode
          break;
        case 5:
          _rowSel = 0;
          break;
      }
    }
    // press LEFT 
    if(event & key_Left) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.prevMenu(); 
          break;
        case 1:
        case 2:
        case 3:
        case 4:
          _adjustSetting(-1);
          break;
        case 5:
          _rowSel = 0;
          break;
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.nextMenu(); 
          break;
        case 1:
        case 2:
        case 3:
        case 4:
          _adjustSetting(+1);
          break;
        case 5:
          _rowSel = 0;
          break;
      }
    }
    // press UP 
    if(event & key_Up) {
      if(hasOEMcontroller())
        _reqOEMWarning();
      else {
        switch(_rowSel) {
          case 0:
            // grab current settings upon entry to edit mode
            adjPump[0] = getHeaterInfo().getPump_Min();
            adjPump[1] = getHeaterInfo().getPump_Max();
            adjFan[0] = getHeaterInfo().getFan_Min();
            adjFan[1] = getHeaterInfo().getFan_Max();
          case 1:
          case 2:
          case 3:
            _rowSel++;
            _colSel = 0;
            UPPERLIMIT(_rowSel, 4);
            break;
          case 5:
            _showStoringMessage();
            setPumpMin(adjPump[0]);
            setPumpMax(adjPump[1]);
            setFanMin(adjFan[0]);
            setFanMax(adjFan[1]);
            saveNV();
            _rowSel = 0;
            break;
        }
      }
    }
    // press DOWN
    if(event & key_Down) {
      switch(_rowSel) {
        case 1:
        case 2:
        case 3:
        case 4:
          _rowSel--;
          _colSel = 0;
          break;
        case 5:
          _rowSel = 0;
          break;
      }
    }
    _ScreenManager.reqUpdate();
  }

  
  if(event & keyRepeat) {
    switch(_rowSel) {
      case 1:
      case 2:
      case 3:
      case 4:
        int adj = 0;
        if(event & key_Right) adj = +1;
        if(event & key_Left) adj = -1;
        if(adj) {
          _adjustSetting(adj);
        }
        break;
    }
    _ScreenManager.reqUpdate();
  }
  return true;
}

void
CFuelMixtureScreen::_adjustSetting(int dir)
{
  switch(_rowSel) {
    case 1:
      adjPump[0] += (float(dir) * 0.1f);
      break;
    case 2:
      adjPump[1] += (float(dir) * 0.1f);
      break;
    case 3:
      adjFan[0] += dir * 10;
      break;
    case 4:
      adjFan[1] += dir * 10;
      break;
  }
  LOWERLIMIT(adjPump[0], 0.5f);
  UPPERLIMIT(adjPump[0], 10.f);
  LOWERLIMIT(adjPump[1], 0.5f);
  UPPERLIMIT(adjPump[1], 10.f);
  LOWERLIMIT(adjFan[0], 1000);
  UPPERLIMIT(adjFan[0], 5000);
  LOWERLIMIT(adjFan[1], 1000);
  UPPERLIMIT(adjFan[1], 5000);
}