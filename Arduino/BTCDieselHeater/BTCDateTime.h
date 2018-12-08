#ifndef __BTCDATETIME_H__
#define __BTCDATETIME_H__

#include "RTClib.h"

class BTCDateTime : public DateTime {
  const char daysOfTheWeek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  const char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  const char monthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
public:
  const char* monthStr() const;
  const char* dowStr() const;
  const char* briefDowStr() const;
  int daysInMonth(int month, int year);
  void adjustDay(int val);
  void adjustMonth(int val);
  void adjustYear(int dir);
  void adjustHour(int dir);
  void adjustMinute(int dir);
  void adjustSecond(int dir);
  BTCDateTime& operator=(const DateTime& rhs);
  BTCDateTime& operator=(const BTCDateTime& rhs);
};

#endif  // __BTCDATETIME_H__
