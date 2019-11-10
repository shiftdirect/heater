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
#include "GPIOSetupScreen.h"
#include "KeyPad.h"
#include "../Utility/NVStorage.h"
#include "../Utility/BTC_GPIO.h"
#include "fonts/Icons.h"
#include "../Utility/BoardDetect.h"

extern CGPIOout GPIOout;
extern CGPIOin GPIOin;
extern CGPIOalg GPIOalg;

int8_t s8abs(int8_t val) 
{
  if(val >= 0)
    return val;
  else
    return -val;
}

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
static const int Column2 = 88;

CGPIOSetupScreen::CGPIOSetupScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _initUI();
  _GPIOparams.in1Mode = CGPIOin1::Disabled;
  _GPIOparams.in2Mode = CGPIOin2::Disabled;
  _GPIOparams.out1Mode = CGPIOout1::Disabled;
  _GPIOparams.out2Mode = CGPIOout2::Disabled;
  _GPIOparams.algMode = CGPIOalg::Disabled;
  _GPIOparams.thresh[0] = 0;
  _GPIOparams.thresh[1] = 0;
  _ExtHold = 0;
}

void 
CGPIOSetupScreen::onSelect()
{
  CPasswordScreen::onSelect();
  _initUI();
  _GPIOparams = NVstore.getUserSettings().GPIO;
  _ExtHold = NVstore.getUserSettings().ExtThermoTimeout;
  _repeatCount = -1;
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
  char msg[16];

  _display.clearDisplay();

  static bool animated = false;
  animated = !animated;

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

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
        case CGPIOin1::Stop:      msgText = "Stop "; break;
      }
      if(msgText)
        _printMenuText(Column1, Line3, msgText, _rowSel == 4);
    }
    _drawBitmap(0, Line2, InputIconInfo);
    _drawBitmap(11, Line2, _2IconInfo);
    {
      const char* msgText = NULL;
      switch(_GPIOparams.in2Mode) {
        case CGPIOin2::Disabled:    msgText = " --- "; break;
        case CGPIOin2::Stop:        msgText = "Stop "; break;
        case CGPIOin2::Thermostat:  msgText = "\352T"; break;
      }
      if(msgText)
        _printMenuText(Column1, Line2, msgText, _rowSel == 2);
      
      if(_GPIOparams.in2Mode == CGPIOin2::Thermostat) {
        _drawBitmap(Column1 + 13, Line2-2, TimerIconInfo);
        const char* modeStr = "No";
        switch(_ExtHold) {
          case 60000: modeStr = "1m"; break;
          case 120000: modeStr = "2m"; break;
          case 300000: modeStr = "5m"; break;
          case 600000: modeStr = "10m"; break;
          case 900000: modeStr = "15m"; break;
          case 1200000: modeStr = "20m"; break;
          case 1800000: modeStr = "30m"; break;
          case 3600000: modeStr = "1hr"; break;
        }
        _printMenuText(Column1 + 29, Line2, modeStr, _rowSel == 3);
      }
    }

    _drawBitmap(70, Line3, OutputIconInfo);
    _drawBitmap(80, Line3, _1IconInfo);
    {
      const char* msgText = NULL;
      switch(_GPIOparams.out1Mode) {
        case CGPIOout1::Disabled: msgText = "---"; break;
        case CGPIOout1::Status:   msgText = "Status"; break;
        case CGPIOout1::User:     msgText = "User"; break;
        case CGPIOout1::Thresh:   
          if(_rowSel == 6) {
            sprintf(msg, "  %d`C", s8abs(_GPIOparams.thresh[0]));
            _printMenuText(Column2, Line3, msg, false);
            _printMenuText(Column2, Line3, _GPIOparams.thresh[0] >= 0 ? ">" : "<", true);
          }
          else {
            sprintf(msg, "%s %d`C", _GPIOparams.thresh[0] >= 0 ? ">" : "<", s8abs(_GPIOparams.thresh[0]));
            _printMenuText(Column2, Line3, msg, _rowSel == 8);
          }
          break;
      }
      if(msgText)
        _printMenuText(Column2, Line3, msgText, _rowSel == 6);
    }
    _drawBitmap(70, Line2, OutputIconInfo);
    _drawBitmap(80, Line2, _2IconInfo);
    {
      const char* msgText = NULL;
      switch(_GPIOparams.out2Mode) {
        case CGPIOout2::Disabled: msgText = "---"; break;
        case CGPIOout2::User:     msgText = "User"; break;
        case CGPIOout2::Thresh:   
          if(_rowSel == 5) {
            sprintf(msg, "  %d`C", s8abs(_GPIOparams.thresh[1]));
            _printMenuText(Column2, Line2, msg, false);
            _printMenuText(Column2, Line2, _GPIOparams.thresh[1] >= 0 ? ">" : "<", true);
          }
          else {
            sprintf(msg, "%s %d`C", _GPIOparams.thresh[1] >= 0 ? ">" : "<", s8abs(_GPIOparams.thresh[1]));
            _printMenuText(Column2, Line2, msg, _rowSel == 7);
          }
          break;
      }
      if(msgText)
        _printMenuText(Column2, Line2, msgText, _rowSel == 5);
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

  return true;
}

