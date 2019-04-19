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
#include "HomeMenuSelScreen.h"
#include "KeyPad.h"
#include "../Protocol/helpers.h"
#include "../Utility/UtilClasses.h"
#include "../Utility/NVStorage.h"
#include "../Utility/GPIO.h"
#include "fonts/Icons.h"



CHomeMenuSelScreen::CHomeMenuSelScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
}

void 
CHomeMenuSelScreen::onSelect()
{
  CScreenHeader::onSelect();
  _rowSel = 0;
  _action = NVstore.getHomeMenu();
}

void
CHomeMenuSelScreen::_initUI()
{
}

bool 
CHomeMenuSelScreen::show()
{
  char msg[16];

  _display.clearDisplay();

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

    if(_rowSel == 4) {
      _printInverted(_display.xCentre(), 0, " Saving Settings ", true, eCentreJustify);
      _printMenuText(_display.xCentre(), 35, "Press UP to", false, eCentreJustify);
      _printMenuText(_display.xCentre(), 43, "confirm save", false, eCentreJustify);
    }
    else {
      _printInverted(_display.xCentre(), 0, " Home Menu Actions ", true, eCentreJustify);
      
      _printMenuText(66, 14, "On timeout:", false, eRightJustify);
      switch(_action.onTimeout) {
        case 0: strcpy(msg, "Default"); break;
        case 1: strcpy(msg, "Detailed"); break;
        case 2: strcpy(msg, "Basic"); break;
        case 3: strcpy(msg, "Clock"); break;
      }
      _printMenuText(70, 14, msg, _rowSel == 3);

      _printMenuText(66, 26, "On Start:", false, eRightJustify);
      switch(_action.onStart) {
        case 0: strcpy(msg, "Default"); break;
        case 1: strcpy(msg, "Detailed"); break;
        case 2: strcpy(msg, "Basic"); break;
        case 3: strcpy(msg, "Clock"); break;
      }
      _printMenuText(70, 26, msg, _rowSel == 2);

      _printMenuText(66, 38, "On Stop:", false, eRightJustify);
      switch(_action.onStop) {
        case 0: strcpy(msg, "Default"); break;
        case 1: strcpy(msg, "Detailed"); break;
        case 2: strcpy(msg, "Basic"); break;
        case 3: strcpy(msg, "Clock"); break;
      }
      _printMenuText(70, 38, msg, _rowSel == 1);

      _printMenuText(_display.xCentre(), 53, " \021     Exit     \020 ", _rowSel == 0, eCentreJustify);
    }
  }
  return true;
}


bool 
CHomeMenuSelScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // UP press
    if(event & key_Up) {
      if(_rowSel == 4) {
        _showStoringMessage();
        NVstore.setHomeMenu(_action);
        saveNV();
        _rowSel = 0;
      }
      else {
        _rowSel++;
        UPPERLIMIT(_rowSel, 3);
      }
    }
    // UP press
    if(event & key_Down) {
      _rowSel--;
      LOWERLIMIT(_rowSel, 0);
    }
    // CENTRE press
    if(event & key_Centre) {
      if(_rowSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
      }
      else {
        _rowSel = 4;
      }
    }
    // LEFT press
    if(event & key_Left) {
      if(_rowSel == 0)
        _ScreenManager.prevMenu();
      else 
        adjust(-1);
    }
    // RIGHT press
    if(event & key_Right) {
      if(_rowSel == 0)
        _ScreenManager.nextMenu();
      else 
        adjust(+1);
    }
  }

  _ScreenManager.reqUpdate();

  return true;
}

void
CHomeMenuSelScreen::adjust(int dir)
{
  switch(_rowSel) {
    case 1:
      _action.onStop += dir;
      ROLLLOWERLIMIT(_action.onStop, 0, 3);
      ROLLUPPERLIMIT(_action.onStop, 3, 0);
      break;
    case 2:
      _action.onStart += dir;
      ROLLLOWERLIMIT(_action.onStart, 0, 3);
      ROLLUPPERLIMIT(_action.onStart, 3, 0);
      break;
    case 3: 
      _action.onTimeout += dir;
      ROLLLOWERLIMIT(_action.onTimeout, 0, 3);
      ROLLUPPERLIMIT(_action.onTimeout, 3, 0);
      break;
  }
}
