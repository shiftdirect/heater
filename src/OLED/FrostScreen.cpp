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
#include "FrostScreen.h"
#include "KeyPad.h"
#include "fonts/Icons.h"



CFrostScreen::CFrostScreen(C128x64_OLED& display, CScreenManager& mgr) : CUIEditScreen(display, mgr) 
{
}

bool 
CFrostScreen::onSelect()
{
  CUIEditScreen::onSelect();
  _frostOn = NVstore.getUserSettings().FrostOn;
  _frostRise = NVstore.getUserSettings().FrostRise;
  _scrollChar = 0;
  return true;
}

bool 
CFrostScreen::show()
{
  char msg[16];

  _display.clearDisplay();

  if(CUIEditScreen::show())   // for showing "saving settings"
    return true;

  _showTitle("Frost Mode");
  
  _drawBitmap(25, 20, frostIconInfo);
  _drawBitmap(45, 16, StartIconInfo);
  if(_frostOn == 0) {
    strcpy(msg, "Disabled"); 
  }
  else {
    sprintf(msg, "< %d`C", _frostOn); 
  }
  _printMenuText(57, 16, msg, _rowSel == 2);

  if(_frostOn) {
    _drawBitmap(45, 30, StopIconInfo);
    if(_frostRise) {
      sprintf(msg, "> %d`C", _frostOn+_frostRise); 
    }
    else {
      strcpy(msg, "User stop");
    }
    _printMenuText(57, 30, msg, _rowSel == 1);
  }
  return true;
}

bool 
CFrostScreen::animate()
{
  if(_saveBusy()) {
    return false;
  }

  const char* pMsg = NULL;
  switch(_rowSel) {
    case 0:
      _printMenuText(_display.xCentre(), 53, " \021  \030Edit  Exit   \020 ", true, eCentreJustify);
      break;
    case 2:
      pMsg = "                    Define auto start temperature for frost mode.                    "; 
      break;
    case 1:
      pMsg = "                    Define auto stop temperature for frost mode.                    ";
      break;
  }
  if(pMsg != NULL) {
    _display.drawFastHLine(0, 52, 128, WHITE);
    _scrollMessage(56, pMsg, _scrollChar);
  }
  return true;
}


bool 
CFrostScreen::keyHandler(uint8_t event)
{
  if(CUIEditScreen::keyHandler(event)) {   // handles save confirm
    return true;
  }

  if(event & keyPressed) {
    // UP press
    if(event & key_Up) {
      _scrollChar = 0;
      if(_rowSel == 0) {
        _rowSel = 1;
        if(_frostOn == 0)
          _rowSel = 2;
      }
      else {
        _rowSel++;
        UPPERLIMIT(_rowSel, 2);
      }
    }
    // DOWN press
    if(event & key_Down) {
      _scrollChar = 0;
      _rowSel--;
      if(_frostOn == 0)
        _rowSel--;
      LOWERLIMIT(_rowSel, 0);
    }
    // CENTRE press
    if(event & key_Centre) {
      if(_rowSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
      }
      else {
        _confirmSave();
        _rowSel = 0;
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
CFrostScreen::adjust(int dir)
{
  switch(_rowSel) {
    case 1:
      _frostRise += dir;
      BOUNDSLIMIT(_frostRise, 0, 30);
      break;
    case 2:
      _frostOn += dir;
      BOUNDSLIMIT(_frostOn, 0, 30);
      break;
  }
}

void
CFrostScreen::_saveNV()
{
  sUserSettings us = NVstore.getUserSettings();
  us.FrostOn = _frostOn;
  us.FrostRise = _frostRise;
  NVstore.setUserSettings(us);
  NVstore.save();
}

