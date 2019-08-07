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
#include "../Utility/NVStorage.h"
#include "../Utility/BTC_GPIO.h"
#include "fonts/Icons.h"
#include "../Utility/BoardDetect.h"

extern CGPIOout GPIOout;
extern CGPIOin GPIOin;
extern CGPIOalg GPIOalg;

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
//static const int Column = 58;
static const int Column = 38;

CGPIOScreen::CGPIOScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _initUI();
  _GPIOparams.inMode = GPIOinNone;
  _GPIOparams.outMode = GPIOoutNone;
  _GPIOparams.algMode = GPIOalgNone;
}

void 
CGPIOScreen::onSelect()
{
  CPasswordScreen::onSelect();
  _initUI();
  _GPIOparams = NVstore.getUserSettings().GPIO;
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
      _showConfirmMessage();
    }
    else {
      _showTitle("GPIO Settings");
      _drawBitmap(10, 14, (getBoardRevision() != BRD_V2_GPIO_NOALG) ? GPIOIconInfo : GPIOIconNoAlgInfo);
      {
        const char* msgText = NULL;
        switch(_GPIOparams.inMode) {
          case GPIOinNone: msgText = "Disabled"; break;
          case GPIOinOn1Off2: msgText = "1:On 2:Off"; break;
          case GPIOinOnHold1: msgText = "1:On 2:\352T"; break;
          case GPIOinOn1Off1: msgText = "1:On/Off"; break;
          case GPIOinExtThermostat2: msgText = "2:\352T"; break;
        }
        if(msgText)
          _printMenuText(Column, Line3, msgText, _rowSel == 3);
      }

      {
        const char* msgText = NULL;
        switch(_GPIOparams.outMode) {
          case GPIOoutNone: msgText = "Disabled"; break;
          case GPIOoutStatus: msgText = "1: Status LED"; break;
          case GPIOoutUser: msgText = "1&2 User"; break;
        }
        if(msgText)
          _printMenuText(Column, Line2, msgText, _rowSel == 2);
      }

      if(getBoardRevision() != BRD_V2_GPIO_NOALG) {  // Not No Analog support
        const char* msgText = NULL;
        switch(_GPIOparams.algMode) {
          case GPIOalgNone: msgText = "Disabled"; break;
          case GPIOalgHeatDemand: msgText = "Ip1 allows"; break;
        }
        if(msgText)
          _printMenuText(Column, Line1, msgText, _rowSel == 1);
      }
    }
  }

  return true;
}

bool 
CGPIOScreen::animate()
{
  CPasswordScreen::animate();
  
  if(!CPasswordScreen::_busy()) {
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
          switch(_GPIOparams.algMode) {
            case GPIOalgNone:       pMsg = "                   Analogue input is ignored.                    "; break;
            case GPIOalgHeatDemand: pMsg = "                   Input 1 enables reading of analogue input to set temperature.                    "; break;
          }
          if(pMsg)
            _scrollMessage(56, pMsg, _scrollChar);
          break;

        case 2:
          _display.drawFastHLine(0, 52, 128, WHITE);
          switch(_GPIOparams.outMode) {
            case GPIOoutNone:   pMsg = "                   Digital outputs are disabled.                    "; break;
            case GPIOoutStatus: pMsg = "                   Output1: LED status indicator.                    "; break;
            case GPIOoutUser:   pMsg = "                   Output 1&2: User controlled.                    "; break;
          }
          if(pMsg)
            _scrollMessage(56, pMsg, _scrollChar);
          break;

        case 3:
          _display.drawFastHLine(0, 52, 128, WHITE);
          switch(_GPIOparams.inMode) {
            case GPIOinNone:    pMsg = "                   Digital inputs are disabled.                    "; break;
            case GPIOinOn1Off2: pMsg = "                   Input 1: Starts upon closure.  Input 2: Stops upon closure.                    "; break;
            case GPIOinOnHold1: pMsg = "                   Input 1: Starts when held closed, stops when opened.  Input2: Max fuel when closed, min fuel when open.                    "; break;
            case GPIOinOn1Off1: pMsg = "                   Input 1: Starts or Stops upon closure.                    "; break;
            case GPIOinExtThermostat2: pMsg = "                   Input 1: not used.  Input 2: Max fuel when closed, min fuel when open.                    "; break;
          }
          if(pMsg)
            _scrollMessage(56, pMsg, _scrollChar);
          break;
      }
      return true;
    }
  }
  return false;
}

