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
#include "fonts/tahoma16.h"
#include "fonts/tahoma24.h"
#include "fonts/Icons.h"
#include "BasicScreen.h"
#include "KeyPad.h"
#include "../Protocol/helpers.h"
#include "../Utility/UtilClasses.h"
#include "../Utility/NVStorage.h"


#define MAXIFONT tahoma_24ptFontInfo
//#define MAXIFONT tahoma_16ptFontInfo

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
  _feedbackType = 0;
  _nModeSel = 0;
}

bool 
CBasicScreen::show()
{
  CScreenHeader::show();

  char msg[20];
  int xPos, yPos;

  float fTemp = getTemperatureSensor();
  if(fTemp > -80) {
    if(NVstore.getDegFMode()) {
      fTemp = fTemp * 9 / 5 + 32;
      sprintf(msg, "%.1f`F", fTemp);
    }
    else {
      sprintf(msg, "%.1f`C", fTemp);
    }

    {
      CTransientFont AF(_display, &MAXIFONT);  // temporarily use a large font
      _printMenuText(_display.xCentre(), 23, msg, false, eCentreJustify);
//      _printMenuText(_display.xCentre(), 25, msg, false, eCentreJustify);
    }
  }
  else {
    _printMenuText(_display.xCentre(), 25, "No Temperature Sensor", false, eCentreJustify);
  }


  // at bottom of screen show either:
  //   Selection between Fixed or Thermostat mode
  //   Current heat demand setting
  //   Run state of heater
  
  if(_showModeTime) {
    const int border = 3;
    const int radius = 4;
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
      _feedbackType = 0;
      _ScreenManager.reqUpdate();
    }
  }
  if((_showModeTime == 0) && _showSetModeTime) {
    long tDelta = millis() - _showSetModeTime;  
    if(tDelta < 0) {
      switch(_feedbackType) {
        case 0:
          // Show current heat demand setting

          if(getThermostatModeActive()) {
            float fTemp = getTemperatureDesired();
            if(NVstore.getDegFMode()) {
              fTemp = fTemp * 9 / 5 + 32;
              sprintf(msg, "Setpoint = %.0f`F", fTemp);
            }
            else {
              sprintf(msg, "Setpoint = %.0f`C", fTemp);
            }
          }
          else {
            sprintf(msg, "Setpoint = %.1fHz", getHeaterInfo().getPump_Fixed());
          }
          break;
        case 1:
        case 2:
          sprintf(msg, "GPIO output #%d %s", _feedbackType, getGPIO(_feedbackType-1) ? "ON" : "OFF");
          break;
      }
      // centre message at bottom of screen
      _printMenuText(_display.xCentre(), _display.height() - _display.textHeight(), msg, false, eCentreJustify);
    }
    else {
      _showSetModeTime = 0;
    }
  }
  if((_showModeTime == 0) && (_showSetModeTime == 0)) {
    showRunState();
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
          repeatCount = -1;         // prevent double handling
          setGPIO(0, !getGPIO(0));  // toggle GPIO output #1
          _showSetModeTime = millis() + 2000;
          _feedbackType = 1;
          _ScreenManager.reqUpdate();
        }
      }
      // hold RIGHT to toggle GPIO output #2
      if(event & key_Right) {
        if(repeatCount > 2) {
          repeatCount = -1;         // prevent double handling
          setGPIO(1, !getGPIO(1));  // toggle GPIO output #2
          _showSetModeTime = millis() + 2000;
          _feedbackType = 2;
          _ScreenManager.reqUpdate();
        }
      }
      // hold DOWN to enter thermostat / fixed mode selection
      if(event & key_Down) {
        if(repeatCount > 2) {
          repeatCount = -1;        // prevent double handling
          _showModeTime = millis() + 5000;
          _nModeSel = getThermostatModeActive() ? 0 : 1;
        }
      }
      // hold UP to toggle degC/degF mode selection
      if(event & key_Up) {
        if(repeatCount > 2) {
          repeatCount = -1;        // prevent double handling
          _showModeTime = millis() + 5000;
          NVstore.setDegFMode(NVstore.getDegFMode() ? 0 : 1);
        }
      }
      // hold CENTRE to turn ON or OFF
      if(event & key_Centre) {
        int runstate = getHeaterInfo().getRunStateEx();
        if(runstate) {   // running, including cyclic mode idle
          if(repeatCount > 5) {
            repeatCount = -1;
            requestOff();         
          }
        }
        else {  // standard idle state
          // standby, request ON
          if(repeatCount > 3) {
            repeatCount = -1;
            requestOn();
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
      if(event & key_Down) {
        if(reqTempDelta(-1)) {
          _showSetModeTime = millis() + 2000;
          _feedbackType = 0;
          _ScreenManager.reqUpdate();
        }
        else 
          _reqOEMWarning();
      }
      // release UP key to increase set demand, provided we are not in mode select
      if(event & key_Up) {
        if(reqTempDelta(+1)) {
          _showSetModeTime = millis() + 2000;
          _feedbackType = 0;
          _ScreenManager.reqUpdate();
        }
        else 
          _reqOEMWarning();
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
            setThermostatMode(1);    // set the new mode
            NVstore.save();
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
            setThermostatMode(0);    // set the new mode
            NVstore.save();
          }
          _ScreenManager.reqUpdate();
        }
      }
    }
    // release CENTRE to accept new mode, and/or show current setting
    if(event & key_Centre) {
      if(repeatCount != -2) {  // prevent after off commands
        if(_showModeTime) {
          _showModeTime = millis(); // force immediate cancellation of showmode (via screen update)
        }
        _showSetModeTime = millis() + 2000; 
        _feedbackType = 0;
      }
      _ScreenManager.reqUpdate();
    }

    repeatCount = -1;
  }
  return true;
}

void 
CBasicScreen::showRunState()
{
  int runstate = getHeaterInfo().getRunStateEx(); 
  int errstate = getHeaterInfo().getErrState(); 

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

    toPrint = getHeaterInfo().getErrStateStr();
  }
  else {
    if(runstate) {
      toPrint = getHeaterInfo().getRunStateStr();
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
