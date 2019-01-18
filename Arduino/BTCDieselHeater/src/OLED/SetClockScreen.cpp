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
// CSetClockScreen
//
// This screen allows the real time clock to be adjusted
//
///////////////////////////////////////////////////////////////////////////

#include "SetClockScreen.h"
#include "KeyPad.h"
#include "../Protocol/helpers.h"
#include "fonts/Arial.h"
#include "../RTC/Clock.h"


CSetClockScreen::CSetClockScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _rowSel = 0;
  _nextT = millis();
  _SaveTime = 0;
}

void
CSetClockScreen::showTime(int)
{
  // override and DO NOTHING!
}

void 
CSetClockScreen::show()
{
  long deltaT = millis() - _nextT;
  if(deltaT >= 0) {
    _nextT += 1000;

    CScreenHeader::show();

    char str[16];
    int xPos, yPos;
    const int col2 = 90;
    const int col3 = _display.width() - border;

    _printInverted(0, 16, " Set Clock ", true);

    const BTCDateTime& now = Clock.get();
    if(_rowSel == 0) {
      // update printable values
      working = now;
      // DELIBERATE DROP THROUGH HERE
    }

    if(_SaveTime) {
      long tDelta = millis() - _SaveTime;
      if(tDelta > 0) 
        _SaveTime = 0;
      _printInverted(_display.xCentre(), 28, "         ", true, eCentreJustify);
      _printInverted(_display.xCentre(), 39, "         ", true, eCentreJustify);
      _printInverted(_display.xCentre(), 34, " STORING ", true, eCentreJustify);
    }
    else {
      yPos = 28;
      xPos = 6;
      // date
      if(_rowSel==0) {
        xPos = 20;
        _printMenuText(xPos, yPos, working.dowStr());
      }          

      sprintf(str, "%d", working.day());
      xPos += 20 + 12;
      _printMenuText(xPos, yPos, str, _rowSel==1, eRightJustify);
      xPos += 4;
      _printMenuText(xPos, yPos, working.monthStr(), _rowSel==2);
      xPos += 22;
      sprintf(str, "%d", working.year());
      _printMenuText(xPos, yPos, str, _rowSel==3);
      // time
      yPos = 40;
      xPos = 26;
      sprintf(str, "%02d", working.hour());
      _printMenuText(xPos, yPos, str, _rowSel==4);
      xPos += 16;
      _printMenuText(xPos, yPos, ":");
      xPos += 8;
      sprintf(str, "%02d", working.minute());
      _printMenuText(xPos, yPos, str, _rowSel==5);
      xPos += 16;
      _printMenuText(xPos, yPos, ":");
      sprintf(str, "%02d", working.second());
      xPos += 8;
      _printMenuText(xPos, yPos, str, _rowSel==6);
      if(_rowSel>=1)
        _printMenuText(_display.width()-border, yPos, "SET", _rowSel==7, eRightJustify);
    }
    // navigation line
    yPos = 53;
    xPos = _display.xCentre();
    _printMenuText(xPos, yPos, " return ", _rowSel==0, eCentreJustify);

  }    
}


void 
CSetClockScreen::keyHandler(uint8_t event)
{

  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      if(_rowSel == 0) {
        _ScreenManager.selectSetTimeScreen(false);
      }
      else {
        if(_rowSel == 7) {  // set the RTC!
          Clock.set(working);
          _SaveTime = millis() + 1500;
        }
        _rowSel = 0;
      }
      return;
    }
    // press LEFT 
    if(event & key_Left) {
      if(_rowSel == 0) {
        _ScreenManager.selectSetTimeScreen(false);
      }
      else {
        adjTimeDate(-1);
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      if(_rowSel == 0) {
        _ScreenManager.selectSetTimeScreen(false);
      }
      else {
        adjTimeDate(+1);
      }
    }
    // press UP 
    if(event & key_Up) {
      _rowSel++;
      ROLLUPPERLIMIT(_rowSel, 7, 1);
    }
    // press DOWN
    if(event & key_Down) {
      if(_rowSel == 0) {
        _ScreenManager.selectSetTimeScreen(false);  // exit: return to clock screen
      } else {
        _rowSel--;
        ROLLLOWERLIMIT(_rowSel, 1, 7);
      }
    }
  }
  if(event & keyRepeat) {
    if(_rowSel>=1) {
      // hold RIGHT 
      if(event & key_Right) {
        adjTimeDate(+1);
      }
      // hold LEFT
      if(event & key_Left) {
        adjTimeDate(-1);
      }
    }
  }

  _nextT = millis();
  _ScreenManager.reqUpdate();
}

void 
CSetClockScreen::adjTimeDate(int dir)
{
  int days;
  switch(_rowSel) {
    case 1:
      working.adjustDay(dir);
      break;
    case 2:
      working.adjustMonth(dir);
      break;
    case 3:
      working.adjustYear(dir);
      break;
    case 4:
      working.adjustHour(dir);
      break;
    case 5:
      working.adjustMinute(dir);
      break;
    case 6:
      working.adjustSecond(dir);
      break;
  }
}
