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
// CTimerChartScreen
//
// This screen shows the timers as a chart for the entire week
//
///////////////////////////////////////////////////////////////////////////

#include "TimerChartScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "../Utility/NVStorage.h"
#include "../../lib/RTClib/RTClib.h"
#include "fonts/MiniFont.h"
#include "../RTC/TimerManager.h"


static uint8_t condensed[7][120];

CTimerChartScreen::CTimerChartScreen(C128x64_OLED& display, CScreenManager& mgr, int instance) : CScreen(display, mgr) 
{
  _rowSel = 0;
  _colSel = 0;
  _SaveTime = 0;
  _instance = instance;
}

void 
CTimerChartScreen::onSelect()
{
  CTimerManager::createMap();
  CTimerManager::condenseMap(condensed);
}

bool 
CTimerChartScreen::show()
{
  if(CTimerManager::hasTimerChanged()) {
    CTimerManager::condenseMap(condensed);
  }
  _display.clearDisplay();

  CTransientFont AF(_display, &miniFontInfo);  // temporarily use a mini font

  _printMenuText(0, 7, "S");
  _printMenuText(0, 14, "M");
  _printMenuText(0, 21, "T");
  _printMenuText(0, 28, "W");
  _printMenuText(0, 35, "T");
  _printMenuText(0, 42, "F");
  _printMenuText(0, 49, "S");

  int hour0 = 8;
  int linespacing = 7;

  for(int tick = 0; tick < 24; tick += 3) {
    int xpos = tick * 5 + hour0;
    _display.setCursor(xpos, 0);
    _display.print(tick);
    for(int dow = 0; dow < 7; dow++) {
      int ypos = dow*linespacing + 8;
      _display.drawFastVLine(xpos, ypos, 3, WHITE);  // solid bar
    }
  }


  for(int dow = 0; dow < 7; dow++) {
    int ypos = dow*linespacing + 7;  // top of first line
    int pixel = 0;
    int subpixel = 0;
    int blockStart = -1;
    for(int interval = 0; interval < 120; interval++) {
      int IDcentre = 0;
      int ID = 0;
      if(condensed[dow][interval] & 0xf) {
        if(blockStart == -1) {
          blockStart = interval;
        }
        if((condensed[dow][interval] & 0x80) == 0) {  
          // one shot timer - draw peppered
          for(int yscan = interval & 1; yscan < 6; yscan+=2)
            _display.drawPixel(interval+hour0, ypos+yscan, WHITE);   // peppered vertical bar
        }
        else {  
          // repeating timer =- draw solid
          _display.drawFastVLine(interval+hour0, ypos, 6, WHITE);  // solid bar
        }
      }
      else {
        if(blockStart >= 0) {
          IDcentre = hour0 + (interval + blockStart) / 2;
          ID = condensed[dow][interval-1];
          blockStart = -1;
        }
        if(pixel == 0)  // every 5th pixel draw a base line
          _display.drawPixel(interval+hour0, ypos+2, subpixel ? WHITE : BLACK);   // base line
      }
      pixel++;
      if(pixel > 4) {
        pixel = 0;
        subpixel++;
        WRAPUPPERLIMIT(subpixel, 2, 0);
      }
      if((interval == 119) && blockStart >=0) {  // timer ran up until midnight
        IDcentre = hour0 + (blockStart + 120) / 2;
        ID = condensed[dow][interval];
      }
      if(IDcentre) {
        char str[8];
        sprintf(str, "%d", ID & 0xf);
        int width = 4;
        IDcentre -= 1;
        if((ID & 0xf) >= 10) {
          IDcentre -= 2;
          width = 8;
        }
        _display.fillRect(IDcentre, ypos, width, 6, WHITE);
        _display.setTextColor(BLACK, WHITE);
        _display.setCursor(IDcentre, ypos);
        _display.print(str);
        _display.setTextColor(WHITE, BLACK);
        IDcentre = 0;
      }
    }
  }

  return true;
}


bool 
CTimerChartScreen::keyHandler(uint8_t event)
{
  static bool bHeld = false;
  // handle initial key press
  if(event & keyPressed) {
    bHeld = false;
    // press CENTRE, UP or DOWN
    if(event & (key_Centre | key_Down | key_Up)) {
      _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // exit: return to clock screen
    }
    // press LEFT - navigate fields, or screens
    if(event & key_Left) {
      _ScreenManager.prevMenu(); 
    }
    // press RIGHT - navigate fields, or screens
    if(event & key_Right) {
      _ScreenManager.nextMenu(); 
    }
  }

  // handle held down keys
  if(event & keyRepeat) {
    bHeld = true;
  }

  if(event & keyReleased) {
    if(!bHeld) {
      if(event & key_Left) {
      }
      // released DOWN - can only leave adjustment by using OK (centre button)
      if(event & key_Down) {
        // adjust selected item
      }
      if(event & key_Right) {
      }
      // released UP 
      if(event & key_Up) {
      }
    }
  }

  _ScreenManager.reqUpdate();
  return true;
}


