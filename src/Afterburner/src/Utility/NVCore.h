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

#ifndef __BTC_NV_CORE_H__
#define __BTC_NV_CORE_H__

#include <Preferences.h>
#include <functional>


bool u8inBounds(uint8_t test, uint8_t minLim, uint8_t maxLim);
bool s8inBounds(int8_t test, int8_t minLim, int8_t maxLim);
bool u8Match2(uint8_t test, uint8_t test1, uint8_t test2);
bool u16inBounds(uint16_t test, uint16_t minLim, uint16_t maxLim);
bool thermoMethodinBounds(uint8_t test, uint8_t minLim, uint8_t maxLim);

class CNVStorage {
  public:
    CNVStorage() {};
    virtual ~CNVStorage() {};
    virtual void init() = 0;
    virtual void load() = 0;
    virtual void save() = 0;
    virtual bool valid() = 0;
};

class CESP32_NVStorage {
protected:
  Preferences preferences;
protected:
  bool validatedLoad(const char* key, int8_t& val, int defVal, std::function<bool(int8_t, int8_t, int8_t)> validator, int min, int max);
  bool validatedLoad(const char* key, uint8_t& val, int defVal, std::function<bool(uint8_t, uint8_t, uint8_t)> validator, int min, int max, uint8_t mask=0xff);
  bool validatedLoad(const char* key, uint16_t& val, int defVal, std::function<bool(uint16_t, uint16_t, uint16_t)> validator, int min, int max);
  bool validatedLoad(const char* key, long& val, long defVal, long min, long max);
  bool validatedLoad(const char* key, char* val, int maxlen, const char* defVal);
  bool validatedLoad(const char* key, float& val, float defVal, float min, float max);
};


#endif // __BTC_NV_CORE_H__

