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
#include "Clock.h"
#include "../Utility/NVStorage.h"
#include "../Protocol/helpers.h"

uint8_t CTimerManager::weekTimerIDs[7][CTimerManager::_dayMinutes];   // b[7] = repeat flag, b[3..0] = timer ID

int  CTimerManager::activeTimer = 0;
int  CTimerManager::activeDow = 0;
int  CTimerManager::nextTimer = 0;
int  CTimerManager::nextStart = 0;
bool CTimerManager::timerChanged = false;

// create a bitmap that describes the pattern of on/off times
void 
CTimerManager::createMap(int timerMask, uint16_t* pTimerMap, uint16_t* pTimerIDs)
{
  if(pTimerMap) {
    memset(pTimerMap, 0, _dayMinutes*sizeof(uint16_t));
    if(pTimerIDs) 
      memset(pTimerIDs, 0, _dayMinutes*sizeof(uint16_t));
  }
  else {
    memset(weekTimerIDs, 0, _dayMinutes*7*sizeof(uint8_t));
  }
  
  for(int timerID=0; timerID < 14; timerID++) {
    // only process timer if it was nominated in supplied timerMask (bitfield), 
    // timer0 = bit0 .. timerN = bitN
    uint16_t timerBit = 0x0001 << timerID;
    if(timerMask & timerBit) {
      sTimer timer;
      // get timer settings
      NVstore.getTimerInfo(timerID, timer);
      // and add info to map if enabled
      createMap(timer, pTimerMap, pTimerIDs);
    }
  }
}

// create a timer map, based only upon the supplied timer info
// the other form of createMap uses the NV stored timer info
void 
CTimerManager::createMap(sTimer& timer, uint16_t* pTimerMap, uint16_t* pTimerIDs)
{
  int timerBit = 0x0001 << timer.timerID;

  if(timer.enabled) {
    // create linear minute of day values for start & stop
    // note that if stop < start, that is treated as a timer that rolls over midnight
    int timestart = timer.start.hour * 60 + timer.start.min;  // linear minute of day
    int timestop = timer.stop.hour * 60 + timer.stop.min;
    for(int dayMinute = 0; dayMinute < _dayMinutes; dayMinute++) {
      for(int dow = 0; dow < 7; dow++) {
        int dayBit = 0x01 << dow;
        if(timer.enabled & dayBit || timer.enabled & 0x80) {  // specific or everyday
          uint16_t activeday = dayBit;  // may also hold non repeat flag later
          uint8_t recordTimer = (timer.timerID + 1) | (timer.repeat ? 0x80 : 0x00);
          if(!timer.repeat) {
            // flag timers that should get cancelled
            activeday |= (activeday << 8);  // combine one shot status in MS byte
          }
          if(timestop > timestart) {
            // treat normal start < stop times (within same day)
            if((dayMinute >= timestart) && (dayMinute < timestop)) {
              if(pTimerMap) {
                pTimerMap[dayMinute] |= activeday;
                if(pTimerIDs) 
                  pTimerIDs[dayMinute] |= timerBit;
              }
              else {
                weekTimerIDs[dow][dayMinute] = recordTimer;
              }
            }
          }
          else {  
            // time straddles a day, start > stop, special treatment required
            if(dayMinute >= timestart) {  
              // true from start until midnight
              if(pTimerMap) {
                pTimerMap[dayMinute] |= activeday;
                if(pTimerIDs) 
                  pTimerIDs[dayMinute] |= timerBit;
              }
              else {
                weekTimerIDs[dow][dayMinute] = recordTimer;
              }
            }
            if(dayMinute < timestop) {
              // after midnight, before stop time, i.e. next day
              // adjust for next day, taking care to wrap week
              if(dow == 6) {     // last day of week?
                activeday >>= 6;  // roll back to start of week
                if(pTimerMap == NULL) {
                  weekTimerIDs[0][dayMinute] = recordTimer;
                }
              }
              else {
                activeday <<= 1;  // next day
                if(pTimerMap == NULL) {
                  weekTimerIDs[dow+1][dayMinute] = recordTimer;
                }
              }
              if(pTimerMap) {
                pTimerMap[dayMinute] |= activeday; 
                if(pTimerIDs)
                  pTimerIDs[dayMinute] |= timerBit;
              }
            } 
          }
        }
      }
    }
  }
}


void 
CTimerManager::condenseMap(uint16_t timerMap[_dayMinutes], int factor)
{
  int opIndex = 0;
  for(int dayMinute = 0; dayMinute < _dayMinutes; ) {
    uint16_t condense = 0;
    for(int subInterval = 0; subInterval < factor; subInterval++) {
      condense |= timerMap[dayMinute++];
      if(dayMinute == _dayMinutes) {
        break;
      }
    }
    timerMap[opIndex++] = condense;
  }
}

uint16_t otherTimers[CTimerManager::_dayMinutes];
uint16_t selectedTimer[CTimerManager::_dayMinutes];
uint16_t timerIDs[CTimerManager::_dayMinutes];


