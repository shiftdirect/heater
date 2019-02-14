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

#include <Arduino.h>
#include "TimerManager.h"
#include "../Utility/NVStorage.h"

// create a bitmap that describes the pattern of on/off times
void 
CTimerManager::createMap(int timerMask, uint16_t timerMap[24*60], uint16_t timerIDs[24*60])
{
  int maxPoints = 24*60;
  memset(timerMap, 0, 24*60*sizeof(uint16_t));
  memset(timerIDs, 0, 24*60*sizeof(uint16_t));
  
  for(int timerID=0; timerID < 14; timerID++) {
    // only process timer if it is nominated in timerMask (bitfield), timer0 = bit0 .. timerN = bitN
    uint16_t timerBit = 0x0001 << timerID;
    if(timerMask & timerBit) {
      sTimer timer;
      // get timer settings
      NVstore.getTimerInfo(timerID, timer);
      // and add info to map if enabled
      if(timer.enabled) {
        // create linear minute of day values for start & stop
        // note that if stop < start, that is treated as a timer that rolls over midnight
        int timestart = timer.start.hour * 60 + timer.start.min;  // linear minute of day
        int timestop = timer.stop.hour * 60 + timer.stop.min;
        for(int dayMinute = 0; dayMinute < maxPoints; dayMinute++) {
          for(int day = 0x01; day != 0x80; day <<= 1) {
            if(timer.enabled & day || timer.enabled & 0x80) {  // specific or everyday
              uint16_t activeday = day;  // may also hold non repeat flag later
              if(!timer.repeat) {
                // flag timers that should get cancelled
                activeday |= (activeday << 8);  // combine one shot status in MS byte
              }
              if(timestop > timestart) {
                // treat normal start < stop times (within same day)
                if((dayMinute >= timestart) && (dayMinute < timestop)) {
                  timerMap[dayMinute] |= activeday;
                  timerIDs[dayMinute] |= timerBit;
                }
              }
              else {  
                // time straddles a day, start > stop, special treatment required
                if(dayMinute >= timestart) {  
                  // true from start until midnight
                  timerMap[dayMinute] |= activeday;
                  timerIDs[dayMinute] |= timerBit;
                }
                if(dayMinute < timestop) {
                  // after midnight, before stop time, i.e. next day
                  // adjust for next day, taking care to wrap week
                  if(day & 0x40)      // last day of week?
                    activeday >>= 6;  // roll back to start of week
                  else 
                    activeday <<= 1;  // next day
                  timerMap[dayMinute] |= activeday; 
                  timerIDs[dayMinute] |= timerBit;
                } 
              }
            }
          }
        }
      }
    }
  }
}

void 
CTimerManager::condenseMap(uint16_t timerMap[24*60], int factor)
{
  int maxPoints = 24*60;

  int opIndex = 0;
  for(int dayMinute = 0; dayMinute < maxPoints; ) {
    uint16_t condense = 0;
    for(int subInterval = 0; subInterval < factor; subInterval++) {
      condense |= timerMap[dayMinute++];
      if(dayMinute == maxPoints) {
        break;
      }
    }
    timerMap[opIndex++] = condense;
  }
}

uint16_t otherTimers[24*60];
uint16_t selectedTimer[24*60];
uint16_t timerIDs[24*60];

int  
CTimerManager::conflictTest(int timerID)
{
  int selectedMask = 0x0001 << timerID;

  createMap(selectedMask, selectedTimer, timerIDs);  // create a map for the nominated timer (under test)
  createMap(0x3fff & ~selectedMask, otherTimers, timerIDs);   // create a map for all other timers, and get their unique IDs
  for(int i=0; i< 24*60; i++) {
    if(otherTimers[i] & selectedTimer[i]) {  // both have the same day bit set - CONFLICT!
      uint16_t timerBit = timerIDs[i];
      int ID = 0;
      while(timerBit) {
        timerBit >>= 1;
        ID++;
      }
      return ID;  
    }
  }
  return 0; // no conflicts :-)
 }