bool 
CGPIOSetupScreen::animate()
{
  CPasswordScreen::animate();
  
  if(!CPasswordScreen::_busy()) {
    if(_rowSel != SaveConfirm) {
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
        case 3:
          _display.drawFastHLine(0, 52, 128, WHITE);
          pMsg = "                   Input 2: External thermostat heater control. Start heater upon closure, stop after open for specified period.                    "; 
          _scrollMessage(56, pMsg, _scrollChar);
          break;
        case 4:
          _display.drawFastHLine(0, 52, 128, WHITE);
          switch(_GPIOparams.in1Mode) {
            case CGPIOin1::Disabled:  pMsg = "                   Input 1: DISABLED.                    "; break;
            case CGPIOin1::Start:     pMsg = "                   Input 1: Starts heater upon closure.                    "; break;
            case CGPIOin1::Run:       pMsg = "                   Input 1: Starts heater when held closed, stops when opened.                    "; break;
            case CGPIOin1::StartStop: pMsg = "                   Input 1: Starts or Stops heater upon closure.                    "; break;
            case CGPIOin1::Stop:      pMsg = "                   Input 1: Stops heater upon closure.                    "; break;
          }
          if(pMsg)
            _scrollMessage(56, pMsg, _scrollChar);
          break;

        case 5:
          _display.drawFastHLine(0, 52, 128, WHITE);
          switch(_GPIOparams.out2Mode) {
            case CGPIOout2::Disabled: pMsg = "                   Output 2: DISABLED.                    "; break;
            case CGPIOout2::User:     pMsg = "                   Output 2: User controlled.                    "; break;
            case CGPIOout2::Thresh:   
              if(_GPIOparams.thresh[1] >= 0)
                pMsg = "                   Output 2: Active if over temperature. Hold LEFT to set under. Hold RIGHT to set over.                   "; 
              else
                pMsg = "                   Output 2: Active if under temperature. Hold LEFT to set under. Hold RIGHT to set over.                   ";
              break;
          }
          if(pMsg)
            _scrollMessage(56, pMsg, _scrollChar);
          break;
        case 6:
          _display.drawFastHLine(0, 52, 128, WHITE);
          switch(_GPIOparams.out1Mode) {
            case CGPIOout1::Disabled: pMsg = "                   Output 1: DISABLED.                    "; break;
            case CGPIOout1::Status:   pMsg = "                   Output 1: LED status indicator.                    "; break;
            case CGPIOout1::User:     pMsg = "                   Output 1: User controlled.                    "; break;
            case CGPIOout1::Thresh:   
              if(_GPIOparams.thresh[0] >= 0)
                pMsg = "                   Output 1: Active if over temperature. Hold LEFT to set under. Hold RIGHT to set over.                   "; 
              else
                pMsg = "                   Output 1: Active if under temperature. Hold LEFT to set under. Hold RIGHT to set over.                   ";
              break;
          }
          if(pMsg)
            _scrollMessage(56, pMsg, _scrollChar);
          break;
        case 7:
          _display.drawFastHLine(0, 52, 128, WHITE);
          if(_GPIOparams.thresh[1] >= 0)
            pMsg = "                   Output 2: Active if over temperature. CENTRE to accept. Hold LEFT to set under.                   "; 
          else
            pMsg = "                   Output 2: Active if under temperature. CENTRE to accept. Hold RIGHT to set over.                   ";
          _scrollMessage(56, pMsg, _scrollChar);
          break;
        case 8:
          _display.drawFastHLine(0, 52, 128, WHITE);
          if(_GPIOparams.thresh[0] >= 0)
            pMsg = "                   Output 1: Active if over temperature. Centre to accept. Hold LEFT to set under.                   "; 
          else
            pMsg = "                   Output 1: Active if under temperature. Centre to accept. Hold RIGHT to set over.                   ";
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
  if(CPasswordScreen::keyHandler(event)) {  // handle confirm save
    return true;
  }

  if(event & keyPressed) {
    _repeatCount = 0;
  }
  
  if(event & keyRepeat) {
    if(_repeatCount >= 0) {
      _repeatCount++;
      // hold LEFT to toggle GPIO output #1
      if(event & key_Left) {
        if(_repeatCount > 1) {
          _repeatCount = -1;      // prevent double handling
          if((_rowSel == 6 || _rowSel == 8) && _GPIOparams.out1Mode == CGPIOout1::Thresh) {
            _GPIOparams.thresh[0] = -s8abs(_GPIOparams.thresh[0]);
            BOUNDSLIMIT(_GPIOparams.thresh[0], -50, -1);
            _rowSel = 8;
          }
          if((_rowSel == 5 || _rowSel == 7)  && _GPIOparams.out2Mode == CGPIOout2::Thresh) {
            _GPIOparams.thresh[1] = -s8abs(_GPIOparams.thresh[1]);
            BOUNDSLIMIT(_GPIOparams.thresh[1], -50, -1);
            _rowSel = 7;
          }
        }
      }
      if(event & key_Right) {
        if(_repeatCount > 1) {
          _repeatCount = -1;      // prevent double handling
          if((_rowSel == 6 || _rowSel == 8)  && _GPIOparams.out1Mode == CGPIOout1::Thresh) {
            _GPIOparams.thresh[0] = s8abs(_GPIOparams.thresh[0]);
            BOUNDSLIMIT(_GPIOparams.thresh[0], 0, 50);
            _rowSel = 8;
          }
          if((_rowSel == 5 || _rowSel == 7) && _GPIOparams.out2Mode == CGPIOout2::Thresh) {
            _GPIOparams.thresh[1] = s8abs(_GPIOparams.thresh[1]);
            BOUNDSLIMIT(_GPIOparams.thresh[1], 0, 50);
            _rowSel = 7;
          }
        }
      }
    }
  }

  if(event & keyReleased) {
    // press LEFT to select previous screen
    if(_repeatCount == 0) {
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
          case 6:
          case 7:
          case 8:
            _scrollChar = 0;
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
          case 4:
          case 5:
          case 6:
          case 7:
          case 8:
            _scrollChar = 0;
            _adjust(+1);
            break;
        }
      }
      if(event & key_Down) {
        _scrollChar = 0;
        _rowSel--;
        if((_rowSel == 3) && (_GPIOparams.in2Mode != CGPIOin2::Thermostat))        
          _rowSel--;   // force skip if not set to external thermostat
        if((_rowSel == 1) && ((getBoardRevision() == BRD_V2_GPIO_NOALG) || (getBoardRevision() == BRD_V3_GPIO_NOALG)))  // GPIO but NO analog support
          _rowSel--;   // force skip if analog input is not supported by PCB
        LOWERLIMIT(_rowSel, 0);
      }
      // UP press
      if(event & key_Up) {
        switch(_rowSel) {
          case 0:
            if((getBoardRevision() == BRD_V2_GPIO_NOALG) ||  (getBoardRevision() == BRD_V3_GPIO_NOALG))   // GPIO but NO Analog support
              _rowSel++;   // force skip if analog input is not supported by PCB
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
            _scrollChar = 0;
            _rowSel++;
            if((_rowSel == 3) && (_GPIOparams.in2Mode != CGPIOin2::Thermostat))        
              _rowSel++;   // force skip if not set to external thermostat
            UPPERLIMIT(_rowSel, 6);
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
          case 6:
            _rowSel = SaveConfirm;
            break;
          case 7:
            _rowSel = 5;
            break;
          case 8:
            _rowSel = 6;
            break;
        }
      }
    }
    _repeatCount = -1;
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
    case 2:
      tVal = _GPIOparams.in2Mode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 2);
      _GPIOparams.in2Mode = (CGPIOin2::Modes)tVal;
      break;
    case 3:
      switch(_ExtHold) {
        case 0: _ExtHold = (dir > 0) ? 60000 : 0; break;
        case 60000: _ExtHold = (dir > 0) ? 120000 : 0; break;
        case 120000: _ExtHold = (dir > 0) ? 300000 : 60000; break;
        case 300000: _ExtHold = (dir > 0) ? 600000 : 120000; break;
        case 600000: _ExtHold = (dir > 0) ? 900000 : 300000; break;
        case 900000: _ExtHold = (dir > 0) ? 1200000 : 600000; break;
        case 1200000: _ExtHold = (dir > 0) ? 1800000 : 900000; break;
        case 1800000: _ExtHold = (dir > 0) ? 3600000 : 1200000; break;
        case 3600000: _ExtHold = (dir > 0) ? 3600000 : 1800000; break;
        default: _ExtHold = 0; break;
      }
      break;
    case 4:
      tVal = _GPIOparams.in1Mode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 4);
      _GPIOparams.in1Mode = (CGPIOin1::Modes)tVal;
      break;
    case 5:   // outputs mode
      tVal = _GPIOparams.out2Mode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 2);
      _GPIOparams.out2Mode = (CGPIOout2::Modes)tVal;
      break;
    case 6:   // outputs mode
      tVal = _GPIOparams.out1Mode;
      tVal += dir;
      WRAPLIMITS(tVal, 0, 3);
      _GPIOparams.out1Mode = (CGPIOout1::Modes)tVal;
      break;
    case 7:
      if(_GPIOparams.thresh[1] < 0) {
        _GPIOparams.thresh[1] += -dir;
        BOUNDSLIMIT(_GPIOparams.thresh[1], -50, -1);
      }
      else {
        _GPIOparams.thresh[1] += dir;
        BOUNDSLIMIT(_GPIOparams.thresh[1], 0, 50);
      }
      break;
    case 8:
      if(_GPIOparams.thresh[0] < 0) {
        _GPIOparams.thresh[0] += -dir;
        BOUNDSLIMIT(_GPIOparams.thresh[0], -50, -1);
      }
      else {
        _GPIOparams.thresh[0] += dir;
        BOUNDSLIMIT(_GPIOparams.thresh[0], 0, 50);
      }
      break;
  }
}


void
CGPIOSetupScreen::_saveNV()
{
  sUserSettings us = NVstore.getUserSettings();
  us.GPIO = _GPIOparams;
  us.ExtThermoTimeout = _ExtHold;
  NVstore.setUserSettings(us);
  NVstore.save();

  setupGPIO();
}
  
