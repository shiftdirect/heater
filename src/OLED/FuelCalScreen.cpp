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
static const int Line2 = 26;
static const int Line1 = 38;


CFuelCalScreen::CFuelCalScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _initUI();
  _mlPerStroke = 0.02;
}

bool 
CFuelCalScreen::onSelect()
{
  CPasswordScreen::onSelect();
  _initUI();
  _mlPerStroke = NVstore.getHeaterTuning().pumpCal;
  _maxUsage = NVstore.getHeaterTuning().maxFuelUsage;
  _warnUsage = NVstore.getHeaterTuning().warnFuelUsage;
  return true;
}

bool 
CFuelCalScreen::show()
{
  char msg[20];
  const int col = 86;

  _display.fillRect(0, 50, 128, 14, BLACK);
  _display.fillRect(col-border, Line3-border, 128-(col-1), 64-Line3-border, BLACK);

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

    if(_animateCount < 0) {
      _display.clearDisplay();
      _animateCount = 0;
    }

    _showTitle("Fuel usage");
    // fuel calibration
    int yPos = Line1;
    _printMenuText(col, yPos, ":", false, eRightJustify);
    _printMenuText(col-7, yPos, "mL/stroke", false, eRightJustify);
    sprintf(msg, "%.03f", _mlPerStroke);
    _printMenuText(col+2, yPos, msg, _rowSel == 1);
    
    // tank calibration
    _drawBitmap(6, Line3+3, BowserIconInfo);
    yPos = Line3;
    _printMenuText(col, yPos, ":", false, eRightJustify);
    _printMenuText(col-7, yPos, "Maximum", false, eRightJustify);
    if(_maxUsage != 0)
      sprintf(msg, "%.1fL", float(_maxUsage) * 0.1f);
    else
      strcpy(msg, "Off");
    _printMenuText(col+2, yPos, msg, _rowSel == 3);

    yPos = Line2;
    _printMenuText(col, yPos, ":", false, eRightJustify);
    _printMenuText(col-7, yPos, "Warning", false, eRightJustify);
    if(_maxUsage != 0)
      sprintf(msg, "%.1fL", float(_warnUsage) * 0.1f);
    else 
      strcpy(msg, "n/a");
    _printMenuText(col+2, yPos, msg, _rowSel == 2);
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
CFuelCalScreen::animate()
{ 
  if(_saveBusy()) {
    return false;
  }

  if(_animateCount >= 0) {
    switch(_animateCount) {
      case 0:
        _display.fillRect(6, Line1-3, FuelIconSmallInfo.width, FuelIconSmallInfo.height+4, BLACK); // scrub prior drip
        _drawBitmap(6, Line1-3, FuelIconSmallInfo);
        break;
      case 2:
        _display.fillRect(6, Line1-3, FuelIconSmallInfo.width, FuelIconSmallInfo.height, BLACK); // scrub prior drip
        _drawBitmap(6, Line1-2, FuelIconSmallInfo);    // drip fuel
        break;
      case 4:
        _display.fillRect(6, Line1-2, FuelIconSmallInfo.width, FuelIconSmallInfo.height, BLACK); // scrub prior drip
        _drawBitmap(6, Line1-1, FuelIconSmallInfo);    // drip fuel
        break;
      case 6:
        _display.fillRect(6, Line1-1, FuelIconSmallInfo.width, FuelIconSmallInfo.height, BLACK); // scrub prior drip
        _drawBitmap(6, Line1, FuelIconSmallInfo);    // drip fuel
        break;
      case 8:
        _display.fillRect(6, Line1, FuelIconSmallInfo.width, FuelIconSmallInfo.height, BLACK); // scrub prior drip
        _drawBitmap(6, Line1+1, FuelIconSmallInfo);    // drip fuel
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
        case 2:
        case 3:
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
        case 2:
        case 3:
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
        case 1:
        case 2:
        case 3:
          _rowSel++;
          UPPERLIMIT(_rowSel, 3);
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
CFuelCalScreen::_adjust(int dir)
{
  switch(_rowSel) {
    case 1:   
      _mlPerStroke += dir * 0.001;
      BOUNDSLIMIT(_mlPerStroke, 0.001, 1);
      break;
    case 2:
      _warnUsage += dir;
      BOUNDSLIMIT(_warnUsage, 0, 100);
      break;
    case 3:
      _maxUsage += dir;
      BOUNDSLIMIT(_maxUsage, 0, 10000);
      break;
  }
}

void
CFuelCalScreen::_saveNV()
{
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  tuning.pumpCal = _mlPerStroke;
  tuning.maxFuelUsage = _maxUsage;
  tuning.warnFuelUsage = _warnUsage;
  NVstore.setHeaterTuning(tuning);
  NVstore.save();
}