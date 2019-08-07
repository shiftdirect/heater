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
static const int Column1 = 26;
static const int Column2 = 85;

CGPIOScreen::CGPIOScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _initUI();
  _GPIOparams.in1Mode = GPIOin1None;
  _GPIOparams.in2Mode = GPIOin2None;
  _GPIOparams.out1Mode = GPIOout1None;
  _GPIOparams.out2Mode = GPIOout2None;
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

  static bool animated = false;
  animated = !animated;

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

    if(_rowSel == 10) {
      _showConfirmMessage();
    }
    else {
      _showTitle("GPIO Settings");
      _drawBitmap(0, 14, (getBoardRevision() != BRD_V2_GPIO_NOALG) ? GPIOIconInfo : GPIOIconNoAlgInfo);
      {
        const char* msgText = NULL;
        switch(_GPIOparams.in1Mode) {
          case GPIOin1None:  msgText = "1: --- "; break;
          case GPIOin1On:    msgText = "1:Start"; break;
          case GPIOin1Hold:  msgText = "1: Run "; break;
          case GPIOin1OnOff: msgText = animated ? "1:Start" : "1: Stop"; break;
        }
        if(msgText)
          _printMenuText(Column1, Line3, msgText, _rowSel == 4);
      }
      {
        const char* msgText = NULL;
        switch(_GPIOparams.in2Mode) {
          case GPIOin2None:           msgText = "2: ---"; break;
          case GPIOin2Off:            msgText = "2:Stop"; break;
          case GPIOin2ExtThermostat:  msgText = "2: \352T "; break;
        }
        if(msgText)
          _printMenuText(Column2, Line3, msgText, _rowSel == 5);
      }

      {
        const char* msgText = NULL;
        switch(_GPIOparams.out1Mode) {
          case GPIOout1None:   msgText = "1: ---  "; break;
          case GPIOout1Status: msgText = "1:Status"; break;
          case GPIOout1User:   msgText = "1: User "; break;
        }
        if(msgText)
          _printMenuText(Column1, Line2, msgText, _rowSel == 2);
      }
      {
        const char* msgText = NULL;
        switch(_GPIOparams.out2Mode) {
          case GPIOout2None: msgText = "2: ---"; break;
          case GPIOout2User: msgText = "2:User"; break;
        }
        if(msgText)
          _printMenuText(Column2, Line2, msgText, _rowSel == 3);
      }

      if(getBoardRevision() != BRD_V2_GPIO_NOALG) {  // Not No Analog support
        const char* msgText = NULL;
        switch(_GPIOparams.algMode) {
          case GPIOalgNone: msgText = "Disabled"; break;
          case GPIOalgHeatDemand: msgText = "Ip1 allows"; break;
        }
        if(msgText)
          _printMenuText(Column1, Line1, msgText, _rowSel == 1);
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
    if(_rowSel != 10) {
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
          switch(_GPIOparams.out1Mode) {
            case GPIOout1None:   pMsg = "                   Output 1: DISABLED.                    "; break;
            case GPIOout1Status: pMsg = "                   Output 1: LED status indicator.                    "; break;
            case GPIOout1User:   pMsg = "                   Output 1: User controlled.                    "; break;
          }
          if(pMsg)
            _scrollMessage(56, pMsg, _scrollChar);
          break;
        case 3:
          _display.drawFastHLine(0, 52, 128, WHITE);
          switch(_GPIOparams.out2Mode) {
            case GPIOout2None:   pMsg = "                   Output 2: DISABLED.                    "; break;
            case GPIOout2User:   pMsg = "                   Output 2: User controlled.                    "; break;
          }
          if(pMsg)
            _scrollMessage(56, pMsg, _scrollChar);
          break;

        case 4:
          _display.drawFastHLine(0, 52, 128, WHITE);
          switch(_GPIOparams.in1Mode) {
            case GPIOin1None:   pMsg = "                   Input 1: DISABLED.                    "; break;
            case GPIOin1On:     pMsg = "                   Input 1: Starts heater upon closure.                    "; break;
            case GPIOin1Hold:   pMsg = "                   Input 1: Starts heater when held closed, stops when opened.                    "; break;
            case GPIOin1OnOff:  pMsg = "                   Input 1: Starts or Stops heater upon closure.                    "; break;
          }
          if(pMsg)
            _scrollMessage(56, pMsg, _scrollChar);
          break;
        case 5:
          _display.drawFastHLine(0, 52, 128, WHITE);
          switch(_GPIOparams.in2Mode) {
            case GPIOin2None:          pMsg = "                   Input 2: DISABLED.                    "; break;
            case GPIOin2Off:           pMsg = "                   Input 2: Stops heater upon closure.                    "; break;
            case GPIOin2ExtThermostat: pMsg = "                   Input 2: External thermostat. Max fuel when closed, min fuel when open.                    "; break;
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
        case 4:
        case 5:
          _scrollChar = 0;
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
        case 4:
        case 5:
          _scrollChar = 0;
          _adjust(+1);
          break;
        case 10:
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
        case 4:
        case 5:
          _scrollChar = 0;
          _rowSel++;
          UPPERLIMIT(_rowSel, 5);
          break;
        case 10:    // confirmed save
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
        case 4:
        case 5:
          _rowSel = 10;
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
      tVal = _GPIOparams.out1Mode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 2);
      _GPIOparams.out1Mode = (GPIOout1Modes)tVal;
      break;
    case 3:   // outputs mode
      tVal = _GPIOparams.out2Mode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 1);
      _GPIOparams.out2Mode = (GPIOout2Modes)tVal;
      break;
    case 4:
      tVal = _GPIOparams.in1Mode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 3);
      _GPIOparams.in1Mode = (GPIOin1Modes)tVal;
      break;
    case 5:
      tVal = _GPIOparams.in2Mode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 2);
      _GPIOparams.in2Mode = (GPIOin2Modes)tVal;
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

  if(getBoardRevision() == BRD_V2_FULLGPIO || getBoardRevision() == BRD_V1_FULLGPIO)
    _printMenuText(55, Line1, "Analogue:", false, eRightJustify);

  if(NVstore.getUserSettings().GPIO.in1Mode == GPIOin1None) {
    _drawBitmap(7, 28, CrossLgIconInfo);
  }
  else {
    _drawBitmap(4, 29, GPIOin.getState(0) ? CloseIconInfo : OpenIconInfo);
  }
  if(NVstore.getUserSettings().GPIO.in2Mode == GPIOin2None) {
    _drawBitmap(30, 28, CrossLgIconInfo);
  }
  else {
    _drawBitmap(27, 29, GPIOin.getState(1) ? CloseIconInfo : OpenIconInfo);
  }

  
  if(NVstore.getUserSettings().GPIO.out1Mode == GPIOout1None) {
    _drawBitmap(87, 28, CrossLgIconInfo);
  }
  else {
    _drawBitmap(86, 29, GPIOout.getState(0) ? BulbOnIconInfo : BulbOffIconInfo);
  }
  if(NVstore.getUserSettings().GPIO.out2Mode == GPIOout2None) {
    _drawBitmap(114, 28, CrossLgIconInfo);
  }
  else {
    _drawBitmap(113, 29, GPIOout.getState(1) ? BulbOnIconInfo : BulbOffIconInfo);
  }


  if(getBoardRevision() == BRD_V2_FULLGPIO || getBoardRevision() == BRD_V1_FULLGPIO) {
    if(NVstore.getUserSettings().GPIO.algMode == GPIOalgNone) {
      _drawBitmap(58, Line1, CrossLgIconInfo);
    }
    else {
      sprintf(msg, "%d%%", GPIOalg.getValue() * 100 / 4096);
      _printMenuText(58, Line1, msg);
    }
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

