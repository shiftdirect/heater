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
  static void createMap(int timermask, uint16_t map[24*60], uint16_t timerIDs[24*60]);
  static void createMap(sTimer& timer, uint16_t timerMap[24*60], uint16_t timerIDs[24*60]);
  static void condenseMap(uint16_t timerMap[24*60], int factor);
  static int  conflictTest(sTimer& timer);
};

#endif //__TIMERMANAGER_H__