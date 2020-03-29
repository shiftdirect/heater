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
#include "../Utility/helpers.h"
#include "../RTC/RTCStore.h"

// main array to hold information of which timer is active at any particular minute of the week
// LSBs are used for the timerID + 1
// MSB is set if the timer repeats
uint8_t CTimerManager::_weekMap[7][CTimerManager::_dayMinutes];   // b[7] = repeat flag, b[3..0] = timer ID

int  CTimerManager::_activeTimer = 0;
int  CTimerManager::_activeDow = 0;
int  CTimerManager::_nextTimer = 0;
int  CTimerManager::_nextStart = 0;
uint8_t CTimerManager::_workingTemperature = 22;
bool CTimerManager::_timerChanged = false;

#define SET_MAPS() {                         \
  if(pTimerMap) {                            \
    pTimerMap[dayMinute] |= activeday;      \
    if(pTimerIDs)                            \
      pTimerIDs[dayMinute] |= timerBit;      \
  }                                          \
  else {                                     \
    _weekMap[dow][dayMinute] = recordTimer;  \
  }                                          \
}

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
    DebugPort.println("Erasing weekMap");
    memset(_weekMap, 0, _dayMinutes*7*sizeof(uint8_t));
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
  if(createOneShotMap(timer, pTimerMap, pTimerIDs)) {
    return;
  }

  if(timer.enabled) {
    // create linear minutes of day values for start & stop
    // note that if stop < start, the timer rolls over midnight
    int timerBit = 0x0001 << timer.timerID;                   // bit required for timer ID map
    int timestart = timer.start.hour * 60 + timer.start.min;  // linear minute of day
    int timestop = timer.stop.hour * 60 + timer.stop.min;
    for(int dayMinute = 0; dayMinute < _dayMinutes; dayMinute++) {
      for(int dow = 0; dow < 7; dow++) {
        int dayBit = 0x01 << dow;
        if(timer.enabled & dayBit || timer.enabled & 0x80) {  // specific or everyday
          uint16_t activeday = dayBit;  // may also hold non repeat flag later
          uint8_t recordTimer = (timer.timerID + 1) | (timer.repeat ? 0x80 : 0x00);  // full week timer ID map
          if(!timer.repeat) {
            // flag timers that should get cancelled
            activeday |= (activeday << 8);  // combine one shot status in MS byte
          }

          // SET_MAPS() macro saves values as below:
          //
          //   activeday -> pTimerMap[dayMinute]           (if pTimerMap != NULL)
          //   timerBit -> pTimerID[dayMinute]             (if pTimerMap != NULL AND pTimerID != NULL)
          //   recordTimer -> weekMap[dow][dayMinute]      (if pTimerMap == NULL)
          //
          if(timestop > timestart) {
            // treat normal start < stop times (within same day)
            if((dayMinute >= timestart) && (dayMinute < timestop)) {
              SET_MAPS();
            }
          }
          else {  
            // time straddles a day, start > stop, special treatment required
            if(dayMinute >= timestart) {  
              // true from start until midnight
              SET_MAPS();
            }
            if(dayMinute < timestop) {
              // after midnight, before stop time, i.e. next day
              // adjust for next day, taking care to wrap week
              if(dow == 6) {     // last day of week?
                dow = 0;
                // because activeday holds both cancel and day info, shift it 
                activeday >>= 6;  // roll back to start of week - 
              }
              else {
                dow++;
                activeday <<= 1;  // next day
              }
              SET_MAPS();
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
          condense = _weekMap[dow][dayMinute];
      }
      timerMap[dow][opIndex++] = condense;
    }
  }
  _timerChanged = false;
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
  int newID = _weekMap[dow][dayMinute];
  if(_activeTimer != newID) {
    
    DebugPort.printf("Timer ID change detected: %d", _activeTimer & 0x0f); 
    if(_activeTimer & 0x80) DebugPort.print("(repeating)");
    DebugPort.printf(" -> %d", newID & 0x0f);
    if(newID & 0x80) DebugPort.print("(repeating)");
    DebugPort.println("");

    if(_activeTimer) {  
      // deal with expired timer
      DebugPort.println("Handling expired timer cleanup");

      if(_activeTimer & 0x80) {
        DebugPort.println("Expired timer repeats, leaving definition alone");
      }
      else {  // non repeating timer
        // delete one shot timer - note that this may require ticking off each day as they appear
        DebugPort.printf("Expired timer does not repeat - Cancelling %d\r\n", _activeTimer);
        int ID = _activeTimer & 0x0f;
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
            DebugPort.printf("Cancelling specific day idx %d\r\n", _activeDow);
            timer.enabled &= ~(0x01 << _activeDow);  // cancel specific day that started the timer
          }
          NVstore.setTimerInfo(ID, timer);
          NVstore.save();
          createMap();
        }
      }
    }

    if(newID) {
      sTimer timer;
      // get timer settings
      int ID = (newID & 0xf) - 1;
      NVstore.getTimerInfo(ID, timer);
      _workingTemperature = timer.temperature;
      DebugPort.printf("Start of timer interval, starting heater @ %dC\r\n", _workingTemperature);
      requestOn();
      _activeDow = dow;   // dow when timer interval start was detected
      retval = 1;
    }
    else {
//      if(!RTC_Store.getFrostOn() && !RTC_Store.getCyclicEngaged())
      if(!RTC_Store.getFrostOn())
        requestOff();
      retval = 2;
      _workingTemperature = RTC_Store.getDesiredTemp();
      DebugPort.printf("End of timer interval, stopping heater & %dC\r\n", _workingTemperature);
    }
    _activeTimer = newID;
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
    if(_weekMap[dow][dayMinute] & 0x0f) {
      _nextTimer = _weekMap[dow][dayMinute];
      _nextStart = dow*_dayMinutes + dayMinute;
      return _nextTimer;
    }
    dayMinute++;
    if(dayMinute == _dayMinutes) {
      dayMinute = 0;
      dow++;
      WRAPUPPERLIMIT(dow, 6, 0);
    }
  }
  _nextTimer = 0;
  return 0;
}