bool 
CGPIOScreen::keyHandler(uint8_t event)
{
  sUserSettings us;
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
          _scrollChar = 0;
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
          _scrollChar = 0;
          _adjust(+1);
          break;
        case 4:
          _rowSel = 0;   // abort save
          break;
      }
    }
    if(event & key_Down) {
        _scrollChar = 0;
        _rowSel--;
        if((_rowSel == 1) && (getBoardRevision() == BRD_V2_GPIO_NOALG))  // GPIO but NO analog support
          _rowSel--;   // force skip if analog input is not supported by PCB
        LOWERLIMIT(_rowSel, 0);
    }
    // UP press
    if(event & key_Up) {
      switch(_rowSel) {
        case 0:
          if(getBoardRevision() == BRD_V2_GPIO_NOALG)   // GPIO but NO Analog support
            _rowSel++;   // force skip if analog input is not supported by PCB
        case 1:
        case 2:
        case 3:
          _scrollChar = 0;
          _rowSel++;
          UPPERLIMIT(_rowSel, 3);
          break;
        case 4:    // confirmed save
          _enableStoringMessage();
          us = NVstore.getUserSettings();
          us.GPIO = _GPIOparams;
          NVstore.setUserSettings(us);
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
//          _ScreenManager.selectMenu(CScreenManager::RootMenuLoop, CScreenManager::GPIOInfoUI);  // force return to main menu GPIO view
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
  int tVal;
  switch(_rowSel) {
    case 1:   // analogue mode
      tVal = _GPIOparams.algMode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 1);
      _GPIOparams.algMode = (GPIOalgModes)tVal;
      break;
    case 2:   // outputs mode
      tVal = _GPIOparams.outMode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 2);
      _GPIOparams.outMode = (GPIOoutModes)tVal;
      break;
    case 3:
      tVal = _GPIOparams.inMode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 4);
      _GPIOparams.inMode = (GPIOinModes)tVal;
      break;
  }
}



CGPIOInfoScreen::CGPIOInfoScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _keyRepeatCount = -1;
}

void
CGPIOInfoScreen::_initUI()
{
}

bool 
CGPIOInfoScreen::show()
{
  CScreenHeader::show(false);
  char msg[16];

  _display.writeFillRect(49, 18, 30, 12, WHITE);
  _printInverted(64, 20, "GPIO", true, eCentreJustify);
  _printMenuText(22, 18, "In", false, eCentreJustify);
  _printMenuText(104, 18, "Out", false, eCentreJustify);
  _printMenuText(11, 20, "1", false, eCentreJustify);
  _printMenuText(34, 20, "2", false, eCentreJustify);
  _printMenuText(91, 20, "1", false, eCentreJustify);
  _printMenuText(118, 20, "2", false, eCentreJustify);

  _printMenuText(55, Line1, "Analogue:", false, eRightJustify);

  if(NVstore.getUserSettings().GPIO.inMode == GPIOinNone) {
    _drawBitmap(7, 28, CrossLgIconInfo);
    _drawBitmap(30, 28, CrossLgIconInfo);
  }
  else {
    _drawBitmap(4, 29, GPIOin.getState(0) ? CloseIconInfo : OpenIconInfo);
    if(NVstore.getUserSettings().GPIO.inMode == GPIOinOn1Off1) 
      _drawBitmap(30, 28, CrossLgIconInfo);
    else 
      _drawBitmap(27, 29, GPIOin.getState(1) ? CloseIconInfo : OpenIconInfo);
  }

  
  if(NVstore.getUserSettings().GPIO.outMode == GPIOoutNone) {
    _drawBitmap(87, 28, CrossLgIconInfo);
    _drawBitmap(114, 28, CrossLgIconInfo);
  }
  else {
    _drawBitmap(86, 29, GPIOout.getState(0) ? BulbOnIconInfo : BulbOffIconInfo);
    if(NVstore.getUserSettings().GPIO.outMode == GPIOoutStatus) 
      _drawBitmap(114, 28, CrossLgIconInfo);
    else 
      _drawBitmap(113, 29, GPIOout.getState(1) ? BulbOnIconInfo : BulbOffIconInfo);
  }

  if(NVstore.getUserSettings().GPIO.algMode == GPIOalgNone) {
    _drawBitmap(58, Line1, CrossLgIconInfo);
  }
  else {
    sprintf(msg, "%d%%", GPIOalg.getValue() * 100 / 4096);
    _printMenuText(58, Line1, msg);
  }

  _printMenuText(_display.xCentre(), 53, " \021                \020 ", true, eCentreJustify);
  return true;
}


bool 
CGPIOInfoScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    _keyRepeatCount = 0;     // unlock tracking of repeat events
    // UP press
    if(event & key_Up) {
    }
    // CENTRE press
    if(event & key_Centre) {
    }
    if(event & key_Down) {
      _ScreenManager.selectMenu(CScreenManager::UserSettingsLoop, CScreenManager::GPIOUI);
    }
  }
  if(event & keyRepeat) {
    if(_keyRepeatCount >= 0) {
      _keyRepeatCount++;
      // hold LEFT to toggle GPIO output #1
      if(event & key_Left) {
        if(_keyRepeatCount > 2) {
          _keyRepeatCount = -1;     // prevent double handling
          toggleGPIOout(0);     // toggle GPIO output #1
        }
      }
      // hold RIGHT to toggle GPIO output #2
      if(event & key_Right) {
        if(_keyRepeatCount > 2) {
          _keyRepeatCount = -1;     // prevent double handling
          toggleGPIOout(1);     // toggle GPIO output #2
        }
      }
    }
  }

  // release event
  if(event & keyReleased) {
    if(_keyRepeatCount == 0) {  // short Up press - lower target
      // press LEFT to select previous screen
      if(event & key_Left) {
        _ScreenManager.prevMenu();
      }
      // press RIGHT to select next screen
      if(event & key_Right) {
        _ScreenManager.nextMenu();
      }
    }
    _keyRepeatCount = -1;
  }
  _ScreenManager.reqUpdate();

  return true;
}

