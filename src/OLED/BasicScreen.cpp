/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2019  Ray Jones <ray@mrjones.id.au>
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

#include <Arduino.h>
#include "fonts/Tahoma24.h"
#include "fonts/Arial.h"
#include "BasicScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "../Utility/UtilClasses.h"
#include "../Utility/NVStorage.h"
#include "../Protocol/HeaterManager.h"
#include "../Utility/TempSense.h"
#include "../RTC/RTCStore.h"

#define MAXIFONT tahoma_24ptFontInfo

///////////////////////////////////////////////////////////////////////////
//
// CBasicScreen
//
// This screen provides a basic control function
//
///////////////////////////////////////////////////////////////////////////

CBasicScreen::CBasicScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _showSetModeTime = 0;
  _showModeTime = 0;
  _showAbortTime = 0;
  _feedbackType = 0;
  _nModeSel = 0;
  _bShowOtherSensors = 0;
}

bool 
CBasicScreen::show()
{
  CScreenHeader::show(false);

  char msg[32];
  int xPos, yPos;
  float fTemp;
  bool bShowLargeTemp = true;

  // at bottom of screen show either:
  //   Selection between Fixed or Thermostat mode
  //   Current heat demand setting
  //   Run state of heater

  if(_showAbortTime) {
    long tDelta = millis() - _showAbortTime;
    if(tDelta < 0) {
      switch(_abortreason) {
        case CDemandManager::eStartOK:      strcpy(msg, "Start OK!"); break;
        case CDemandManager::eStartTooWarm: strcpy(msg, "Ignored - too warm!"); break;
        case CDemandManager::eStartSuspend: strcpy(msg, "Suspended - too warm!"); break;
        case CDemandManager::eStartLVC:     strcpy(msg, "Ignored - low voltage!"); break;
        case CDemandManager::eStartLowFuel: strcpy(msg, "Ignored - fuel empty!"); break;
      }
      // centre message at bottom of screen
      _printMenuText(_display.xCentre(), _display.height() - _display.textHeight(), msg, false, eCentreJustify);
    }
    else {
      _showAbortTime = 0;
    }
  }
  else {
    if(_showModeTime) {
      const int border = 3;
      // Show selection between Fixed or Thermostat mode
      long tDelta = millis() - _showModeTime;
      if(tDelta < 0) {

        yPos = _display.height() - _display.textHeight() - border;  // bottom of screen, with room for box

        // display "Fixed Hz" at lower right, allowing space for a selection surrounding box
        strcpy(msg, "Fixed Hz");
        xPos = _display.width() - border;     // set X position to finish short of RHS
        _printMenuText(xPos, yPos, msg, _nModeSel == 1, eRightJustify);

        // display "Thermostat" at lower left, allowing space for a selection surrounding box
        strcpy(msg, "Thermostat");
        xPos = border;
        _printMenuText(xPos, yPos, msg, _nModeSel == 0);

        // setThermostatMode(_nModeSel == 0 ? 1 : 0);    // set the new mode
      }
      else {
        // cancel selection mode, apply whatever is boxed
        _showModeTime = 0;
        _showSetModeTime = millis() + 5000;  // then make the new mode setting be shown
        _bShowOtherSensors = 0;
        _feedbackType = 0;
        _ScreenManager.reqUpdate();
      }
    }
    if((_showModeTime == 0) && _showSetModeTime) {
      showHeaderDetail(true);
      long tDelta = millis() - _showSetModeTime;  
      if(tDelta < 0) {
        switch(_feedbackType) {
          case 0:
            // Show current heat demand setting

            if(CDemandManager::isThermostat()) {
              if(CDemandManager::isExtThermostatMode()) {
//                sprintf(msg, "External @ %.1fHz", getHeaterInfo().getPump_Fixed());
                sprintf(msg, "External @ %.1fHz", HeaterManager.getPumpDemand());
              }
              else {
                float fTemp = CDemandManager::getDegC();
                if(NVstore.getUserSettings().degF) {
                  fTemp = fTemp * 9 / 5 + 32;
                  sprintf(msg, "Setpoint = %.0f`F", fTemp);
                }
                else {
                  sprintf(msg, "Setpoint = %.0f`C", fTemp);
                }
              }
            }
            else {
              sprintf(msg, "Setpoint = %.1fHz", HeaterManager.getPumpDemand());
              // sprintf(msg, "Setpoint = %.1fHz", getHeaterInfo().getPump_Fixed());
            }
            break;
          case 1:
          case 2:
            sprintf(msg, "GPIO output #%d %s", _feedbackType, getGPIOout(_feedbackType-1) ? "ON" : "OFF");
            break;
        }
        // centre message at bottom of screen
        _printMenuText(_display.xCentre(), _display.height() - _display.textHeight(), msg, false, eCentreJustify);

        int numSensors = getTempSensor().getNumSensors();
        if(_bShowOtherSensors && numSensors > 1) {
          bShowLargeTemp = false;
          CTransientFont AF(_display, &arial_8ptFontInfo);
          int yPos = numSensors == 4 ? 14 : 23;
          if(getTempSensor().getTemperature(1, fTemp)) {
            CTempSense::format(msg, fTemp);
          }
          else {
            strcpy(msg, "---");
          }
          _printMenuText(50, yPos, "External:", false, eRightJustify);
          _printMenuText(54, yPos, msg);

          yPos += 13;
          if(numSensors > 2) {
            if(getTempSensor().getTemperature(2, fTemp)) {
              CTempSense::format(msg, fTemp);
            }
            else {
              strcpy(msg, "---");
            }
            _printMenuText(50, yPos, (numSensors == 3) ? "Aux:" : "Aux1:", false, eRightJustify);
            _printMenuText(54, yPos, msg);
          }

          yPos += 13;
          if(numSensors > 3) {
            if(getTempSensor().getTemperature(3, fTemp)) {
              CTempSense::format(msg, fTemp);
            }
            else {
              strcpy(msg, "---");
            }
            _printMenuText(50, yPos, "Aux2:", false, eRightJustify);
            _printMenuText(54, yPos, msg);
          }
        }

      }
      else {
        _showSetModeTime = 0;
      }
    }
  }
  if((_showModeTime == 0) && (_showSetModeTime == 0)) {
    showRunState();
    showHeaderDetail(false);
  }

  if(bShowLargeTemp) {
    float fTemp = getTemperatureSensor(0);  // Primary system sensor - the one used for thermostat modes
    if(fTemp > -80) {
      if(NVstore.getUserSettings().degF) {
        fTemp = fTemp * 9 / 5 + 32;
        sprintf(msg, "%.1f`F", fTemp);
      }
      else {
        sprintf(msg, "%.1f`C", fTemp);
      }

      {
        CTransientFont AF(_display, &MAXIFONT);  // temporarily use a large font
        _printMenuText(_display.xCentre(), 23, msg, false, eCentreJustify);
      }
    }
    else {
      _printMenuText(_display.xCentre(), 25, "No Temperature Sensor", false, eCentreJustify);
    }
  }

  return true;
}


