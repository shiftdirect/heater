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
#include "ExperimentalSettingsScreen.h"
#include "KeyPad.h"
#include "../Protocol/helpers.h"
#include "../Utility/UtilClasses.h"
#include "../Utility/NVStorage.h"


///////////////////////////////////////////////////////////////////////////
//
// CExperimentalSettingsScreen
//
// This screen provides control over experimental features
//
///////////////////////////////////////////////////////////////////////////

static const int Line3 = 14;
static const int Line2 = 27;
static const int Line1 = 40;
static const int Column = 70;

CExperimentalSettingsScreen::CExperimentalSettingsScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _initUI();
  _window = 10;
  _thermoMode = 0;
  _cyclicMode = 0;
}

void 
CExperimentalSettingsScreen::onSelect()
{
  CPasswordScreen::onSelect();
  _initUI();
  _window = NVstore.getThermostatMethodWindow();
  _thermoMode = NVstore.getThermostatMethodMode();
  _cyclicMode = NVstore.getCyclicMode();
}

void
CExperimentalSettingsScreen::_initUI()
{
  _rowSel = 0;
  _animateCount = 0;
}

bool 
CExperimentalSettingsScreen::show()
{
  char msg[20];
  _display.clearDisplay();

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

    if(_rowSel == 4) {
      _printInverted(_display.xCentre(), 0, " Saving Settings ", true, eCentreJustify);
      _printMenuText(_display.xCentre(), 35, "Press UP to", false, eCentreJustify);
      _printMenuText(_display.xCentre(), 43, "confirm save", false, eCentreJustify);
    }
    else {
      _printInverted(_display.xCentre(), 0, " Experimental ", true, eCentreJustify);
      _printMenuText(65, Line3, "Overtemp:", false, eRightJustify);
      _printMenuText(65, Line2, "Thermostat:", false, eRightJustify);
      _printMenuText(65, Line1, "Window:", false, eRightJustify);
      sprintf(msg, "%.1f\367C", _window);  // \367 is octal for Adafruit degree symbol
      _printMenuText(Column, Line1, msg, _rowSel == 1);
      switch(_thermoMode) {
        case 1: 
          _printMenuText(Column, Line2, "Deadband", _rowSel == 2);
          break;
        case 2: 
          _printMenuText(Column, Line2, "Linear Hz", _rowSel == 2);
          break;
        default: 
          _printMenuText(Column, Line2, "Standard", _rowSel == 2);
          break;
      }
      if(_cyclicMode) {
        sprintf(msg, "> %d\367C", _cyclicMode+1);  // \367 is octal for Adafruit degree symbol
      }
      else {
        strcpy(msg, "OFF");
      }
      _printMenuText(Column, Line3, msg, _rowSel == 3);
    }
  }

  return true;
}

bool 
CExperimentalSettingsScreen::animate()
{
  if(_rowSel != 4) {
    int yPos = 53;
    int xPos = _display.xCentre();
    const char* pMsg = NULL;
    switch(_rowSel) {
      case 0:
        _printMenuText(xPos, yPos, " Exit ", true, eCentreJustify);
        _printMenuText(_display.width(), yPos, "\030Edit", false, eRightJustify);
        break;
      case 1:
        _display.drawFastHLine(0, 52, 128, WHITE);
        pMsg = "                    User defined window for custom thermostat modes.                    ";
        _scrollMessage(56, pMsg, _startChar);
        break;
      case 2:
        _display.drawFastHLine(0, 52, 128, WHITE);
        switch(_thermoMode) {
          case 1:
            pMsg = "                   Controller holds measured temperature at the set point whilst within the window.                    ";
            break;
          case 2:
            pMsg = "                   Controller uses Fixed Hz mode, adjusting pump rate linearly across the set point window.                    ";
            break;
          default:
            pMsg = "                   Use heater's standard thermostat control.                    ";
            break;
        }
        if(pMsg)
          _scrollMessage(56, pMsg, _startChar);
        break;
      case 3:
        _display.drawFastHLine(0, 52, 128, WHITE);
        pMsg = "                    Controller auto cycles heater if over temperature occurs.                    ";
        _scrollMessage(56, pMsg, _startChar);
        break;
    }
    return true;
  }
  return false;
}

bool 
CExperimentalSettingsScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // press LEFT to select previous screen
    if(event & key_Left) {
      switch(_rowSel) {
        case 2:
          _startChar = 0;
        case 1:
        case 3:
          _adjust(-1);
          break;
        case 4:
          _rowSel = 0;   // abort save
          break;
      }
    }
    // press RIGHT to select next screen
    if(event & key_Right) {
      switch(_rowSel) {
        case 2:
          _startChar = 0;
        case 1:
        case 3:
          _adjust(+1);
          break;
        case 4:
          _rowSel = 0;   // abort save
          break;
      }
    }
    if(event & key_Down) {
      if(_rowSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::BranchMenu, CScreenManager::FontDumpUI);
      }
      else {
        _rowSel--;
        LOWERLIMIT(_rowSel, 0);
      }
    }
    // UP press
    if(event & key_Up) {
      switch(_rowSel) {
        case 0:
        case 1:
        case 2:
        case 3:
          _startChar = 0;
          _rowSel++;
          UPPERLIMIT(_rowSel, 3);
          break;
        case 4:    // confirmed save
          _showStoringMessage();
          NVstore.setThermostatMethodMode(_thermoMode);
          NVstore.setThermostatMethodWindow(_window);
          NVstore.setCyclicMode(_cyclicMode);
          saveNV();
          _rowSel = 0;
          break;
      }
    }
    // CENTRE press
    if(event & key_Centre) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
          break;
        case 1:
        case 2:
        case 3:
          _rowSel = 4;
          break;
      }
    }
    _ScreenManager.reqUpdate();
  }

  return true;
}

void 
CExperimentalSettingsScreen::_adjust(int dir)
{
  switch(_rowSel) {
    case 1:   // window
      _window += (dir * 0.1);
      UPPERLIMIT(_window, 6.3);
      LOWERLIMIT(_window, 0.2);
      break;
    case 2:   // thermostat mode
      _thermoMode += dir;
      ROLLLOWERLIMIT(_thermoMode, 0, 2);
      ROLLUPPERLIMIT(_thermoMode, 2, 0);
      break;
    case 3:
      _cyclicMode += dir;
      UPPERLIMIT(_cyclicMode, 10);
      LOWERLIMIT(_cyclicMode, 0);
      break;
  }
}
