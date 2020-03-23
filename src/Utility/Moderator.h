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

#ifndef __BTC_MODERATOR_H__
#define __BTC_MODERATOR_H__

#include <map>
#include "../../lib/ArduinoJson/ArduinoJson.h"
#include "../RTC/Timers.h"
#include "DebugPort.h"
#include "BTC_GPIO.h"


class CTimerModerator {
  sTimer Memory[14];
  enum eType { eStart, eStop, eDays, eRpt, eTemp};
  const char* _getName(eType type);
  int _shouldSend(int channel, const sTimer& toSend);
public:
  CTimerModerator();
  bool addJson(int channel, const sTimer& toSend, JsonObject& root);
	void reset();
	void reset(int channel);
};


class CStringModerator {
  std::map<const char*, std::string> Memory;
public:
  const char* shouldSend(const char* name, const char* value);
  bool addJson(const char* name, const char* value, JsonObject& root);
	void reset();
	void reset(const char* name);
};

class sModeratorHoldoff {
  unsigned long period;
  unsigned long tripTime;
public:
  sModeratorHoldoff() {
    period = 0;
    tripTime = 0;
  }
  sModeratorHoldoff(const sModeratorHoldoff& rhs) {
    period = rhs.period;
    tripTime = rhs.tripTime;
  }
  void set(unsigned long per) {
    period = per;
    reArm();
  }
  void reArm() {
    tripTime = (millis() + period) | 1;
  }
  bool expired() {
    long tDelta = millis() - tripTime;
    return tDelta >= 0;
  }
  void expire() {
    tripTime = millis();
  }
};

template <class T>
class TModerator {
  std::map<const char*, T> Memory;
  std::map<const char*, sModeratorHoldoff> _holdoff;
public:
  bool shouldSend(const char* name, T value);
  void setHoldoff(const char* name, unsigned long period);
  bool addJson(const char* name, T value, JsonObject& root, unsigned long holdoff=0);
	void reset();
	void reset(const char* name);
};

template<class T>
bool TModerator<T>::shouldSend(const char* name, T value) 
{
  bool retval = true;
  auto it = Memory.find(name);
  if(it != Memory.end()) {
    retval = it->second != value;
    if(retval) {
      // check if a minimum refresh interval has been defined
      auto holdoff = _holdoff.find(name);
      if(holdoff != _holdoff.end()) {
        if(holdoff->second.expired()) {
          holdoff->second.reArm();
        }
        else {
          retval = false;
        }
      }
    }
    if(retval) {
      it->second = value;
    }
  }
  else {
    Memory[name] = value;
  }
  return retval;
}

template<class T>
void TModerator<T>::setHoldoff(const char* name, unsigned long period)
{
  if(period) {
    auto it = _holdoff.find(name);
    if(it == _holdoff.end()) {
      sModeratorHoldoff holdoff;
      holdoff.set(period);
      _holdoff[name] = holdoff;
    }
  }
}

template<class T>
bool TModerator<T>::addJson(const char* name, T value, JsonObject& root, unsigned long holdoff) 
{
  setHoldoff(name, holdoff);
  bool retval = shouldSend(name, value);
  if(retval) {
    root.set(name, value);
  }
  return retval;
}


template<class T>
void TModerator<T>::reset() 
{
 	for(auto it = Memory.begin(); it != Memory.end(); ++it) {
    it->second = it->second+100;
  } 
 	for(auto it = _holdoff.begin(); it != _holdoff.end(); ++it) {
     it->second.expire();
  }
}

template<class T>
void TModerator<T>::reset(const char* name)
{
  {
    auto it = Memory.find(name);
    if(it != Memory.end()) {
      DebugPort.printf("Resetting moderator: \"%s\"", name);
      it->second = it->second+100;
    }
  }
  {
    auto it = _holdoff.find(name);
    if(it != _holdoff.end()) {
      it->second.expire();
    }
  }
}

class CModerator {
  TModerator<uint32_t> u32Moderator;
  TModerator<int> iModerator;
  TModerator<float> fModerator;
  TModerator<uint8_t> ucModerator;
  CStringModerator szModerator;
public:
  // integer values
  bool addJson(const char* name, int value, JsonObject& root) { 
    return iModerator.addJson(name, value, root); 
  };
  bool addJson(const char* name, uint32_t value, JsonObject& root) { 
    return u32Moderator.addJson(name, value, root); 
  };
  bool addJson(const char* name, unsigned long value, JsonObject& root) { 
    return u32Moderator.addJson(name, value, root); 
  };
  // float values
  bool addJson(const char* name, float value, JsonObject& root, unsigned long holdoff=0) { 
    return fModerator.addJson(name, value, root, holdoff); 
  };
  // uint8_t values
  bool addJson(const char* name, uint8_t value, JsonObject& root) { 
    return ucModerator.addJson(name, value, root); 
  };
  // const char* values
  bool addJson(const char* name, const char* value, JsonObject& root) { 
    return szModerator.addJson(name, value, root); 
  };
  bool shouldSend(const char* name, int value) { 
    return iModerator.shouldSend(name, value); 
  };
  bool shouldSend(const char* name, uint32_t value) { 
    return u32Moderator.shouldSend(name, value); 
  };
  bool shouldSend(const char* name, unsigned long value) { 
    return u32Moderator.shouldSend(name, value); 
  };
  bool shouldSend(const char* name, float value) { 
    return fModerator.shouldSend(name, value); 
  };
  bool shouldSend(const char* name, uint8_t value) { 
    return ucModerator.shouldSend(name, value); 
  };
  bool shouldSend(const char* name, const char* value) { 
    return szModerator.shouldSend(name, value); 
  };
  // force changes on all held values
  void reset() {
    iModerator.reset();
    fModerator.reset();
    ucModerator.reset();
    szModerator.reset();
    u32Moderator.reset();
  };
  void reset(const char* name) {
    iModerator.reset(name);
    fModerator.reset(name);
    ucModerator.reset(name);
    szModerator.reset(name);
    u32Moderator.reset(name);
  };
};

#endif // __BTC_MODERATOR_H__
