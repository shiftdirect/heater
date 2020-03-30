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
#include "LVCScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "../Utility/macros.h"
#include "../Utility/NVStorage.h"
#include "../Protocol/Protocol.h"
#include "fonts/Icons.h"

static const int Line3 = 14;
static const int Line2 = 20;
static const int Line1 = 36;


CLVCScreen::CLVCScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _initUI();
  _LVC = 115;
}

void 
CLVCScreen::onSelect()
{
  CPasswordScreen::onSelect();
  _initUI();
  _LVC = NVstore.getHeaterTuning().lowVolts;
}

bool 
CLVCScreen::show()
{
  char msg[20];
  const int col = 90;

  _display.fillRect(0, 50, 128, 14, BLACK);
  _display.fillRect(col-border, Line3-border, 128-(col-1), 64-Line3-border, BLACK);

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

    if(_animateCount < 0) {
      _display.clearDisplay();
      _animateCount = 0;
    }

    _showTitle("Low Volt Cutout");
    // low voltage cutout
    int yPos = Line2;
    _printMenuText(col, yPos, "Shutdown < ", false, eRightJustify);
    if(_LVC) 
      sprintf(msg, "%.1fV", float(_LVC) * 0.1);
    else
      strcpy(msg, "OFF");
    _printMenuText(col, yPos, msg, _rowSel == 1);
    // navigation line
    yPos = 53;
    int xPos = _display.xCentre();

    switch(_rowSel) {
      case 0:
        _printMenuText(xPos, yPos, " \021     Exit     \020 ", true, eCentreJustify);
        break;
      default:
        _display.drawFastHLine(0, 52, 128, WHITE);
        _printMenuText(xPos, 56, "\030\031Sel          \033\032 Adj", false, eCentreJustify);
        _printMenuText(xPos, 56, "Save", false, eCentreJustify);
        break;
    }
  }

  return true;
}


bool 
CLVCScreen::animate()
{ 
  if(_saveBusy()) {
    return false;
  }

  if(_animateCount >= 0) {
    switch(_animateCount) {
      case 0:
        _display.fillRect(0, Line3-3, BatteryIconInfo.width, 35, BLACK);
        _drawBitmap(0, Line2-1 , BatteryIconInfo);
        break;
      case 2:
        _display.fillRect(BatteryIconInfo.width - 4, Line2+2, 2, 5, BLACK);   // deplete battery
        break;
      case 4:
        _display.fillRect(BatteryIconInfo.width - 7, Line2+2, 2, 5, BLACK);   // deplete battery
        break;
      case 6:
        _display.fillRect(BatteryIconInfo.width - 10, Line2+2, 2, 5, BLACK);   // deplete battery
        break;
      case 8:
        _display.fillRect(BatteryIconInfo.width - 13, Line2+2, 2, 5, BLACK);   // deplete battery
        break;
    }

    _animateCount++;
    WRAPUPPERLIMIT(_animateCount, 9, 0);
  }

  return true;
}


bool 
CLVCScreen::keyHandler(uint8_t event)
{
  if(CUIEditScreen::keyHandler(event)) {  // handles save confirm
    return true;
  }

  if(event & keyRepeat) {
    if(event & key_Left) {
      _adjust(-1);
    } 
    if(event & key_Right) {
      _adjust(+1);
    } 
  }

  if(event & keyPressed) {
    // press LEFT to select previous screen
    if(event & key_Left) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.prevMenu();
          break;
        case 1:
          _adjust(-1);
          break;
      }
    }
    // press RIGHT to select next screen
    if(event & key_Right) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.nextMenu();
          break;
        case 1:
          _adjust(+1);
          break;
      }
    }
    if(event & key_Down) {
      _rowSel--;
      LOWERLIMIT(_rowSel, 0);
    }
    // UP press
    if(event & key_Up) {
      switch(_rowSel) {
        case 0:
          _rowSel++;
          UPPERLIMIT(_rowSel, 1);
          break;
      }
    }
    // CENTRE press
    if(event & key_Centre) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);
          break;
        case 1:
          _animateCount = -1;
          _display.clearDisplay();          
          _confirmSave();
          _rowSel = 0;
          break;
      }
    }
    _ScreenManager.reqUpdate();
  }

  return true;
}

void 
CLVCScreen::_adjust(int dir)
{
  switch(_rowSel) {
    case 1:
      if(_LVC == 0) {
        if(NVstore.getHeaterTuning().sysVoltage == 120)
          _LVC = dir > 0 ? 115 : 0;
        else 
          _LVC = dir > 0 ? 230 : 0;
      }
      else {
        _LVC += dir;
        if(NVstore.getHeaterTuning().sysVoltage == 120) {
          if(_LVC < 100)
            _LVC = 0;
          else 
            UPPERLIMIT(_LVC, 125);
        }
        else {
          if(_LVC < 200)
            _LVC = 0;
          else 
            UPPERLIMIT(_LVC, 250);
        }
      }
      break;
  }
}

void
CLVCScreen::_saveNV()
{
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  tuning.lowVolts = _LVC;
  NVstore.setHeaterTuning(tuning);
  NVstore.save();
}