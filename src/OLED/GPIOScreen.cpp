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
// CGPIOSetupScreen
//
// This screen provides control over GPIO features
//
///////////////////////////////////////////////////////////////////////////

static const int Line3 = 14;
static const int Line2 = 27;
static const int Line1 = 40;
static const int Column1 = 19;
static const int Column2 = 83;

CGPIOSetupScreen::CGPIOSetupScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _initUI();
  _GPIOparams.in1Mode = CGPIOin1::Disabled;
  _GPIOparams.in2Mode = CGPIOin2::Disabled;
  _GPIOparams.out1Mode = CGPIOout1::Disabled;
  _GPIOparams.out2Mode = CGPIOout2::Disabled;
  _GPIOparams.algMode = CGPIOalg::Disabled;
}

void 
CGPIOSetupScreen::onSelect()
{
  CPasswordScreen::onSelect();
  _initUI();
  _GPIOparams = NVstore.getUserSettings().GPIO;
}

void
CGPIOSetupScreen::_initUI()
{
  _rowSel = 0;
  _animateCount = 0;
}

bool 
CGPIOSetupScreen::show()
{
  _display.clearDisplay();

  static bool animated = false;
  animated = !animated;

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

    if(_rowSel == 10) {
      _showConfirmMessage();
    }
    else {
      _showTitle("GPIO Configuration");
      _drawBitmap(0, Line3, InputIconInfo);
      _drawBitmap(11, Line3, _1IconInfo);
      {
        const char* msgText = NULL;
        switch(_GPIOparams.in1Mode) {
          case CGPIOin1::Disabled:  msgText = " --- "; break;
          case CGPIOin1::Start:     msgText = "Start"; break;
          case CGPIOin1::Run:       msgText = "Run  "; break;
          case CGPIOin1::StartStop: msgText = animated ? "Start" : "Stop "; break;
        }
        if(msgText)
          _printMenuText(Column1, Line3, msgText, _rowSel == 3);
      }
      _drawBitmap(0, Line2, InputIconInfo);
      _drawBitmap(11, Line2, _2IconInfo);
      {
        const char* msgText = NULL;
        switch(_GPIOparams.in2Mode) {
          case CGPIOin2::Disabled:    msgText = " --- "; break;
          case CGPIOin2::Stop:        msgText = "Stop "; break;
          case CGPIOin2::Thermostat:  msgText = "\352T   "; break;
        }
        if(msgText)
          _printMenuText(Column1, Line2, msgText, _rowSel == 2);
      }

      _drawBitmap(65, Line3, OutputIconInfo);
      _drawBitmap(75, Line3, _1IconInfo);
      {
        const char* msgText = NULL;
        switch(_GPIOparams.out1Mode) {
          case CGPIOout1::Disabled: msgText = " ---  "; break;
          case CGPIOout1::Status:   msgText = "Status"; break;
          case CGPIOout1::User:     msgText = "User  "; break;
        }
        if(msgText)
          _printMenuText(Column2, Line3, msgText, _rowSel == 5);
      }
      _drawBitmap(65, Line2, OutputIconInfo);
      _drawBitmap(75, Line2, _2IconInfo);
      {
        const char* msgText = NULL;
        switch(_GPIOparams.out2Mode) {
          case CGPIOout2::Disabled: msgText = " ---  "; break;
          case CGPIOout2::User:     msgText = "User  "; break;
        }
        if(msgText)
          _printMenuText(Column2, Line2, msgText, _rowSel == 4);
      }

      if(getBoardRevision() == BRD_V2_FULLGPIO || getBoardRevision() == BRD_V1_FULLGPIO) {
        _drawBitmap(0, Line1-1, algIconInfo);  
        const char* msgText = NULL;
        switch(_GPIOparams.algMode) {
          case CGPIOalg::Disabled: msgText = "Disabled"; break;
          case CGPIOalg::HeatDemand: msgText = "Enabled"; break;
        }
        if(msgText)
          _printMenuText(23, Line1, msgText, _rowSel == 1);
      }
    }
  }

  return true;
}

