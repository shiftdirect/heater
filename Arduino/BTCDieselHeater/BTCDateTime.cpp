#include "BTCDateTime.h"
#include "helpers.h"

const char*
BTCDateTime::dowStr() const
{
  return daysOfTheWeek[dayOfTheWeek()];
}

const char*
BTCDateTime::monthStr()  const
{
  return months[month()-1];
}

BTCDateTime& 
BTCDateTime::operator=(const DateTime& rhs)
{
  yOff = rhs.year()-2000;
  m = rhs.month();
  d = rhs.day();
  hh = rhs.hour();
  mm = rhs.minute();
  ss = rhs.second();
}

BTCDateTime& 
BTCDateTime::operator=(const BTCDateTime& rhs)
{
  yOff = rhs.yOff;
  m = rhs.m;
  d = rhs.d;
  hh = rhs.hh;
  mm = rhs.mm;
  ss = rhs.ss;
}

void 
BTCDateTime::adjustDay(int dir)
{
  int days = daysInMonth(m, yOff);
  if(dir > 0) {
    if(d == days)  d = 1;
    else  d++;
  }
  else {
    if(d == 1) d = days;
    else  d--;
  }
}

void
BTCDateTime::adjustMonth(int dir)
{
  if(dir > 0) {
    if(m == 12)  m = 1;
    else m++;
  }
  else {
    if(m == 1)  m = 12;
    else  m--;
  }
  int days = daysInMonth(m, yOff);   // trap shorter months
  UPPERLIMIT(d, days);
}

void
BTCDateTime::adjustYear(int dir)
{
  yOff += dir;
  int days = daysInMonth(m, yOff);
  UPPERLIMIT(d, days);              // pick up 29 Feb
}

void
BTCDateTime::adjustHour(int dir)
{
  if(dir > 0) {
    if(hh == 23)  hh = 0;
    else  hh++;
  }
  else {
    if(hh == 0)  hh = 23;
    else  hh--;
  }
}

void
BTCDateTime::adjustMinute(int dir)
{
  if(dir > 0) {
    if(mm == 59)  mm = 0;
    else  mm++;
  }
  else {
    if(mm == 0)  mm = 59;
    else  mm--;
  }
}

void
BTCDateTime::adjustSecond(int dir)
{
  if(dir > 0) {
    if(ss == 59)  ss = 0;
    else  ss++;
  }
  else {
    if(ss == 0)  ss = 59;
    else  ss--;
  }
}

int 
BTCDateTime::daysInMonth(int month, int year)
{
  if(month >= 1 && month <= 12) {
    int days = monthDays[month-1];
    if((month == 2) && ((year & 0x03) == 0))
      days++;
    return days;
  }
  return -1;
}
