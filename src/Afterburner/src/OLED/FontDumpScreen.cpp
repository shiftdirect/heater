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
#include "FontDumpScreen.h"
#include "KeyPad.h"

///////////////////////////////////////////////////////////////////////////
//
// CFontDumpScreen
//
// This screen provides control over experimental features
//
///////////////////////////////////////////////////////////////////////////

static const int Lines[4] = { 24, 34, 44, 54 };

CFontDumpScreen::CFontDumpScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreen(display, mgr) 
{
  _initUI();
}

void 
CFontDumpScreen::onSelect()
{
  CScreen::onSelect();
}

void
CFontDumpScreen::_initUI()
{
  _startChar = 0;
}

bool 
CFontDumpScreen::show()
{
  _display.clearDisplay();
  
  char msg[8];

  _printInverted(_display.xCentre(), 0, " Adafruit Font ", true, eCentreJustify);
  int column = 15;
  for(int i=0; i<16; i++) {
    sprintf(msg, "%X", i);
    _printMenuText(column, 12, msg);
    column += 7;
  }
  for(int row = 0; row < 4; row++) {
    int currentChar = row * 16 + _startChar;
    sprintf(msg, "%02X", currentChar);
    _printMenuText(0, Lines[row], msg);
    column = 15;
    for(int i=0; i<16; i++) {
      msg[0] = currentChar++;
      msg[1] = 0;
      _printMenuText(column, Lines[row], msg);
      column += 7;
    }
  }
  _display.drawFastVLine(13, 12, 61, WHITE);
  _display.drawFastHLine(0, 21, 128, WHITE);

  return true;
}


bool 
CFontDumpScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // press LEFT or UP to show prior 64 characters
    if(event & (key_Left | key_Up)) {
      _startChar -= 64;
    }
    // press RIGHT or DOWN to show next 64 characters
    if(event & (key_Right | key_Down)) {
      _startChar += 64;
    }
    // CENTRE press
    if(event & key_Centre) {
      _ScreenManager.selectMenu(CScreenManager::UserSettingsLoop, CScreenManager::ExThermostatUI);  // force return to prior menu
    }
    _ScreenManager.reqUpdate();
  }

  return true;
}

