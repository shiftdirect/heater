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

#include "Moderator.h"
#include "DebugPort.h"
//#include "NVStorage.h"

CTimerModerator::CTimerModerator()
{
  reset();
}

const char* 
CTimerModerator::_getName(eType type) 
{
  switch(type) {
    case eStart: return "TimerStart";
    case eStop: return "TimerStop";
    case eDays: return "TimerDays";
    case eRpt: return "TimerRepeat";
    case eTemp: return "TimerTemp";
    default: return "TimerType?";
  }
}

int 
CTimerModerator::_shouldSend(int timer, const sTimer& toSend)
{
  int retval = 0;
  if(Memory[timer].start != toSend.start)
    retval |= (0x01 << eStart);
  if(Memory[timer].stop != toSend.stop)
    retval |= (0x01 << eStop);
  if(Memory[timer].enabled != toSend.enabled)
    retval |= (0x01 << eDays);
  if(Memory[timer].repeat != toSend.repeat)
    retval |= (0x01 << eRpt);
  if(Memory[timer].temperature != toSend.temperature)
    retval |= (0x01 << eTemp);

  Memory[timer] = toSend;

  return retval;
}

bool 
CTimerModerator::addJson(int timer, const sTimer& toSend, JsonObject& root)
{
  int retval = _shouldSend(timer, toSend);
  if( retval ) {
    for(int JSONtype=0; JSONtype<5; JSONtype++) {
      if(retval & (0x01 << JSONtype))
        root.set(_getName(eType(JSONtype)), getTimerJSONStr(timer, JSONtype));
    }
  }
  return retval != 0;
}

void
CTimerModerator::reset()
{
  for(int timer= 0; timer < 14; timer++) {
    Memory[timer].start.hour = -1;  // force full update
    Memory[timer].stop.hour = -1;  // force full update
    Memory[timer].enabled = 0xff;  // invalid combination - force full update
    Memory[timer].repeat = 0xff;    
    Memory[timer].temperature = 0xff;  
  } 
}

void
CTimerModerator::reset(int timer)
{
  if(timer >= 0 && timer < 14) {
    Memory[timer].start.hour = -1;  // force full update
    Memory[timer].stop.hour = -1;  // force full update
    Memory[timer].enabled = 0xff;  // invalid combination - force full update
    Memory[timer].repeat = 0xff;    
    Memory[timer].temperature = 0xff;  
  } 
}


const char* 
CStringModerator::shouldSend(const char* name, const char* value) 
{
  std::string sValue = value;
  auto it = Memory.find(name);
  if(it != Memory.end()) {
    if(it->second == sValue)
      return NULL;    // unchanged
    it->second = sValue;
    return it->second.c_str();
  }
  else {
    return (Memory[name] = sValue).c_str();
  }
}

bool 
CStringModerator::addJson(const char* name, const char* value, JsonObject& root) 
{
  const char* toSend = shouldSend(name, value);  // returns pointer to mapped value - persistent!!!!
  if(toSend) {
    root.set(name, toSend);  // use std::string held in this class's Memory - can trust this is persistent!
    return true;
  }
  return false;
}

void 
CStringModerator::reset() 
{
 	for(auto it = Memory.begin(); it != Memory.end(); ++it) {
    Memory.erase(it);
  } 
}

void 
CStringModerator::reset(const char* name)
{
  auto it = Memory.find(name);
  if(it != Memory.end()) {
    Memory.erase(it);
  }
}
/*
CGPIOModerator::CGPIOModerator()
{
  reset();
}

void
CGPIOModerator::reset()
{
  Memory.inState[0] = !Memory.inState[0];
  Memory.inState[1] = !Memory.inState[1];
  Memory.outState[0] = !Memory.outState[0];
  Memory.outState[1] = !Memory.outState[1];
  Memory.algVal = Memory.algVal+10000;
  Memory.inMode = (GPIOinModes)-1;
  Memory.outMode = (GPIOoutModes)-1;
  Memory.algMode = (GPIOalgModes)-1;
}


bool 
CGPIOModerator::addJson(const sGPIO& toSend, JsonObject& root)
{
  bool retval = false;

  char msgs[5][16];
  if(Memory.inState[0] != toSend.inState[0]) {
    root.set("GPin1", toSend.inState[0] ? 1 : 0);
    retval = true;
  }
  if(Memory.inState[1] != toSend.inState[1]) {
    root.set("GPin2", toSend.inState[1] ? 1 : 0);
    retval = true;
  }
  if(Memory.outState[0] != toSend.outState[0]) {
    root.set("GPout1", toSend.outState[0] ? 1 : 0);
    retval = true;
  }
  if(Memory.outState[1] != toSend.outState[1]) {
    root.set("GPout2", toSend.outState[1] ? 1 : 0);
    retval = true;
  }
  if(Memory.algVal != toSend.algVal) {
    root.set("GPalg", toSend.algVal);
    retval = true;
  }
  if(Memory.inMode != toSend.inMode) {
    root.set("GPinMode", GPIOinNames[toSend.inMode]);
    retval = true;
  }
  if(Memory.outMode != toSend.outMode) {
    root.set("GPoutMode", GPIOoutNames[toSend.outMode]);
    retval = true;
  }
  if(Memory.algMode != toSend.algMode) {
    root.set("GPalgMode", GPIOalgNames[toSend.algMode]);
    retval = true;
  }
  
  Memory = toSend;

  return retval;
}
*/