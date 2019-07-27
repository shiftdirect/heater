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


///////////////////////////////////////////////////////////////////////////
//
// CSettingsScreen
//
// This screen allows the fuel mixture endpoints to be adjusted
//
///////////////////////////////////////////////////////////////////////////

#include "InheritSettingsScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "../Protocol/Protocol.h"


CInheritSettingsScreen::CInheritSettingsScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _initUI();
}

void 
CInheritSettingsScreen::onSelect()
{
  // ensure standard entry to screen - especially after a dimming timeout
  CPasswordScreen::onSelect();
  _nAdoptSettings = hasOEMLCDcontroller() ? 1 : 2;
}

void 
CInheritSettingsScreen::_initUI()
{
  // ensure standard entry to screen - especially after a dimming timeout
  _nAdoptSettings = 0;
}

bool 
CInheritSettingsScreen::show()
{
  CScreenHeader::show(false);

  _display.writeFillRect(0, 16, 96, 12, WHITE);
  _printInverted(3, 18, "Inherit Settings", true);

  if(!CPasswordScreen::show()) {

    switch(_nAdoptSettings) {
      case 0:
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
        _ScreenManager.reqUpdate();
        return false;
      case 1:
        _display.clearDisplay();
        _display.writeFillRect(0, 0, 128, 24, WHITE);
        _printInverted(_display.xCentre(),  4, "ADOPT LCD controller", true, eCentreJustify);
        _printInverted(_display.xCentre(), 13, "settings?           ", true, eCentreJustify);
        _printMenuText(_display.xCentre(), 35, "Press RIGHT to", false, eCentreJustify);
        _printMenuText(_display.xCentre(), 45, "inherit and save", false, eCentreJustify);
        break;
      case 2:
        _display.clearDisplay();
        _display.writeFillRect(0, 0, 128, 24, WHITE);
        _printInverted(_display.xCentre(), 4, " CANNOT inherit knob ", true, eCentreJustify);
        _printInverted(_display.xCentre(), 13, " controller settings ", true, eCentreJustify);
        _printMenuText(_display.xCentre(), 35, "Press any key", false, eCentreJustify);
        _printMenuText(_display.xCentre(), 45, "to abort", false, eCentreJustify);
        break;
    }

  }
  
  return true;
}


bool 
CInheritSettingsScreen::keyHandler(uint8_t event)
{
  if(CPasswordScreen::keyHandler(event)) {
    if(_isPasswordOK()) {
      setPumpMin(getHeaterInfo().getPump_Min());
      setPumpMax(getHeaterInfo().getPump_Max());
      setFanMin(getHeaterInfo().getFan_Min());
      setFanMax(getHeaterInfo().getFan_Max());
      setFanSensor(getHeaterInfo().getFan_Sensor());
      setSystemVoltage(getHeaterInfo().getSystemVoltage());
      saveNV();
      _enableStoringMessage();
      _nAdoptSettings = 0;  // will cause return to main menu after storing message expires
    }
  }

  else {
    if(event & keyPressed) {
      // press RIGHT 
      if(event & key_Right) {
        if(hasOEMLCDcontroller()) {  // inheritance only valid for LCD controllers
          _getPassword();
          _ScreenManager.reqUpdate();
          return true;
        }
      }
      _nAdoptSettings = 0;  // will cause return to main menu 
    }
  }
  _ScreenManager.reqUpdate();
  return true;
}

