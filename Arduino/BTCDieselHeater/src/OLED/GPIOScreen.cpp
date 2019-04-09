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
#include "GPIOScreen.h"
#include "KeyPad.h"
#include "../Protocol/helpers.h"
#include "../Utility/UtilClasses.h"
#include "../Utility/NVStorage.h"
#include "../Utility/GPIO.h"

extern CGPIOout GPIOout;
extern CGPIOin GPIOin;

///////////////////////////////////////////////////////////////////////////
//
// CGPIOScreen
//
// This screen provides control over GPIO features
//
///////////////////////////////////////////////////////////////////////////

static const int Line3 = 14;
static const int Line2 = 27;
static const int Line1 = 40;
static const int Column = 58;

CGPIOScreen::CGPIOScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _initUI();
  _GPIOinMode = 0;
  _GPIOoutMode = 0;
  _GPIOalgMode = 0;
}

void 
CGPIOScreen::onSelect()
{
  CPasswordScreen::onSelect();
  _initUI();
  _GPIOinMode = NVstore.getGPIOinMode();
  _GPIOoutMode = NVstore.getGPIOoutMode();
  _GPIOalgMode = NVstore.getGPIOalgMode();
}

void
CGPIOScreen::_initUI()
{
  _rowSel = 0;
  _animateCount = 0;
}

bool 
CGPIOScreen::show()
{
  _display.clearDisplay();

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

    if(_rowSel == 4) {
      _printInverted(_display.xCentre(), 0, " Saving Settings ", true, eCentreJustify);
      _printMenuText(_display.xCentre(), 35, "Press UP to", false, eCentreJustify);
      _printMenuText(_display.xCentre(), 43, "confirm save", false, eCentreJustify);
    }
    else {
      _printInverted(_display.xCentre(), 0, " GPIO Settings ", true, eCentreJustify);
      _printMenuText(55, Line3, "Inputs:", false, eRightJustify);
      _printMenuText(55, Line2, "Outputs:", false, eRightJustify);
      _printMenuText(55, Line1, "Analogue:", false, eRightJustify);
      switch(_GPIOinMode) {
        case 0:
          _printMenuText(Column, Line3, "Disabled", _rowSel == 3);
          break;
        case 1: 
          _printMenuText(Column, Line3, "1-On 2-Off", _rowSel == 3);
          break;
        case 2: 
          _printMenuText(Column, Line3, "1-On 2-\352T", _rowSel == 3);
          break;
        case 3: 
          _printMenuText(Column, Line3, "1-On/Off", _rowSel == 3);
          break;
      }
      switch(_GPIOoutMode) {
        case 0:
          _printMenuText(Column, Line2, "Disabled", _rowSel == 2);
          break;
        case 1:
          _printMenuText(Column, Line2, "1:LED", _rowSel == 2);
          break;
        case 2:
          _printMenuText(Column, Line2, "1&2:User", _rowSel == 2);
          break;
      }
      switch(_GPIOalgMode) {
        case 0:
          _printMenuText(Column, Line1, "Disabled", _rowSel == 1);
          break;
        case 1:
          _printMenuText(Column, Line1, "Ip1 allows", _rowSel == 1);
          break;
      }
    }
  }

  return true;
}

bool 
CGPIOScreen::animate()
{
  if(_rowSel != 4) {
    int yPos = 53;
    int xPos = _display.xCentre();
    const char* pMsg = NULL;
    switch(_rowSel) {
      case 0:
        _printMenuText(xPos, yPos, " \021  \030Edit  Exit   \020 ", true, eCentreJustify);
        break;
      case 1:
        _display.drawFastHLine(0, 52, 128, WHITE);
        switch(_GPIOalgMode) {
          case 0:
            pMsg = "                   Analogue input is ignored.                    ";
            break;
          case 1:
            pMsg = "                   Input 1 enables reading of analogue input to set temperature.                    ";
          break;
        }
        if(pMsg)
          _scrollMessage(56, pMsg, _startChar);
        break;
      case 2:
        _display.drawFastHLine(0, 52, 128, WHITE);
        switch(_GPIOoutMode) {
          case 0:
            pMsg = "                   Digital outputs are disabled.                    ";
            break;
          case 1:
            pMsg = "                   Output1: LED status indicator.                    ";
            break;
          case 2:
            pMsg = "                   Output 1&2: User controlled.                    ";
            break;
        }
        if(pMsg)
          _scrollMessage(56, pMsg, _startChar);
        break;
      case 3:
        _display.drawFastHLine(0, 52, 128, WHITE);
        switch(_GPIOinMode) {
          case 0:
            pMsg = "                   Digital inputs are disabled.                    ";
            break;
          case 1:
            pMsg = "                   Input 1: Starts when closed. Input 2: Stops when closed.                    ";
            break;
          case 2:
            pMsg = "                   Input 1: Starts when held closed, stops when opened. Input2: Max fuel when closed, min fuel when open.                    ";
            break;
          case 3:
            pMsg = "                   Input 1: Starts or Stops when closed.                    ";
            break;
        }
        if(pMsg)
          _scrollMessage(56, pMsg, _startChar);
        break;
    }
    return true;
  }
  return false;
}

bool 
CGPIOScreen::keyHandler(uint8_t event)
{
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
          _startChar = 0;
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
        case 0:
          _ScreenManager.nextMenu();
          break;
        case 1:
        case 2:
        case 3:
          _startChar = 0;
          _adjust(+1);
          break;
        case 4:
          _rowSel = 0;   // abort save
          break;
      }
    }
    if(event & key_Down) {
      _startChar = 0;
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
          _startChar = 0;
          _rowSel++;
          UPPERLIMIT(_rowSel, 3);
          break;
        case 4:    // confirmed save
          _showStoringMessage();
          NVstore.setGPIOinMode(_GPIOinMode);
          NVstore.setGPIOoutMode(_GPIOoutMode);
          NVstore.setGPIOalgMode(_GPIOalgMode);
          saveNV();

          setupGPIO();
          
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
CGPIOScreen::_adjust(int dir)
{
  switch(_rowSel) {
    case 1:   // analogue mode
      _GPIOalgMode += dir;
      UPPERLIMIT(_GPIOalgMode, 1);
      LOWERLIMIT(_GPIOalgMode, 0);
      break;
    case 2:   // outputs mode
      _GPIOoutMode += dir;
      ROLLLOWERLIMIT(_GPIOoutMode, 0, 2);
      ROLLUPPERLIMIT(_GPIOoutMode, 2, 0);
      break;
    case 3:
      _GPIOinMode += dir;
      ROLLUPPERLIMIT(_GPIOinMode, 3, 0);
      ROLLLOWERLIMIT(_GPIOinMode, 0, 3);
      break;
  }
}
