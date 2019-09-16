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

#include "128x64OLED.h"
#include "MenuSelScreen.h"
#include "KeyPad.h"
#include "fonts/Icons.h"



CMenuSelScreen::CMenuSelScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
}

void 
CMenuSelScreen::onSelect()
{
  CScreenHeader::onSelect();
  _rowSel = 0;
  _menuMode = NVstore.getUserSettings().menuMode;
  _scrollChar = 0;
  _bReload = false;
}

void
CMenuSelScreen::_initUI()
{
}

bool 
CMenuSelScreen::show()
{
  char msg[16];

  _display.clearDisplay();

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

    if(_bReload) {
      _ScreenManager.reqReload();
      return false;
    }

    if(_rowSel == SaveConfirm) {
      _showConfirmMessage();
    }
    else {
      _showTitle("Menu Mode");
      
      _drawBitmap(30, 16, MenuIconInfo);
      switch(_menuMode) {
        case 0: strcpy(msg, "Standard"); break;
        case 1: strcpy(msg, "Basic"); break;
        case 2: strcpy(msg, "No heater"); break;
      }
      _printMenuText(50, 16, msg, _rowSel == 1);
    }
  }
  return true;
}

bool 
CMenuSelScreen::animate()
{
  if(!CPasswordScreen::_busy() && !CPasswordScreen::isPasswordBusy()) {
    if(_bReload) {
      _ScreenManager.reqReload();
      return false;
    }
    if(_rowSel != SaveConfirm) {
      const char* pMsg = NULL;
      switch(_menuMode) {
        case 0:
          pMsg = "                    Standard, complete menu allows full access to features.                    ";
          break;
        case 1:
          pMsg = "                    Basic simplified menu.                    ";
          break;
        case 2:
          pMsg = "                    No heater mode.                    ";
          break;
      }
      _display.drawFastHLine(0, 52, 128, WHITE);
      if(pMsg != NULL)
        _scrollMessage(56, pMsg, _scrollChar);
      return true;
    }
  }
  return false;
}


bool 
CMenuSelScreen::keyHandler(uint8_t event)
{
  if(CPasswordScreen::keyHandler(event)) {
    if(_isPasswordOK()) {
      _rowSel = 1;
    }
  }

  else {
    _scrollChar = 0;
    sUserSettings us;
    if(event & keyPressed) {
      // UP press
      if(event & key_Up) {
        if(_rowSel == SaveConfirm) {
          _enableStoringMessage();
          us = NVstore.getUserSettings();
          us.menuMode = _menuMode;
          NVstore.setUserSettings(us);
          NVstore.save();
          switch(us.menuMode) {
            case 0: DebugPort.println("Invoking Full menu control mode"); break;
            case 1: DebugPort.println("Invoking Basic menu mode"); break;
            case 2: DebugPort.println("Invoking No Heater menu mode"); break;
          }
          _bReload = true;
          _rowSel = 0;
        }
        else {
          _getPassword();
        }
      }
      // DOWN press
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
          _rowSel = SaveConfirm;
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
  }

  return true;
}

void
CMenuSelScreen::adjust(int dir)
{
  switch(_rowSel) {
    case 1:
      _menuMode += dir;
      WRAPLIMITS(_menuMode, 0, 2);
      break;
  }
}



