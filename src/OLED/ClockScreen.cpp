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

#include <Arduino.h>
#include "ClockScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "fonts/Tahoma24.h"
#include "fonts/Arial.h"
#include "../RTC/Clock.h"
#include "../Protocol/Protocol.h"
#include "../Utility/NVStorage.h"

///////////////////////////////////////////////////////////////////////////
//
// CClockScreen
//
// This screen presents a large format clock
//
///////////////////////////////////////////////////////////////////////////


CClockScreen::CClockScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _colon = false;
  _keyRepeatCount = -1;
}



bool 
CClockScreen::show()
{
  showHeaderDetail(true);

  CScreenHeader::show(false);
  
  const BTCDateTime& now = Clock.get();

  char str[32];
  int clockcentre = _display.xCentre();
  int hr = now.hour();
  if(NVstore.getUserSettings().clock12hr) {
    if(hr == 0)
      hr = 12;
    else if (hr > 12)
      hr -= 12;
    clockcentre -= 8; // allow space for AM/PM indicators
  }

  
  if(_colon)
    sprintf(str, "%d:%02d", hr, now.minute());
  else 
    sprintf(str, "%d %02d", hr, now.minute());
  _colon = !_colon;

  int yPos = 25;
  int timewidth = 0;
  {
    CTransientFont AF(_display, &tahoma_24ptFontInfo);  // temporarily use a large font
    _printMenuText(clockcentre, yPos, str, false, eCentreJustify);
    CRect extents;
    extents.xPos = 0;
    extents.yPos = 0;
    _display.getTextExtents(str, extents);
    timewidth = extents.width;
  }
  if(NVstore.getUserSettings().clock12hr) {
    CTransientFont AF(_display, &arial_8ptBoldFontInfo);  // temporarily use a large font
    if(now.hour() >= 12)
      _printMenuText(clockcentre + timewidth/2 + 5, yPos + 14, "PM");
    else 
      _printMenuText(clockcentre + timewidth/2 + 5, yPos, "AM");
  }
  sprintf(str, "%s %d %s %d", now.dowStr(), now.day(), now.monthStr(), now.year());
  _printMenuText(_display.xCentre(), 56, str, false, eCentreJustify);

  return true;
}


bool 
CClockScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    _keyRepeatCount = 0;     // unlock tracking of repeat events
    // press UP
    if(event & key_Up) {
      _ScreenManager.selectMenu(CScreenManager::BranchMenu, CScreenManager::SetClockUI);   // switch to clock set screen
    }
    // press DOWN
    if(event & key_Down) {
      if(!NVstore.getUserSettings().NoHeater) {
        _ScreenManager.selectMenu(CScreenManager::TimerMenuLoop);    // switch to timer set screen loop
      }
    }
  }
  if(event & keyRepeat) {
    if(_keyRepeatCount >= 0) {
      _keyRepeatCount++;
      // hold LEFT to toggle GPIO output #1
      if(event & key_Left) {
        if(_keyRepeatCount > 2) {
          _keyRepeatCount = -1;     // prevent double handling
          toggleGPIOout(0);     // toggle GPIO output #1
        }
      }
      // hold RIGHT to toggle GPIO output #2
      if(event & key_Right) {
        if(_keyRepeatCount > 2) {
          _keyRepeatCount = -1;     // prevent double handling
          toggleGPIOout(1);     // toggle GPIO output #2
        }
      }
      // hold CENTRE to toggle On/Off state
      if(event & key_Centre) {
        if(!NVstore.getUserSettings().NoHeater) {
          int runstate = getHeaterInfo().getRunStateEx();
          if(runstate) {   // running, including cyclic mode idle
            if(_keyRepeatCount > 5) {
              _keyRepeatCount = -1;
              requestOff();         
            }
          }
          else {  // standard idle state
            // standby, request ON
            if(_keyRepeatCount > 3) {
              _keyRepeatCount = -1;
              requestOn();
            }
          }
        }
      }
    }
  }
  // release event
  if(event & keyReleased) {
    if(_keyRepeatCount == 0) {  // short Up press - lower target
      if(event & key_Left) {
        _ScreenManager.prevMenu();
      }
      if(event & key_Right) {
        _ScreenManager.nextMenu();
      }
    }
    _keyRepeatCount = -1;
  }
  return true;
}

