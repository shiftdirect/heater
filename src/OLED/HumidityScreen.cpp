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
#include "HumidityScreen.h"
#include "KeyPad.h"
#include "fonts/Icons.h"



CHumidityScreen::CHumidityScreen(C128x64_OLED& display, CScreenManager& mgr) : CUIEditScreen(display, mgr) 
{
}

void 
CHumidityScreen::onSelect()
{
  CUIEditScreen::onSelect();
  _humidityThresh = NVstore.getUserSettings().humidityStart;
  _scrollChar = 0;
}

bool 
CHumidityScreen::show()
{
  char msg[16];

  _display.clearDisplay();

  if(CUIEditScreen::show())   // for showing "saving settings"
    return true;

  _showTitle("Humidity Mode");
  
  _drawBitmap(25, 20, humidityIconInfo);
  _drawBitmap(50, 21, StartIconInfo);
  if(_humidityThresh == 0) {
    strcpy(msg, "Disabled"); 
  }
  else {
    sprintf(msg, "> %d%%", _humidityThresh); 
  }
  _printMenuText(62, 22, msg, _rowSel == 2);

 
  return true;
}

bool 
CHumidityScreen::animate()
{
  if(_saveBusy()) {
    return false;
  }

  const char* pMsg = NULL;
  switch(_rowSel) {
    case 0:
      _printMenuText(_display.xCentre(), 53, " \021  \030Edit  Exit   \020 ", true, eCentreJustify);
      break;
    case 1:
      pMsg = "                    Define humidity threshold to start heater automatically.                    "; 
      break;
  }
  if(pMsg != NULL) {
    _display.drawFastHLine(0, 52, 128, WHITE);
    _scrollMessage(56, pMsg, _scrollChar);
  }
  return true;
}


bool 
CHumidityScreen::keyHandler(uint8_t event)
{
  if(CUIEditScreen::keyHandler(event)) {   // handles save confirm
    return true;
  }

  if(event & keyRepeat) {
    // LEFT hold
    if(event & key_Left) {
      if(_rowSel == 0)
        _ScreenManager.prevMenu();
      else 
        adjust(-1);
    }
    // RIGHT hold
    if(event & key_Right) {
      if(_rowSel == 0)
        _ScreenManager.nextMenu();
      else 
        adjust(+1);
    }
  }
  if(event & keyPressed) {
    // UP press
    if(event & key_Up) {
      _scrollChar = 0;
      _rowSel++;
      UPPERLIMIT(_rowSel, 1);
    }
    // DOWN press
    if(event & key_Down) {
      _scrollChar = 0;
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
CHumidityScreen::adjust(int dir)
{
  switch(_rowSel) {
    case 1:
      if(dir > 0) {
        _humidityThresh++;
        BOUNDSLIMIT(_humidityThresh, 50, 100);
      }
      else {
        if(_humidityThresh) {
          if(_humidityThresh == 50)
            _humidityThresh = 0;
          else 
            _humidityThresh--;
        }
      }
      break;
  }
}

void
CHumidityScreen::_saveNV()
{
  sUserSettings us = NVstore.getUserSettings();
  us.humidityStart = _humidityThresh;
  NVstore.setUserSettings(us);
  NVstore.save();
}

