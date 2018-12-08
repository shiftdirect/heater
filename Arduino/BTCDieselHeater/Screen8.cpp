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
#include "Screen8.h"
#include "Tahoma16.h"
#include "Clock.h"

///////////////////////////////////////////////////////////////////////////
//
// CScreen8
//
// This screen presents a large format clock
//
///////////////////////////////////////////////////////////////////////////


CScreen8::CScreen8(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _colon = false;
}

void
CScreen8::showTime(int)
{
  // override and DO NOTHING!
}


void 
CScreen8::show()
{
  CScreenHeader::show();
  
  const BTCDateTime& now = Clock.get();

  char str[32];
//  if(now.second() & 0x01)
  if(_colon)
    sprintf(str, "%d:%02d", now.hour(), now.minute());
  else 
    sprintf(str, "%d %02d", now.hour(), now.minute());
  _colon = !_colon;

  int yPos = 25;
  {
    CTransientFont AF(_display, &tahoma_16ptFontInfo);  // temporarily use a large font
    _printMenuText(_display.xCentre(), yPos, str, false, eCentreJustify);
  }
  sprintf(str, "%s %d %s %d", now.dowStr(), now.day(), now.monthStr(), now.year());
  _printMenuText(_display.xCentre(), 56, str, false, eCentreJustify);
}


void 
CScreen8::keyHandler(uint8_t event)
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