int  
CTimerManager::conflictTest(sTimer& timerInfo)
{
  int selectedMask = 0x0001 << timerInfo.timerID;  // bit mask for timer we are testing
  int othersMask = 0x3fff & ~selectedMask;

  memset(selectedTimer, 0, sizeof(selectedTimer));

  createMap(timerInfo, selectedTimer);            // create a usage map from the supplied timer info (under test)
  createMap(othersMask, otherTimers, timerIDs);   // create a map for all other timers, and get their unique IDs

  for(int i=0; i< _dayMinutes; i++) {
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

void
CTimerManager::condenseMap(uint8_t timerMap[7][120])
{
  for(int dow = 0; dow < 7; dow++) {
    int opIndex = 0;
    for(int dayMinute = 0; dayMinute < _dayMinutes; ) {
      uint8_t condense = 0;
      for(int subInterval = 0; subInterval < 12; subInterval++, dayMinute++) {
        if(!condense)
          condense = weekTimerIDs[dow][dayMinute];
      }
      timerMap[dow][opIndex++] = condense;
    }
  }
  timerChanged = false;
}

int  
CTimerManager::manageTime(int _hour, int _minute, int _dow)
{
  const BTCDateTime& currentTime = Clock.get();
  int hour = currentTime.hour();
  int minute = currentTime.minute();
  int dow = currentTime.dayOfTheWeek();

  int retval = 0;
  int dayMinute = (hour * 60) + minute;
  int newID = weekTimerIDs[dow][dayMinute];
  if(activeTimer != newID) {
    
    DebugPort.printf("Timer ID change detected: %d", activeTimer & 0x0f); 
    if(activeTimer & 0x80) DebugPort.print("(repeating)");
    DebugPort.printf(" -> %d", newID & 0x0f);
    if(newID & 0x80) DebugPort.print("(repeating)");
    DebugPort.println("");

    if(activeTimer) {  
      // deal with expired timer
      DebugPort.println("Handling expired timer cleanup");

      if(activeTimer & 0x80) {
        DebugPort.println("Expired timer repeats, leaving definition alone");
      }
      else {  // non repeating timer
        // delete one shot timer - note that this may require ticking off each day as they appear
        DebugPort.printf("Expired timer does not repeat - Cancelling %d\r\n", activeTimer);
        int ID = activeTimer & 0x0f;
        if(ID) {
          ID--;
          sTimer timer;
          // get timer settings
          NVstore.getTimerInfo(ID, timer);
          if(timer.enabled & 0x80) {
            DebugPort.println("Cancelling next day"); 
            timer.enabled = 0;   // ouright cancel anyday timer
          }
          else {
            DebugPort.printf("Cancelling specific day idx %d\r\n", activeDow);
            timer.enabled &= ~(0x01 << activeDow);  // cancel specific day that started the timer
          }
          NVstore.setTimerInfo(ID, timer);
          NVstore.save();
          createMap();
        }
      }
    }

    if(newID) {
      DebugPort.println("Start of timer interval, starting heater");
      requestOn();
      activeDow = dow;   // dow when timer interval start was detected
      retval = 1;
    }
    else {
      DebugPort.println("End of timer interval, stopping heater");
      requestOff();
      retval = 2;
    }
    activeTimer = newID;
  }
  findNextTimer(hour, minute, dow);
  return retval;
}

int  
CTimerManager::findNextTimer(int hour, int minute, int dow)
{
  int dayMinute = hour*60 + minute;

  int limit = 24*60*7;  
  while(limit--) {
    if(weekTimerIDs[dow][dayMinute] & 0x0f) {
      nextTimer = weekTimerIDs[dow][dayMinute];
      nextStart = dow*_dayMinutes + dayMinute;
      return nextTimer;
    }
    dayMinute++;
    if(dayMinute == _dayMinutes) {
      dayMinute = 0;
      dow++;
      ROLLUPPERLIMIT(dow, 6, 0);
    }
  }
  nextTimer = 0;
  return 0;
}

int 
CTimerManager::getNextTimer()
{
  return nextTimer;
}

void
CTimerManager::getTimer(int idx, sTimer& timerInfo)
{
  NVstore.getTimerInfo(idx, timerInfo);
}

int 
CTimerManager::setTimer(sTimer& timerInfo)
{
  if(!conflictTest(timerInfo)) {
    NVstore.setTimerInfo(timerInfo.timerID, timerInfo);
    NVstore.save();
    createMap();
    manageTime(0,0,0);
    timerChanged = true;
    return 1;
  }
  return 0;
}

int 
CTimerManager::conflictTest(int ID)
{
  if(!(ID >= 0 && ID < 14))
    return 0;

  sTimer timerInfo;
  CTimerManager::getTimer(ID, timerInfo);   // get info for selected timer
  int conflictID = CTimerManager::conflictTest(timerInfo);   // test against all others
  if(conflictID) {
    timerInfo.enabled = 0;   // cancel enabled status if it conflicts with others
    CTimerManager::setTimer(timerInfo);  // stage the timer settings, without being enabled
  }
  createMap();
  manageTime(0,0,0);
  timerChanged = true;
  return conflictID;
}
