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
// CScreen5
//
// This screen allows the fuel mixture endpoints to be adjusted
//
///////////////////////////////////////////////////////////////////////////

#include "128x64OLED.h"
#include "KeyPad.h"
#include "helpers.h"
#include "Screen6.h"
#include "RTClib.h"
#include "Arial.h"
#include "BTCDateTime.h"

extern RTC_DS3231 rtc;


CScreen6::CScreen6(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _rowSel = 0;
  _colSel = 0;
  _nextT = millis();
}

void
CScreen6::showTime(int)
{
  // override and DO NOTHING!
}

void 
CScreen6::show()
{
  long deltaT = millis() - _nextT;
  if(deltaT >= 0) {
    _nextT += 1000;

    CScreenHeader::show();

    char str[16];
    int xPos, yPos;
    const int col2 = 90;
    const int col3 = _display.width() - border;

    _printInverted(0, 16, " Clock ", true);

    const BTCDateTime& now = getCurrentTime();
    switch(_rowSel) {
      case 0:
        // update printable values
        working = now;
        // DELIBERATE DROP THROUGH HERE
      case 1:
        yPos = 28;
        xPos = 6;
        // date
        if(_rowSel==0) {
          xPos = 20;
          _printMenuText(xPos, yPos, working.dowStr());
        }          

        sprintf(str, "%d", working.day());
        xPos += 20 + 12;
        _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==0, eRightJustify);
        xPos += 4;
        _printMenuText(xPos, yPos, working.monthStr(), _rowSel==1 && _colSel==1);
        xPos += 22;
        sprintf(str, "%d", working.year());
        _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==2);
        // time
        yPos = 40;
        xPos = 26;
        sprintf(str, "%02d", working.hour());
        _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==3);
        xPos += 16;
        _printMenuText(xPos, yPos, ":");
        xPos += 8;
        sprintf(str, "%02d", working.minute());
        _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==4);
        xPos += 16;
        _printMenuText(xPos, yPos, ":");
        sprintf(str, "%02d", working.second());
        xPos += 8;
        _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==5);
        if(_rowSel==1)
          _printMenuText(_display.width()-border, yPos, "SET", _colSel==6, eRightJustify);
        // navigation line
        yPos = 53;
        xPos = _display.xCentre();
        _printMenuText(xPos, yPos, "<-             ->", _rowSel==0, eCentreJustify);
        break;

    }
  }    
}


void 
CScreen6::keyHandler(uint8_t event)
{

  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      switch(_rowSel) {
        case 1:
          _rowSel = 0;
          if(_colSel == 6) {  // set the RTC!
            rtc.adjust(working);
          }
          break;
      }
      return;
    }
    // press LEFT 
    if(event & key_Left) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.prevScreen(); 
          break;
        case 1:
          _colSel--;
          ROLLLOWERLIMIT(_colSel, 0, 6);
          break;
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.nextScreen(); 
          break;
        case 1:
          _colSel++;
          ROLLUPPERLIMIT(_colSel, 6, 0);
          break;
      }
    }
    // press UP 
    if(event & key_Up) {
      switch(_rowSel) {
        case 0:
          _rowSel = 1;
          _colSel = 0;
          break;
        case 1:
          adjTimeDate(+1);
          break;
      }
    }
    // press DOWN
    if(event & key_Down) {
      switch(_rowSel) {
        case 1:
          adjTimeDate(-1);
          break;
      }
    }
  }
  if(event & keyRepeat) {
    if(_rowSel==1) {
      // press UP 
      if(event & key_Up) {
        adjTimeDate(+1);
      }
      // press DOWN
      if(event & key_Down) {
        adjTimeDate(-1);
      }
    }
  }

  _nextT = millis();
  _ScreenManager.reqUpdate();
}

void 
CScreen6::adjTimeDate(int dir)
{
  int days;
  switch(_colSel) {
    case 0:
      working.adjustDay(dir);
      break;
    case 1:
      working.adjustMonth(dir);
      break;
    case 2:
      working.adjustYear(dir);
      break;
    case 3:
      working.adjustHour(dir);
      break;
    case 4:
      working.adjustMinute(dir);
      break;
    case 5:
      working.adjustSecond(dir);
      break;
  }
}
