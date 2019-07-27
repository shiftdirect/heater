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

#include "MenuTrunkScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "../Utility/macros.h"
#include "fonts/Arial.h"

///////////////////////////////////////////////////////////////////////////
//
// CMenuTrunkScreen
//
// This screen presents Bluetooth status information
//
///////////////////////////////////////////////////////////////////////////


static const int LIMIT_AWAY = 0;
static const int LIMIT_LEFT = 1;
static const int LIMIT_RIGHT = 2;

CMenuTrunkScreen::CMenuTrunkScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreen(display, mgr) 
{
  _initUI();
}

void
CMenuTrunkScreen::onSelect()
{
  CScreen::onSelect();
//  _initUI();
}

void
CMenuTrunkScreen::_initUI()
{
  _rowSel = 0;
  _colSel = 0;
}

bool 
CMenuTrunkScreen::show()
{
  _display.clearDisplay();

  CScreen::show();

  int yPos[] = { 53, 41, 29, 17 };
  
  _showTitle("Menu Trunk");

  _printMenuText(_display.xCentre(), yPos[_rowSel], " \021                \020 ", true, eCentreJustify);

  _printMenuText(_display.xCentre(), yPos[3], "Heater Tuning", false, eCentreJustify);
  _printMenuText(_display.xCentre(), yPos[2], "System Settings", false, eCentreJustify);
  _printMenuText(_display.xCentre(), yPos[1], "User Settings", false, eCentreJustify);
  _printMenuText(_display.xCentre(), yPos[0], "Root menu", false, eCentreJustify);

  return true;
}

bool 
CMenuTrunkScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      _rowSel = 0;
    }
    // press LEFT 
    if(event & key_Left) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);
          _ScreenManager.prevMenu(); 
          break;
        case 1:
          _ScreenManager.selectMenu(CScreenManager::UserSettingsLoop);
//          _ScreenManager.prevMenu(); 
          break;
        case 2:
          _ScreenManager.selectMenu(CScreenManager::SystemSettingsLoop);
//          _ScreenManager.prevMenu(); 
          break;
        case 3:
          _ScreenManager.selectMenu(CScreenManager::BranchMenu, CScreenManager::HtrSettingsUI);
//          _ScreenManager.selectMenu(CScreenManager::TuningMenuLoop);
//          _ScreenManager.prevMenu(); 
          break;
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);
          _ScreenManager.nextMenu(); 
          break;
        case 1:
          _ScreenManager.selectMenu(CScreenManager::UserSettingsLoop);
//          _ScreenManager.nextMenu(); 
          break;
        case 2:
          _ScreenManager.selectMenu(CScreenManager::SystemSettingsLoop);
//          _ScreenManager.nextMenu(); 
          break;
        case 3:
          _ScreenManager.selectMenu(CScreenManager::BranchMenu, CScreenManager::HtrSettingsUI);

//          _ScreenManager.selectMenu(CScreenManager::TuningMenuLoop);
//          _ScreenManager.nextMenu(); 
          break;
      }
    }
    // press UP
    if(event & key_Up) {
      _rowSel++;
      UPPERLIMIT(_rowSel, 3);
    }
    // press DOWN
    if(event & key_Down) {
      _rowSel--;
      LOWERLIMIT(_rowSel, 0);
    }
    _ScreenManager.reqUpdate();
  }

  return true;
}

