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

#include "BTScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "../Bluetooth/BluetoothAbstract.h"
#include "../Utility/NVStorage.h"
#include "fonts/Arial.h"

///////////////////////////////////////////////////////////////////////////
//
// CBTScreen
//
// This screen presents Bluetooth status information
//
///////////////////////////////////////////////////////////////////////////


static const int LIMIT_AWAY = 0;
static const int LIMIT_LEFT = 1;
static const int LIMIT_RIGHT = 2;

CBTScreen::CBTScreen(C128x64_OLED& display, CScreenManager& mgr) : CUIEditScreen(display, mgr) 
{
}

bool 
CBTScreen::show()
{
  _showTitle("Bluetooth info");

  int yPos = 35;
  _printMenuText(0, yPos, "MAC:");
  _printMenuText(25, yPos, getBluetoothClient().getMAC());

  _printMenuText(_display.xCentre(), 53, " \021                \020 ", true, eCentreJustify);
  _printMenuText(_display.xCentre(), 53, "Exit", false, eCentreJustify);
  return true;
}

bool 
CBTScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
    }
    // press LEFT 
    if(event & key_Left) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.prevMenu(); 
          break;
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.nextMenu(); 
          break;
      }
    }
    // press UP
    if(event & key_Up) {
    }
    // press DOWN
    if(event & key_Down) {
    }
    _ScreenManager.reqUpdate();
  }

  return true;
}

