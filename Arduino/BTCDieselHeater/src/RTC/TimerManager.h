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
// CTimerManager
//
// This provides management of the timers
//
///////////////////////////////////////////////////////////////////////////

#ifndef __TIMERMANAGER_H__
#define __TIMERMANAGER_H__

#include <stdint.h>

struct sTimer;

class CTimerManager {
public:
  static const int _dayMinutes = 24*60;
  static void createMap(int timermask = 0x3fff, uint16_t* timerMap = NULL, uint16_t* timerIDs = NULL);
  static void createMap(sTimer& timer, uint16_t* timerMap = NULL, uint16_t* timerIDs = NULL);
  static void condenseMap(uint16_t timerMap[_dayMinutes], int factor);
  static void condenseMap(uint8_t timerMap[7][120]);
  static int  conflictTest(sTimer& timer);
  static int  conflictTest(int ID);
  static int  manageTime(int hour, int minute, int dow);
  static int  findNextTimer(int hour, int minute, int dow);
  static int  getNextTimer();
  static void getTimer(int idx, sTimer& timerInfo);
  static int  setTimer(sTimer& timerInfo);
private:
  static int activeTimer;
  static int activeDow;
  static int prevState;
  static int nextTimer;
  static int nextStart;
  static uint8_t weekTimerIDs[7][_dayMinutes];   // b[7] = repeat flag, b[3..0] = timer ID
};

#endif //__TIMERMANAGER_H__