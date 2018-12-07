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

extern RTC_DS3231 rtc;

static char daysOfTheWeek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static char monthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
void readClock(DateTime& now);
int daysInMonth(int month, int year);

CScreen6::CScreen6(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _rowSel = 0;
  _colSel = 0;
  _nextT = millis();
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

    DateTime now;
    switch(_rowSel) {
      case 0:
        // update printable values
        readClock(now);
        _month = now.month();
        _year = now.year();
        _day = now.day();
        _hour = now.hour();
        _min = now.minute();
        _sec = now.second();
        // DELIBERATE DROP THROUGH HERE
      case 1:
        yPos = 28;
        xPos = 6;
        // date
        if(_rowSel==0) {
          xPos = 20;
          _printMenuText(xPos, yPos, daysOfTheWeek[now.dayOfTheWeek()]);
        }          

        sprintf(str, "%d", _day);
        xPos += 20 + 12;
        _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==0, eRightJustify);
        xPos += 4;
        _printMenuText(xPos, yPos, months[_month-1], _rowSel==1 && _colSel==1);
        xPos += 22;
        sprintf(str, "%d", _year);
        _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==2);
        // time
        yPos = 40;
        xPos = 26;
        sprintf(str, "%02d", _hour);
        _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==3);
        xPos += 16;
        _printMenuText(xPos, yPos, ":");
        xPos += 8;
        sprintf(str, "%02d", _min);
        _printMenuText(xPos, yPos, str, _rowSel==1 && _colSel==4);
        xPos += 16;
        _printMenuText(xPos, yPos, ":");
        sprintf(str, "%02d", _sec);
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
            rtc.adjust(DateTime(_year, _month, _day, _hour, _min, _sec));
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

  _nextT = millis();
  _ScreenManager.reqUpdate();
}

void readClock(DateTime& now)
{
    now = rtc.now();
}   

int daysInMonth(int month, int year)
{
  if(month >= 1 && month <= 12) {
    int days = monthDays[month-1];
    if((month == 2) && ((year & 0x03) == 0))
      days++;
    return days;
  }
  return -1;
}

void 
CScreen6::adjTimeDate(int dir)
{
  int days;
  switch(_colSel) {
    case 0:
      _day += dir;
      days = daysInMonth(_month, _year);
      ROLLUPPERLIMIT(_day, days, 1);
      ROLLLOWERLIMIT(_day, 1, days);
      break;
    case 1:
      _month += dir;
      ROLLUPPERLIMIT(_month, 12, 1);
      ROLLLOWERLIMIT(_month, 1, 12);
      days = daysInMonth(_month, _year);   // trap shorter months
      UPPERLIMIT(_day, days);
      break;
    case 2:
      _year += dir;
      days = daysInMonth(_month, _year);
      UPPERLIMIT(_day, days);              // pick up 29 Feb
      break;
    case 3:
      _hour += dir;
      ROLLUPPERLIMIT(_hour, 23, 0);
      ROLLLOWERLIMIT(_hour, 0, 23);
      break;
    case 4:
      _min += dir;
      ROLLUPPERLIMIT(_min, 59, 0);
      ROLLLOWERLIMIT(_min, 0, 59);
      break;
    case 5:
      _sec += dir;
      ROLLUPPERLIMIT(_sec, 59, 0);
      ROLLLOWERLIMIT(_sec, 0, 59);
      break;
  }
}