bool 
CGPIOSetupScreen::animate()
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
            case CGPIOalg::Disabled:   pMsg = "                   Analogue input is ignored.                    "; break;
            case CGPIOalg::HeatDemand: pMsg = "                   Input 1 enables reading of analogue input to set temperature.                    "; break;
          }
          if(pMsg)
            _scrollMessage(56, pMsg, _scrollChar);
          break;

        case 5:
          _display.drawFastHLine(0, 52, 128, WHITE);
          switch(_GPIOparams.out1Mode) {
            case CGPIOout1::Disabled: pMsg = "                   Output 1: DISABLED.                    "; break;
            case CGPIOout1::Status:   pMsg = "                   Output 1: LED status indicator.                    "; break;
            case CGPIOout1::User:     pMsg = "                   Output 1: User controlled.                    "; break;
          }
          if(pMsg)
            _scrollMessage(56, pMsg, _scrollChar);
          break;
        case 4:
          _display.drawFastHLine(0, 52, 128, WHITE);
          switch(_GPIOparams.out2Mode) {
            case CGPIOout2::Disabled: pMsg = "                   Output 2: DISABLED.                    "; break;
            case CGPIOout2::User:     pMsg = "                   Output 2: User controlled.                    "; break;
          }
          if(pMsg)
            _scrollMessage(56, pMsg, _scrollChar);
          break;

        case 3:
          _display.drawFastHLine(0, 52, 128, WHITE);
          switch(_GPIOparams.in1Mode) {
            case CGPIOin1::Disabled:  pMsg = "                   Input 1: DISABLED.                    "; break;
            case CGPIOin1::Start:     pMsg = "                   Input 1: Starts heater upon closure.                    "; break;
            case CGPIOin1::Run:       pMsg = "                   Input 1: Starts heater when held closed, stops when opened.                    "; break;
            case CGPIOin1::StartStop: pMsg = "                   Input 1: Starts or Stops heater upon closure.                    "; break;
          }
          if(pMsg)
            _scrollMessage(56, pMsg, _scrollChar);
          break;
        case 2:
          _display.drawFastHLine(0, 52, 128, WHITE);
          switch(_GPIOparams.in2Mode) {
            case CGPIOin2::Disabled:   pMsg = "                   Input 2: DISABLED.                    "; break;
            case CGPIOin2::Stop:       pMsg = "                   Input 2: Stops heater upon closure.                    "; break;
            case CGPIOin2::Thermostat: pMsg = "                   Input 2: External thermostat. Max fuel when closed, min fuel when open.                    "; break;
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
CGPIOSetupScreen::keyHandler(uint8_t event)
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
CGPIOSetupScreen::_adjust(int dir)
{
  int tVal;
  switch(_rowSel) {
    case 1:   // analogue mode
      tVal = _GPIOparams.algMode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 1);
      _GPIOparams.algMode = (CGPIOalg::Modes)tVal;
      break;
    case 5:   // outputs mode
      tVal = _GPIOparams.out1Mode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 2);
      _GPIOparams.out1Mode = (CGPIOout1::Modes)tVal;
      break;
    case 4:   // outputs mode
      tVal = _GPIOparams.out2Mode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 1);
      _GPIOparams.out2Mode = (CGPIOout2::Modes)tVal;
      break;
    case 3:
      tVal = _GPIOparams.in1Mode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 3);
      _GPIOparams.in1Mode = (CGPIOin1::Modes)tVal;
      break;
    case 2:
      tVal = _GPIOparams.in2Mode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 2);
      _GPIOparams.in2Mode = (CGPIOin2::Modes)tVal;
      break;
  }
}



CGPIOInfoScreen::CGPIOInfoScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreen(display, mgr) 
{
  _keyRepeatCount = -1;
}

void
CGPIOInfoScreen::_initUI()
{
}

