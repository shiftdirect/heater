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
#include "HeaterSettingsScreen.h"
#include "KeyPad.h"
#include "../Protocol/helpers.h"
#include "../Utility/UtilClasses.h"


///////////////////////////////////////////////////////////////////////////
//
// CHeaterSettingsScreen
//
// This screen provides a basic control function
//
///////////////////////////////////////////////////////////////////////////

CHeaterSettingsScreen::CHeaterSettingsScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _rowSel = 0;
  _fanSensor = 1;
  _glowPower = 5;
  _sysVoltage = 12;
}

bool 
CHeaterSettingsScreen::show()
{
  char msg[20];
  int xPos, yPos;
  _display.clearDisplay();

  _printInverted(0, 0, " Heater Settings ", true);

  if(!CPasswordScreen::show()) {
    yPos = 14;

    if(_rowSel == 4) {
      _printMenuText(_display.xCentre(), 35, "Press UP to", false, eCentreJustify);
      _printMenuText(_display.xCentre(), 43, "confirm save", false, eCentreJustify);
    }
    else {
      _printMenuText(98, yPos, "Glow plug power:", false, eRightJustify);
      sprintf(msg, "PF-%d", _glowPower);
      _printMenuText(100, yPos, msg, _rowSel == 3);
      yPos = 27;
      _printMenuText(98, yPos, "Fan sensor:", false, eRightJustify);
      sprintf(msg, "SN-%d", _fanSensor);
      _printMenuText(100, yPos, msg, _rowSel == 2);
      yPos = 40;
      _printMenuText(98, yPos, "System voltage:", false, eRightJustify);
      sprintf(msg, "%dV", _sysVoltage);
      _printMenuText(100, yPos, msg, _rowSel == 1);
    }
    // navigation line
    yPos = 53;
    xPos = _display.xCentre();
    _printMenuText(xPos, yPos, "<-             ->", _rowSel == 0, eCentreJustify);
  }
  return true;
}


bool 
CHeaterSettingsScreen::keyHandler(uint8_t event)
{
  if(CPasswordScreen::keyHandler(event)) {
    if(_isPasswordOK()) {
      _rowSel++;
    }
  }
  else {
    if(event & keyPressed) {
      // press LEFT to select previous screen
      if(event & key_Left) {
        switch(_rowSel) {
          case 0:
            _ScreenManager.prevScreen();
            break;
          case 1:
          case 2:
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
          case 0:
            _ScreenManager.nextScreen();
            break;
          case 1:
          case 2:
          case 3:
            _adjust(+1);
            break;
          case 4:
            _rowSel = 0;   // abort save
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
            _getPassword();  // nav line ,request password
            break;
          case 1:
          case 2:
          case 3:
            _rowSel++;
            UPPERLIMIT(_rowSel, 3);
            break;
          case 4:    // confirmed save
            _showStoringMessage();
            break;
        }
      }
      // CENTRE press
      if(event & key_Centre) {
        switch(_rowSel) {
          case 1:
          case 2:
          case 3:
            _rowSel = 4;
            break;
        }
      }
    }
  }
  return true;
}

void 
CHeaterSettingsScreen::_adjust(int dir)
{
  switch(_rowSel) {
    case 1:   // system voltage
      _sysVoltage = (_sysVoltage == 12) ? 24 : 12;
      break;
    case 2:   // fan sensor
      _fanSensor = (_fanSensor == 1) ? 2 : 1;
      break;
    case 3:   // glow power
      _glowPower += dir;
      UPPERLIMIT(_glowPower, 6);
      LOWERLIMIT(_glowPower, 1);
      break;
  }
}
