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


///////////////////////////////////////////////////////////////////////////
//
// CSetClockScreen
//
// This screen allows the real time clock to be adjusted
//
///////////////////////////////////////////////////////////////////////////

#include "SetClockScreen.h"
#include "KeyPad.h"
#include "fonts/Arial.h"
#include "../RTC/Clock.h"
#include "../Utility/macros.h"
#include "../Utility/NVStorage.h"


CSetClockScreen::CSetClockScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreen(display, mgr) 
{
  _initUI();
}

void
CSetClockScreen::onSelect()
{
  CScreen::onSelect();
  _initUI();
}

void
CSetClockScreen::_initUI()
{
  _rowSel = 0;
  _nextT = millis();
  _SaveTime = 0;
  _12hr = NVstore.getUserSettings().clock12hr;
}

void
CSetClockScreen::showTime(int)
{
  // override and DO NOTHING!
}

bool 
CSetClockScreen::show()
{
  long deltaT = millis() - _nextT;
  if(deltaT >= 0) {
    _nextT += 1000;

    CScreen::show();
    _display.clearDisplay();

    char str[16];
    int xPos, yPos;

    _showTitle("Set Clock");

    const BTCDateTime& now = Clock.get();
    if(_rowSel == 0) {
      // update printable values
      _working = now;
      if(_12hr) {
        if(_working.hour() < 12)
          _12hr = 1;
        else 
          _12hr = 2;
      }
    }

    if(_SaveTime) {
      _showStoringMessage();
      long tDelta = millis() - _SaveTime;
      if(tDelta > 0) {
        _SaveTime = 0;
      }
    }
    else {
      yPos = 20;
      xPos = 6;
      // date
      if(_rowSel==0) {
        xPos = 18;
        _printMenuText(xPos, yPos, _working.dowStr());
        xPos = 20;
      }          

      sprintf(str, "%d", _working.day());
      xPos += 20 + 12;
      _printMenuText(xPos, yPos, str, _rowSel==1, eRightJustify);
      xPos += 4;
      _printMenuText(xPos, yPos, _working.monthStr(), _rowSel==2);
      xPos += 22;
      sprintf(str, "%d", _working.year());
      _printMenuText(xPos, yPos, str, _rowSel==3);
      // time
      yPos = 32;
      xPos = 8;
      int hr = _working.hour();
      if(_12hr) {
        if(hr == 0)
          hr = 12;
        if(hr > 12)
          hr -= 12;
      }
      sprintf(str, "%02d", hr);
      _printMenuText(xPos, yPos, str, _rowSel==4);
      xPos += 16;
      _printMenuText(xPos, yPos, ":");
      xPos += 8;
      sprintf(str, "%02d", _working.minute());
      _printMenuText(xPos, yPos, str, _rowSel==5);
      xPos += 16;
      _printMenuText(xPos, yPos, ":");
      sprintf(str, "%02d", _working.second());
      xPos += 8;
      _printMenuText(xPos, yPos, str, _rowSel==6);
      xPos += 20;
      const char* annuc = "24hr";
      switch(_12hr) {
        case 1: annuc = "AM"; break;
        case 2: annuc = "PM"; break;
      }
      _printMenuText(xPos, yPos, annuc, _rowSel == 7);
      xPos += 28;
      if(_rowSel>=1)
        _printMenuText(_display.width()-border, yPos, "SET", _rowSel==8, eRightJustify);

      // navigation line
      xPos = _display.xCentre();
      if(_rowSel == 0) {
        yPos = 53;
        _printMenuText(_display.width(), yPos, "\030Edit", false, eRightJustify);
        _printMenuText(xPos, yPos, " Exit ", true, eCentreJustify);
      }
      else {
        _display.drawFastHLine(0, 52, 128, WHITE);
        _printMenuText(xPos, 56, "\033\032 Sel         \030\031 Adj", false, eCentreJustify);
        if(_rowSel == 8) {
          _printMenuText(xPos, 56, "Save", false, eCentreJustify);
        }
        else {
          _printMenuText(xPos, 56, "Abort", false, eCentreJustify);
        }
      }
    }
  }    
  return true;
}


bool 
CSetClockScreen::keyHandler(uint8_t event)
{

  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      if(_rowSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop); // exit, return to clock screen
      }
      else {
        if(_rowSel == 8) {  // set the RTC!
          Clock.set(_working);
          _SaveTime = millis() + 1500;
        }
        // always save the 12/24hr selection on any OK
        sUserSettings us = NVstore.getUserSettings();
        us.clock12hr = _12hr ? 1 : 0;
        NVstore.setUserSettings(us);
        NVstore.save();
        _rowSel = 0;
      }
    }
    // press LEFT 
    if(event & key_Left) {
      if(_rowSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop); // exit, return to clock screen
      }
      else {
        _rowSel--;
        WRAPLOWERLIMIT(_rowSel, 1, 8);
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      if(_rowSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop); // exit, return to clock screen
      }
      else {
        _rowSel++;
        WRAPUPPERLIMIT(_rowSel, 8, 1);
      }
    }
    // press UP 
    if(event & key_Up) {
      if(_rowSel == 0) {
        _rowSel = 7;
      }
      else {
        _adjTimeDate(+1);
      }
    }
    // press DOWN
    if(event & key_Down) {
      if(_rowSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop); // exit, return to clock screen
      } else {
        _adjTimeDate(-1);
      }
    }
  }

  if(event & keyRepeat) {
    if(_rowSel >= 1) {
      // hold RIGHT 
      if(event & key_Up) {
        _adjTimeDate(+1);
      }
      // hold LEFT
      if(event & key_Down) {
        _adjTimeDate(-1);
      }
    }
  }

  _nextT = millis();
  _ScreenManager.reqUpdate();
  return true;
}

void 
CSetClockScreen::_adjTimeDate(int dir)
{
  switch(_rowSel) {
    case 1:
      _working.adjustDay(dir);
      break;
    case 2:
      _working.adjustMonth(dir);
      break;
    case 3:
      _working.adjustYear(dir);
      break;
    case 4:
      _working.adjustHour(dir);
      if(_12hr == 1 && _working.hour() >= 12)
        _12hr = 2;
      if(_12hr == 2 && _working.hour() < 12)
        _12hr = 1;
      break;
    case 5:
      _working.adjustMinute(dir);
      break;
    case 6:
      _working.adjustSecond(dir);
      break;
    case 7:
      DebugPort.printf("hr1=%d ", _working.hour());
      _12hr += dir;
      WRAPLIMITS(_12hr, 0, 2);
      if(_12hr == 1 && _working.hour() >= 12)
        _working.adjustHour12();
      if(_12hr == 2 && _working.hour() < 12)
        _working.adjustHour12();
      DebugPort.printf("hr2=%d ", _working.hour());
      DebugPort.printf("_12hr=%d\r\n", _12hr);
      break;
  }
}