bool 
CBasicScreen::keyHandler(uint8_t event)
{
  static int repeatCount = -1;

  if(event & keyPressed) {
    repeatCount = 0;     // unlock tracking of repeat events
  }

  //
  // use repeat function for key hold detection
  //
  if(event & keyRepeat) {
    if(repeatCount >= 0) {
      repeatCount++;
      // hold LEFT to toggle GPIO output #1
      if(event & key_Left) {
        if(repeatCount > 2) {
          repeatCount = -1;      // prevent double handling
          if(toggleGPIOout(0)) {    // toggle GPIO output #1
            _showSetModeTime = millis() + 5000;
            _bShowOtherSensors = 0;
            _feedbackType = 1;
            _ScreenManager.reqUpdate();
          }
        }
      }
      // hold RIGHT to toggle GPIO output #2
      if(event & key_Right) {
        if(repeatCount > 2) {
          repeatCount = -1;         // prevent double handling
          if(toggleGPIOout(1)) {    // toggle GPIO output #2
            _showSetModeTime = millis() + 5000;
            _bShowOtherSensors = 0;
            _feedbackType = 2;
            _ScreenManager.reqUpdate();
          }
        }
      }
      // hold DOWN to enter thermostat / fixed mode selection
      if(event & key_Down) {
        if(repeatCount > 2) {
          repeatCount = -1;        // prevent double handling
          if(NVstore.getUserSettings().menuMode < 2) {
            _showModeTime = millis() + 5000;
            _nModeSel = CDemandManager::isThermostat() ? 0 : 1;
          }
        }
      }
      // hold UP to toggle degC/degF mode selection
      if(event & key_Up) {
        if(repeatCount > 2) {
          repeatCount = -1;        // prevent double handling
          if(NVstore.getUserSettings().menuMode < 2) {
            _showModeTime = millis() + 5000;
          }
          sUserSettings settings = NVstore.getUserSettings();
          toggle(settings.degF);
          NVstore.setUserSettings(settings);
          NVstore.save();
        }
      }
      // hold CENTRE to turn ON or OFF
      if(event & key_Centre) {
        if(NVstore.getUserSettings().menuMode < 2) {
          // int runstate = getHeaterInfo().getRunStateEx();
          int runstate = HeaterManager.getRunStateEx();
          if(runstate && !RTC_Store.getFrostOn()) {   // running, including cyclic mode idle
            if(repeatCount > 5) {
              repeatCount = -1;
              requestOff();         
            }
          }
          else {  // standard idle state
            // standby, request ON
            if(repeatCount > 3) {
              repeatCount = -1;
              _abortreason = requestOn();
              if(_abortreason != CDemandManager::eStartOK) {
                _showAbortTime = millis() + 5000;
              }
            }
          }
        }
      }
    }
  }

  //
  // key released handling
  //
  if(event & keyReleased) {
    if(!_showModeTime) {
      // release DOWN key to reduce set demand, provided we are not in mode select
      if(NVstore.getUserSettings().menuMode < 2) {
        if(event & key_Down) {
          if(CDemandManager::deltaDemand(-1)) {
            _showSetModeTime = millis() + 5000;
            _bShowOtherSensors = 0;
            _feedbackType = 0;
            _ScreenManager.reqUpdate();
          }
          else 
            _reqOEMWarning();
        }
        // release UP key to increase set demand, provided we are not in mode select
        if(event & key_Up) {
          if(CDemandManager::deltaDemand(+1)) {
            _showSetModeTime = millis() + 5000;
            _bShowOtherSensors = 0;
            _feedbackType = 0;
            _ScreenManager.reqUpdate();
          }
          else 
            _reqOEMWarning();
        }
      }
    }
    if(event & key_Left) {
      if(repeatCount >= 0) {
        if(!_showModeTime) {
          _ScreenManager.prevMenu();
        }
        else {
          if(hasOEMcontroller())
            _reqOEMWarning();
          else {
            _showModeTime = millis() + 5000;
            _nModeSel = 0;
            CDemandManager::setThermostatMode(1);    // set the new mode
          }
          _ScreenManager.reqUpdate();
        }
      }
    }
    if(event & key_Right) {
      if(repeatCount >= 0) {
        if(!_showModeTime)
          _ScreenManager.nextMenu();
        else {
          if(hasOEMcontroller())
            _reqOEMWarning();
          else {
            _showModeTime = millis() + 5000;
            _nModeSel = 1;
            CDemandManager::setThermostatMode(0);    // set the new mode
          }
          _ScreenManager.reqUpdate();
        }
      }
    }
    // release CENTRE to accept new mode, and/or show current setting
    if(event & key_Centre) {
      if(NVstore.getUserSettings().menuMode < 2) {
        if(repeatCount != -2) {  // prevent after off commands
          if(_showModeTime) {
            _showModeTime = millis(); // force immediate cancellation of showmode (via screen update)
          }
          _showSetModeTime = millis() + 5000; 
          _bShowOtherSensors = 1;
          _feedbackType = 0;
        }
        _ScreenManager.reqUpdate();
      }
    }

    repeatCount = -1;
  }
  return true;
}

