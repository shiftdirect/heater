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

CHeaterSettingsScreen::CHeaterSettingsScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _rowSel = 0;
  _SaveTime = 0;
  _fanSensor = 1;
  _glowPower = 5;
  _sysVoltage = 12;
}

void 
CHeaterSettingsScreen::show()
{
  char msg[20];
  int xPos, yPos;
  _display.clearDisplay();

  _printInverted(0, 0, " Heater Settings ", true);

  if(_SaveTime) {
    long tDelta = millis() - _SaveTime;
    if(tDelta > 0) 
      _SaveTime = 0;
    _printInverted(_display.xCentre(), 28, "         ", true, eCentreJustify);
    _printInverted(_display.xCentre(), 39, "         ", true, eCentreJustify);
    _printInverted(_display.xCentre(), 34, " STORING ", true, eCentreJustify);
  }
  else {
    yPos = 14;
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
    // navigation line
    yPos = 53;
    xPos = _display.xCentre();
    _printMenuText(xPos, yPos, "<-             ->", _rowSel == 0, eCentreJustify);
  }
}


void 
CHeaterSettingsScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // press LEFT to select previous screen, or Fixed Hz mode when in mode select
    if(event & key_Left) {
      if(_rowSel == 0)
        _ScreenManager.prevScreen();
      else {
        _adjust(-1);
      }
    }
    // press RIGHT to selecxt next screen, or Thermostat mode when in mode select
    if(event & key_Right) {
      if(_rowSel == 0)
        _ScreenManager.nextScreen();
      else {
        _adjust(+1);
      }
    }
    if(event & key_Down) {
      _rowSel--;
      LOWERLIMIT(_rowSel, 1);
    }
    if(event & key_Up) {
      _rowSel++;
      UPPERLIMIT(_rowSel, 3);
    }
    if(event & key_Centre) {
      if(_rowSel) {
        _SaveTime = millis() + 1500;
        _ScreenManager.reqUpdate();
        _rowSel = 0;
      }
    }
  }
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
