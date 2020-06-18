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
// CPasswordScreen
//
// This class allows a password entry page to pop up
//
///////////////////////////////////////////////////////////////////////////

#include "UIEditScreen.h"
#include "KeyPad.h"
#include "../Utility/macros.h"
#include "../Utility/NVStorage.h"
#include "fonts/Arial.h"

CUIEditScreen::CUIEditScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreen(display, mgr) 
{
  _SaveTime = 0;
  _bReqSave = false;
  _initUI();
}

bool
CUIEditScreen::onSelect()
{
  CScreen::onSelect();
  _initUI();
  return true;
}

void
CUIEditScreen::_initUI()
{
  _rowSel = 0;
  _colSel = 0;
  _animateCount = 0;
}

bool 
CUIEditScreen::show()
{
  CScreen::animate();  // precautionary, in case derived class forgets to call
  
  __expireSave();
  if(_SaveTime) {
    _display.clearDisplay();
    _showStoringMessage();
    return true;
  }
  else if(_bReqSave) {
    _showConfirmMessage();
    return true;
  }

  return false;
}

bool 
CUIEditScreen::animate()
{
  return __expireSave();
}

bool 
CUIEditScreen::__expireSave()
{
  if(_SaveTime) {
    long tDelta = millis() - _SaveTime;
    if(tDelta > 0) {
      _SaveTime = 0;
      _display.clearDisplay();
      _ScreenManager.reqUpdate();
    }
    return true;
  }
  return false;
}

bool
CUIEditScreen::_saveBusy()
{
  __expireSave();
  return _SaveTime != 0 || _bReqSave;
}

bool 
CUIEditScreen::keyHandler(uint8_t event)
{
  if(_bReqSave) {
    if(event & keyPressed) {
      _bReqSave = false;
      if(event & key_Up) {
        _enableStoringMessage();
        _saveNV();
      }
      onSelect();
      return true;
    }
  }

  return false;
}

void 
CUIEditScreen::_enableStoringMessage()
{
  _SaveTime = millis() + 1500;
  _ScreenManager.reqUpdate();
}


void
CUIEditScreen::_confirmSave()
{
  _bReqSave = true;
}