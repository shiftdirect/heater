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
#include "FuelCalScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "../Utility/macros.h"
#include "../Utility/NVStorage.h"
#include "../Protocol/Protocol.h"
#include "fonts/Icons.h"

static const int Line3 = 14;
static const int Line2 = 20;
static const int Line1 = 36;


CFuelCalScreen::CFuelCalScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _initUI();
  _mlPerStroke = 0.02;
  _LVC = 115;
  _tOfs = 0;
}

void 
CFuelCalScreen::onSelect()
{
  CPasswordScreen::onSelect();
  _initUI();
  _mlPerStroke = NVstore.getHeaterTuning().pumpCal;
  _LVC = NVstore.getHeaterTuning().lowVolts;
//  _tOfs = NVstore.getHeaterTuning().DS18B20probe[0].offset;
}

void
CFuelCalScreen::_initUI()
{
  _rowSel = 0;
  _animateCount = 0;
}

bool 
CFuelCalScreen::show()
{
  char msg[20];
  const int col = 90;

  _display.fillRect(0, 50, 128, 14, BLACK);
  _display.fillRect(col-border, Line3-border, 128-(col-1), 64-Line3-border, BLACK);

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

    if(_rowSel == 10) {
      _showConfirmMessage();
    }
    else {
      if(_animateCount < 0) {
        _display.clearDisplay();
        _animateCount = 0;
      }
//      _printInverted(_display.xCentre(), 0, " Special Features ", true, eCentreJustify);
      _showTitle("Special Features");
      // fuel calibration
      int yPos = Line1;
      _printMenuText(col, yPos, "mL/stroke : ", false, eRightJustify);
      sprintf(msg, "%.03f", _mlPerStroke);
      _printMenuText(col, yPos, msg, _rowSel == 1);
      // low voltage cutout
      yPos = Line2;
      _printMenuText(col, yPos, "L.V.C. < ", false, eRightJustify);
      if(_LVC) 
        sprintf(msg, "%.1fV", float(_LVC) * 0.1);
      else
        strcpy(msg, "OFF");
      _printMenuText(col, yPos, msg, _rowSel == 2);
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
  }

  return true;
}


bool 
CFuelCalScreen::animate()
{ 
  if(_animateCount >= 0) {
    switch(_animateCount) {
      case 0:
        _display.fillRect(0, Line3-3, BatteryIconInfo.width, 35, BLACK);
        _drawBitmap(6, Line1-3, FuelIconSmallInfo);
        _drawBitmap(0, Line2-1 , BatteryIconInfo);
//        _drawBitmap(5, Line3-3, miniThermoIconInfo);
        break;
      case 2:
        _display.fillRect(6, Line1-3, FuelIconSmallInfo.width, FuelIconSmallInfo.height, BLACK); // scrub prior drip
        _drawBitmap(6, Line1-2, FuelIconSmallInfo);    // drip fuel
        _display.fillRect(BatteryIconInfo.width - 4, Line2+2, 2, 5, BLACK);   // deplete battery
//        _display.fillRect(7, Line3+3, 2, 2, WHITE);    // grow thermometer
        break;
      case 4:
        _display.fillRect(6, Line1-2, FuelIconSmallInfo.width, FuelIconSmallInfo.height, BLACK); // scrub prior drip
        _drawBitmap(6, Line1-1, FuelIconSmallInfo);    // drip fuel
        _display.fillRect(BatteryIconInfo.width - 7, Line2+2, 2, 5, BLACK);   // deplete battery
//        _display.fillRect(7, Line3+2, 2, 1, WHITE);    // grow thermometer
        break;
      case 6:
        _display.fillRect(6, Line1-1, FuelIconSmallInfo.width, FuelIconSmallInfo.height, BLACK); // scrub prior drip
        _drawBitmap(6, Line1, FuelIconSmallInfo);    // drip fuel
        _display.fillRect(BatteryIconInfo.width - 10, Line2+2, 2, 5, BLACK);   // deplete battery
//        _display.fillRect(7, Line3+1, 2, 1, WHITE);    // grow thermometer
        break;
      case 8:
        _display.fillRect(6, Line1, FuelIconSmallInfo.width, FuelIconSmallInfo.height, BLACK); // scrub prior drip
        _drawBitmap(6, Line1+1, FuelIconSmallInfo);    // drip fuel
        _display.fillRect(BatteryIconInfo.width - 13, Line2+2, 2, 5, BLACK);   // deplete battery
//        _display.fillRect(7, Line3, 2, 1, WHITE);    // grow thermometer
        break;
    }

    _animateCount++;
    WRAPUPPERLIMIT(_animateCount, 9, 0);
  }

  return true;
}


bool 
CFuelCalScreen::keyHandler(uint8_t event)
{
  sHeaterTuning tuning;

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
        case 2:
        case 3:
          _adjust(-1);
          break;
        case 10:
          _rowSel = 0;   // abort save
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
        case 2:
        case 3:
          _adjust(+1);
          break;
        case 10:
          _rowSel = 0;   // abort save
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
        case 1:
        case 2:
        case 3:
          _rowSel++;
//          UPPERLIMIT(_rowSel, 3);
          UPPERLIMIT(_rowSel, 2);
          break;
        case 10:    // confirmed save
          _display.clearDisplay();
          _animateCount = -1;
          _enableStoringMessage();
          tuning = NVstore.getHeaterTuning();
          tuning.pumpCal = _mlPerStroke;
          tuning.lowVolts = _LVC;
//          tuning.DS18B20probe[0].offset = _tOfs;
          NVstore.setHeaterTuning(tuning);
          saveNV();
          _rowSel = 0;
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
        case 2:
        case 3:
          _animateCount = -1;
          _display.clearDisplay();          
          _rowSel = 10;
          break;
      }
    }
    _ScreenManager.reqUpdate();
  }

  return true;
}

void 
CFuelCalScreen::_adjust(int dir)
{
  switch(_rowSel) {
    case 1:   
      _mlPerStroke += dir * 0.001;
      BOUNDSLIMIT(_mlPerStroke, 0.001, 1);
      break;
    case 2:
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
/*    case 3:
      _tOfs += dir * 0.1;
      BOUNDSLIMIT(_tOfs, -10, 10);
      break;
*/
  }
}
