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
#include "../Utility/DebugPort.h"
#include "../Utility/macros.h"
#include "../Utility/NVStorage.h"
#include "../Protocol/Protocol.h"
#include "../Protocol/HeaterManager.h"
#include "fonts/Icons.h"


CFuelMixtureScreen::CFuelMixtureScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _initUI();
}

bool
CFuelMixtureScreen::onSelect()
{
  if(HeaterManager.getHeaterStyle() != 0)
    return false;

  CPasswordScreen::onSelect();
  
  _load();
  return true;
}

void
CFuelMixtureScreen::_load()
{
  adjPump[0] = NVstore.getHeaterTuning().getPmin();
  adjPump[1] = NVstore.getHeaterTuning().getPmax();
  adjFan[0] = NVstore.getHeaterTuning().Fmin;
  adjFan[1] = NVstore.getHeaterTuning().Fmax;
}

bool 
CFuelMixtureScreen::show()
{
  char str[16];
  int xPos, yPos;
  const int col3 = _display.width() - border;

  _display.fillRect(70, 0, 58, 64, BLACK);    // scrub variables
  _display.fillRect(0, 50, 128, 14, BLACK);   // scrub footer

  if(CPasswordScreen::show()) {
    return true;
  }

  if(_animateCount == -1) {
    _animateCount = 0;
    _display.clearDisplay();
  }
  // Pump Minimum adjustment
  yPos = border + 36;
  _printMenuText(65, yPos, "Min", false, eRightJustify);
  sprintf(str, "%.1f Hz", adjPump[0]); 
  _printMenuText(col3, yPos, str, _rowSel == 1, eRightJustify);
  // Pump Maximum adjustment
  yPos = border + 24;
  _printMenuText(65, yPos, "Max", false, eRightJustify);
  sprintf(str, "%.1f Hz", adjPump[1]);
  _printMenuText(col3, yPos, str, _rowSel == 2, eRightJustify);
  // Fan Minimum adjustment
  yPos = border + 12;
  _printMenuText(65, yPos, "Min", false, eRightJustify);
  sprintf(str, "%d RPM", adjFan[0]);
  _printMenuText(col3, yPos, str, _rowSel == 3, eRightJustify);
  // Fan Maximum adjustment
  yPos = border;
  _printMenuText(65, yPos, "Max", false, eRightJustify);
  sprintf(str, "%d RPM", adjFan[1]);
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

  
  return true;
}

bool 
CFuelMixtureScreen::animate()
{
  if(_saveBusy()) 
    return false;

  if(_animateCount >= 0) {

    int xPos = 20;
    int yPos = 5;
    int yFuel = 30;
    _display.fillRect(xPos, yPos, FanIcon1Info.width, FanIcon1Info.height, BLACK);
    _display.fillRect(xPos+5, yFuel, FuelIconInfo.width, FuelIconInfo.height + 4, BLACK);
    _drawBitmap(xPos+5, yFuel+_animateCount, FuelIconInfo);
    switch(_animateCount) {
      case 0: _drawBitmap(xPos, yPos, FanIcon1Info); break;
      case 1: _drawBitmap(xPos, yPos, FanIcon2Info); break;
      case 2: _drawBitmap(xPos, yPos, FanIcon3Info); break;
      case 3: _drawBitmap(xPos, yPos, FanIcon4Info); break;
    }

    _animateCount++;
    WRAPUPPERLIMIT(_animateCount, 3, 0);
  }
  return true; 
}

bool 
CFuelMixtureScreen::keyHandler(uint8_t event)
{
  if(CUIEditScreen::keyHandler(event)) {  // handle save confirm
    return true;
  }

  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      if(_rowSel) {
        _animateCount = -1;
        _display.clearDisplay();
        _confirmSave();   // enter save confirm mode
        _rowSel = 0;
      }
      else {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);
      }
    }
    // press LEFT 
    if(event & key_Left) {
      if(_rowSel) {
        _adjustSetting(-1);
      } 
      else {
        _ScreenManager.prevMenu(); 
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      if(_rowSel) {
        _adjustSetting(+1);
      }
      else {
        _ScreenManager.nextMenu(); 
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
            _load();
            // adjPump[0] = getHeaterInfo().getPump_Min();
            // adjPump[1] = getHeaterInfo().getPump_Max();
            // adjFan[0] = getHeaterInfo().getFan_Min();
            // adjFan[1] = getHeaterInfo().getFan_Max();
          case 1:
          case 2:
          case 3:
            _rowSel++;
            UPPERLIMIT(_rowSel, 4);
            break;
        }
      }
    }
    // press DOWN
    if(event & key_Down) {
      if(_rowSel) {
        _rowSel--;
        if(_rowSel == 0) {
          _load();  // dispose of any changes, re-obtain current settings 
        }
      }
    }
    _ScreenManager.reqUpdate();
  }

  if(event & keyRepeat) {
    if(_rowSel) {
      int adj = 0;
      if(event & key_Right) adj = +1;
      if(event & key_Left) adj = -1;
      if(adj) {
        _adjustSetting(adj);
      }
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
  BOUNDSLIMIT(adjPump[0], 0.5f, 10.f);
  BOUNDSLIMIT(adjPump[1], 0.5f, 10.f);
  BOUNDSLIMIT(adjFan[0], 500, 5000);
  BOUNDSLIMIT(adjFan[1], 1000, 5000);
}

void
CFuelMixtureScreen::_saveNV()
{
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  tuning.setPmin(adjPump[0]);
  tuning.setPmax(adjPump[1]);
  tuning.setFmin(adjFan[0]);
  tuning.setFmax(adjFan[1]);
  NVstore.setHeaterTuning(tuning);
  NVstore.save();
}