int 
CTimerManager::getNextTimer()
{
  return _nextTimer;
}

int  
CTimerManager::getActiveTimer()
{
  return _activeTimer;
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
    _timerChanged = true;
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
  _timerChanged = true;
  return conflictID;
}

// special handling for next occurence of one-shot, non-repeating timers 
bool
CTimerManager::createOneShotMap(sTimer& timer, uint16_t* pTimerMap, uint16_t* pTimerIDs)
{
  if((timer.enabled == 0x80) && !timer.repeat) {  // on-shot next occurrence timer
    DebugPort.printf("One shot, next occurence timer #%d\r\n", timer.timerID+1);
    int timerBit = 0x0001 << timer.timerID;                   // value required for full week map
    int timestart = timer.start.hour * 60 + timer.start.min;  // linear minute of day
    int timestop = timer.stop.hour * 60 + timer.stop.min;
    // create masking based upon TODAY
    const BTCDateTime tNow = Clock.get();
    int dow = tNow.dayOfTheWeek();
    int todayTime = tNow.hour() * 60 + tNow.minute();
    // wrap to next day if start time falls behind current time
    if(todayTime >= timestart) {
      dow++;
      WRAPUPPERLIMIT(dow, 6, 0);
    }
    // create masks and record values for the assorted target arrays
    uint16_t activeday = 1 << dow;  // set day bit
    activeday |= activeday << 8;    // and set non repeat flag in MSB
    uint8_t recordTimer = (timer.timerID + 1);

    // SET_MAPS() macro saves values as below:
    //
    //   activeday -> pTimerMap[dayMinute]          (if pTimerMap != NULL)
    //   timerBit -> pTimerID[dayMinute]            (if pTimerMap != NULL AND pTimerID != NULL)
    //   recordTimer -> weekMap[dow][dayMinute]     (if pTimerMap == NULL)
    //
    if(timestart < timestop) {  
      // timer does not wrap midnight - easy linear workout :-)
      for(int dayMinute = timestart; dayMinute < timestop; dayMinute++) {
        SET_MAPS();
      }
    }
    else {  
      // timer rolls over midnight
      // fill map up from start time till end of day
      for(int dayMinute = timestart; dayMinute < _dayMinutes; dayMinute++) {
        SET_MAPS();
      }
      // advance to next day, wrapping if required
      dow++;
      WRAPUPPERLIMIT(dow, 6, 0);
      activeday = 1 << dow;           // set day bit
      activeday |= activeday << 8;    // and set non repeat flag in MSB
      // complete map from midnight till stop time
      for(int dayMinute = 0; dayMinute < timestop; dayMinute++) {
        SET_MAPS();
      }
    }
    return true;
  }
  return false;
}

// Concept of timer working temperature is that when a timer runs, it installs 
// the programmed temeprature for that timer as the new set point.
// When the timer stops, it reverts to the usual user set temperature.
//
// BUT, if a timer is running, the working temperature is updated with the new demand
// and the user temperature is also changed accordingly. 
// The programmed timer temeprature is not altered and wil lrecur in the future when that time runs.
uint8_t
CTimerManager::getWorkingTemperature()
{
  return _workingTemperature;
}

void 
CTimerManager::setWorkingTemperature(uint8_t newDegC)
{
  _workingTemperature = newDegC;
  RTC_Store.setDesiredTemp(newDegC);
}