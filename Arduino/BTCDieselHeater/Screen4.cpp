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
#include "KeyPad.h"
#include "helpers.h"
#include "Screen4.h"
#include "BTCWifi.h"

///////////////////////////////////////////////////////////////////////////
//
// CScreen4
//
// This screen presents sundry information
// eg WiFi status
//
///////////////////////////////////////////////////////////////////////////


CScreen4::CScreen4(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
}


void 
CScreen4::show()
{
  CScreenHeader::show();
  
  CRect extents;

  _display.setCursor(0, 24);
  _display.print("IP addr:");
  _display.setCursor(_display.width(), 24);
  if(isWifiConnected()) {
    _display.printRightJustified(getWifiAddrStr());
  }
  else {
    _display.printRightJustified("Not active");
  }

//  _display.display();
}


void 
CScreen4::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      return;
    }
    // press LEFT 
    if(event & key_Left) {
      _ScreenManager.prevScreen(); 
    }
    // press RIGHT 
    if(event & key_Right) {
      _ScreenManager.nextScreen(); 
    }
  }
}

