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
#include "VersionInfoScreen.h"
#include "KeyPad.h"
#include "../Protocol/helpers.h"
#include "../Utility/UtilClasses.h"
#include "../Utility/NVStorage.h"
#include "../Utility/GPIO.h"
#include "fonts/Icons.h"



CVersionInfoScreen::CVersionInfoScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
}

void 
CVersionInfoScreen::onSelect()
{
  CScreenHeader::onSelect();
}

void
CVersionInfoScreen::_initUI()
{
}

bool 
CVersionInfoScreen::show()
{
  char msg[16];

  _display.clearDisplay();

  _printInverted(_display.xCentre(), 0, " Version Information ", true, eCentreJustify);
  
  _printMenuText(0, 14, "Firmware:");
  sprintf(msg, "V%.1f", getVersion());
  _printMenuText(55, 14, msg);
  _printMenuText(55, 25, getVersionDate());

  _printMenuText(0, 38, "Hardware:");
  int PCB = getBoardRevision();
  sprintf(msg, "V%.1f", float(PCB)*0.1f);
  _printMenuText(55, 38, msg);
  if(PCB == 20) {
    _printMenuText(108, 38, "Analog", false, eCentreJustify);
    _display.drawLine(88, 42, 127, 42, WHITE);
  }

  _printMenuText(_display.xCentre(), 53, " \021              \020 ", true, eCentreJustify);
  return true;
}


bool 
CVersionInfoScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // UP press
    if(event & key_Up) {
    }
    // CENTRE press
    if(event & key_Centre) {
    }
    // LEFT press
    if(event & key_Left) {
      _ScreenManager.prevMenu();
    }
    // RIGHT press
    if(event & key_Right) {
      _ScreenManager.nextMenu();
    }
  }

  _ScreenManager.reqUpdate();

  return true;
}

