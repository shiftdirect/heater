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

#ifndef __BTCDATETIME_H__
#define __BTCDATETIME_H__

#include "../../lib/RTClib/RTClib.h"

const char daysOfTheWeek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

class BTCDateTime : public DateTime {
  const char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  const char monthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
public:
  const char* monthStr() const;
  const char* dowStr() const;
  int  daysInMonth(int month, int year) const;
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