bool 
CGPIOInfoScreen::animate()
{
  char msg[16];

  _display.clearDisplay();
  _showTitle("GPIO status");

  _drawBitmap(0, 14, InputIconInfo); 
  _drawBitmap(11, 14, _1IconInfo); 
  _drawBitmap(0, 27, InputIconInfo); 
  _drawBitmap(11, 27, _2IconInfo); 
  _drawBitmap(75, 14, OutputIconInfo); 
  _drawBitmap(86, 14, _1IconInfo); 
  _drawBitmap(75, 27, OutputIconInfo); 
  _drawBitmap(86, 27, _2IconInfo); 

  if(getBoardRevision() == BRD_V2_FULLGPIO || getBoardRevision() == BRD_V1_FULLGPIO)
    _printMenuText(0, Line1, "Analogue:", false, eRightJustify);

  switch(NVstore.getUserSettings().GPIO.in1Mode) {
    case CGPIOin1::Disabled:   
      _drawBitmap(23, 14, CrossLgIconInfo); 
      break;
    case CGPIOin1::Start:      
      _drawBitmap(23, 14, StartIconInfo); 
      break;
    case CGPIOin1::Run:        
      _drawBitmap(23, 14, RunIconInfo); 
      break;  
    case CGPIOin1::StartStop:  
      _drawBitmap(23, 14, StartIconInfo);  
      _drawBitmap(30, 14, StopIconInfo); 
      break;
  }
  _drawBitmap(40, 16, GPIOin.getState(0) ? CloseIconInfo : OpenIconInfo);

  switch(NVstore.getUserSettings().GPIO.in2Mode) {
    case CGPIOin2::Disabled:   
      _drawBitmap(23, 27, CrossLgIconInfo); 
      break;
    case CGPIOin2::Stop:       
      _drawBitmap(23, 27, StopIconInfo); 
      break;
    case CGPIOin2::Thermostat: 
      _printMenuText(23, 27, "\352T"); 
      break;
  }
  _drawBitmap(40, 28, GPIOin.getState(1) ? CloseIconInfo : OpenIconInfo);

  int bulbmode = GPIOout.getState(0);
  static bool iconstate = false;
  switch(NVstore.getUserSettings().GPIO.out1Mode) {
    case CGPIOout1::Disabled: 
      _drawBitmap(99, 14, CrossLgIconInfo); 
      break;
    case CGPIOout1::Status:   
      _drawBitmap(99, 14, InfoIconInfo);  
      if(iconstate && bulbmode == 2)     // animate bulb icon when status is PWM mode
        _drawBitmap(110, 13, BulbOn2IconInfo); 
      else
        _drawBitmap(110, 13, bulbmode ? BulbOnIconInfo : BulbOffIconInfo); 
      iconstate = !iconstate;
      break;
    case CGPIOout1::User:     
      _drawBitmap(99, 15, UserIconInfo);  
      _drawBitmap(110, 13, bulbmode ? BulbOnIconInfo : BulbOffIconInfo); 
      break;
  }

  switch(NVstore.getUserSettings().GPIO.out2Mode) {
    case CGPIOout2::Disabled: _drawBitmap(99, 27, CrossLgIconInfo); break;
    case CGPIOout2::User:     
      _drawBitmap(99, 27, UserIconInfo);  
      _drawBitmap(110, 26, GPIOout.getState(1) ? BulbOnIconInfo : BulbOffIconInfo); 
      break;
  }


  if(getBoardRevision() == BRD_V2_FULLGPIO || getBoardRevision() == BRD_V1_FULLGPIO) {
    _drawBitmap(0, Line1-1, algIconInfo);  
    if(NVstore.getUserSettings().GPIO.algMode == CGPIOalg::Disabled) {
      _drawBitmap(23, Line1, CrossLgIconInfo);
    }
    else {
      sprintf(msg, "%d%%", GPIOalg.getValue() * 100 / 4096);
      _printMenuText(23, Line1, msg);
    }
  }

  _printMenuText(_display.xCentre(), 53, " \021                \020 ", true, eCentreJustify);
  return true;
}

bool 
CGPIOInfoScreen::show()
{
  return false;//  CScreenHeader::show(false);
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