void 
CBasicScreen::showRunState()
{
  if(NVstore.getUserSettings().menuMode == 2)
    return;

  int runstate = HeaterManager.getRunStateEx(); 
  int errstate = HeaterManager.getErrState(); 
  // int runstate = getHeaterInfo().getRunStateEx(); 
  // int errstate = getHeaterInfo().getErrState(); 

  if(errstate) errstate--;  // correct for +1 biased return value

  static bool toggle = false;
  const char* toPrint = NULL;
  _display.setTextColor(WHITE, BLACK);
    if(errstate && ((runstate == 0) || (runstate > 5))) {

    // flash error code
    char msg[16];
    toggle = !toggle;
    if(toggle) {
      // create an "E-XX" message to display
      sprintf(msg, "E-%02d", errstate);
    }
    else {
      strcpy(msg, "          ");
    }
    int xPos = _display.xCentre();
    int yPos = _display.height() - 2*_display.textHeight();
    _printMenuText(xPos, yPos, msg, false, eCentreJustify);

    // toPrint = getHeaterInfo().getErrStateStr();
    toPrint = HeaterManager.getErrStateStr();
  }
  else {
    if(runstate) {
      // toPrint = getHeaterInfo().getRunStateStr();
      toPrint = HeaterManager.getRunStateStr();
      // simplify starting states
      switch(runstate) {
        case 1:
        case 2:
        case 3:
        case 4:
          toPrint = "Starting"; 
          break;
      }
    }
  }
  if(toPrint) {
    // locate at bottom centre
    _printMenuText(_display.xCentre(), _display.height() - _display.textHeight(), toPrint, false, eCentreJustify);
  }
}
