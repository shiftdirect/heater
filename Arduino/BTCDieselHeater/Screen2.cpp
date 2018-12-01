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
#include "tahoma16.h"
#include "Icons.h"
#include "Screen2.h"
#include "KeyPad.h"
#include "helpers.h"
#include "UtilClasses.h"


#define MAXIFONT tahoma_16ptFontInfo

///////////////////////////////////////////////////////////////////////////
//
// CScreen2
//
// This screen provides a basic control function
//
///////////////////////////////////////////////////////////////////////////

CScreen2::CScreen2(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _showSetMode = 0;
  _showMode = 0;
  _nModeSel = 0;
}

void 
CScreen2::show()
{
  CScreenHeader::show();

  char msg[20];
  int xPos, yPos;

  sprintf(msg, "%.1f`", getActualTemperature());
  {
    CTransientFont AF(_display, &MAXIFONT);  // temporarily use a large font
    _drawMenuText(_display.xCentre(), 25, msg, false, eCentreJustify);
  }


  // at bottom of screen show either:
  //   Selection between Fixed or Thermostat mode
  //   Current heat demand setting
  //   Run state of heater
  
  if(_showMode) {
    const int border = 3;
    const int radius = 4;
    // Show selection between Fixed or Thermostat mode
    long tDelta = millis() - _showMode;
    if(tDelta < 0) {

      yPos = _display.height() - _display.textHeight() - border;  // bottom of screen, with room for box

      // display "Fixed Hz" at lower right, allowing space for a selection surrounding box
      strcpy(msg, "Fixed Hz");
      xPos = _display.width() - border;     // set X position to finish short of RHS
      _drawMenuText(xPos, yPos, msg, _nModeSel == 1, eRightJustify);

      // display "Thermostat" at lower left, allowing space for a selection surrounding box
      strcpy(msg, "Thermostat");
      xPos = border;
      _drawMenuText(xPos, yPos, msg, _nModeSel == 0);

      setThermostatMode(_nModeSel == 0 ? 1 : 0);    // set the new mode
    }
    else {
      // cancel selection mode, apply whatever is boxed
      _showMode = 0;
      _showSetMode = millis() + 5000;  // then make the new mode setting be shown
    }
  }
  if((_showMode == 0) && _showSetMode) {
    long tDelta = millis() - _showSetMode;  
    if(tDelta < 0) {
      // Show current heat demand setting
      if(getHeaterInfo().isThermostat()) {
        sprintf(msg, "Setpoint = %d`C", getHeaterInfo().getTemperature_Desired());
      }
      else {
        sprintf(msg, "Setpoint = %.1fHz", getHeaterInfo().getPump_Fixed());
      }
      // centre message at bottom of screen
      _drawMenuText(_display.xCentre(), _display.height() - _display.textHeight(), msg, false, eCentreJustify);
    }
    else {
      _showSetMode = 0;
    }
  }
  if((_showMode == 0) && (_showSetMode == 0)) {
    showRunState();
  }
}


void 
CScreen2::keyHandler(uint8_t event)
{
  static int repeatCount = -1;

  if(event & keyPressed) {
    repeatCount = 0;     // unlock tracking of repeat events
    // press LEFT to select previous screen, or Fixed Hz mode when in mode select
    if(event & key_Left) {
      if(!_showMode)
        _ScreenManager.prevScreen();
      else {
        _showMode = millis() + 5000;
        _nModeSel = 0;
        _ScreenManager.reqUpdate();
      }
    }
    // press RIGHT to selecxt next screen, or Thermostat mode when in mode select
    if(event & key_Right) {
      if(!_showMode)
        _ScreenManager.nextScreen();
      else {
        _showMode = millis() + 5000;
        _nModeSel = 1;
        _ScreenManager.reqUpdate();
      }
    }
    // press UP & DOWN to toggle thermostat / fixed Hz mode
    // impossible with 5 way switch!
    uint8_t doubleKey = key_Down | key_Up;
    if((event & doubleKey) == doubleKey) {
      reqThermoToggle();
      _showSetMode = millis() + 2000;
    }
  }
  // use repeat function for key hold detection
  if(event & keyRepeat) {
    if(repeatCount >= 0) {
      repeatCount++;
      // hold DOWN to enter thermostat / fixed mode selection
      if(event & key_Down) {
        if(repeatCount > 2) {
          repeatCount = -1;        // prevent double handling
          _showMode = millis() + 5000;
          _nModeSel = getHeaterInfo().isThermostat() ? 0 : 1;
        }
      }
      // hold CENTRE to turn ON or OFF
      if(event & key_Centre) {
        if(getHeaterInfo().getRunState()) {
          // running, request OFF
          if(repeatCount > 5) {
            repeatCount = -2;        // prevent double handling
            requestOff();
          }
        }
        else {
          // standby, request ON
          if(repeatCount > 3) {
            repeatCount = -1;
            requestOn();
          }
        }
      }
    }
  }
  if(event & keyReleased) {
    if(!_showMode) {
      // release DOWN key to reduce set demand, provided we are not in mode select
      if(event & key_Down) {
        reqTempChange(-1);
        _showSetMode = millis() + 2000;
      }
      // release UP key to increase set demand, provided we are not in mode select
      if(event & key_Up) {
        reqTempChange(+1);
        _showSetMode = millis() + 2000;
      }
    }
    // release CENTRE to accept new mode, and/or show current setting
    if(event & key_Centre) {
      if(repeatCount != -2) {  // prevent after off commands
        if(_showMode) {
          _showMode = millis(); // force immediate cancellation of showmode (via screen update)
        }
        _showSetMode = millis() + 2000;
      }
      _ScreenManager.reqUpdate();
    }

    repeatCount = -1;
  }
}

void 
CScreen2::showRunState()
{
  int runstate = getHeaterInfo().getRunState(); 
  int errstate = getHeaterInfo().getErrState(); 

  if(errstate) errstate--;  // correct for +1 biased return value

  static bool toggle = false;
  const char* toPrint = NULL;
  _display.setTextColor(WHITE, BLACK);
  if(runstate >= 0 && runstate <= 8) {
    if(((runstate == 0) || (runstate > 5)) && errstate) {

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
      _drawMenuText(xPos, yPos, msg, false, eCentreJustify);

      toPrint = getHeaterInfo().getErrStateStr();
    }
    else {
      if(runstate) {
        if(runstate < 5)        toPrint = "Starting";
        else if(runstate == 5)  toPrint = "Running";
        else if(runstate == 8)  toPrint = "Cooling";
        else                    toPrint = "Shutting down";
      }
    }
  }
  if(toPrint) {
    // locate at bottom centre
    _drawMenuText(_display.xCentre(), _display.height() - _display.textHeight(), toPrint, false, eCentreJustify);
  }
}
