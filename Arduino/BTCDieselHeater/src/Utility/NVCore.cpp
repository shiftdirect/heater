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

#include <Arduino.h>
#include "NVCore.h"
#include "DebugPort.h"


bool 
CESP32_NVStorage::validatedLoad(const char* key, char* val, int maxlen, const char* defVal)
{
  char probe[128];
  bool retval = true;
  strcpy(probe, "TestPresence");
  int len = preferences.getString(key, probe, 127);
  if(len == 0 || strcmp(probe, "TestPresence") == 0) {
    preferences.putString(key, defVal);
    DebugPort.printf("CESP32HeaterStorage::validatedLoad<char*> default installed %s=%s", key, defVal);
    retval = false;
  }
  preferences.getString(key, val, maxlen);
  val[maxlen] = 0;  // ensure null terminated
  return retval;
}

bool
CESP32_NVStorage::validatedLoad(const char* key, uint8_t& val, int defVal, std::function<bool(uint8_t, uint8_t, uint8_t)> validator, int min, int max, uint8_t mask)
{
  val = preferences.getUChar(key, defVal);
  if(!validator(val & mask, min, max)) {

    DebugPort.printf("CESP32HeaterStorage::validatedLoad<uint8_t> invalid read %s=%d", key, val);
    DebugPort.printf(" validator(%d,%d) reset to %d\r\n", min, max, defVal);

    val = defVal;
    preferences.putUChar(key, val);
    return false;
  }
  return true;
}

bool
CESP32_NVStorage::validatedLoad(const char* key, int8_t& val, int defVal, std::function<bool(int8_t, int8_t, int8_t)> validator, int min, int max)
{
  val = preferences.getChar(key, defVal);
  if(!validator(val, min, max)) {

    DebugPort.printf("CESP32HeaterStorage::validatedLoad<int8_t> invalid read %s=%d", key, val);
    DebugPort.printf(" validator(%d,%d) reset to %d\r\n", min, max, defVal);

    val = defVal;
    preferences.putChar(key, val);
    return false;
  }
  return true;
}

bool
CESP32_NVStorage::validatedLoad(const char* key, uint16_t& val, int defVal, std::function<bool(uint16_t, uint16_t, uint16_t)> validator, int min, int max)
{
  val = preferences.getUShort(key, defVal);
  if(!validator(val, min, max)) {

    DebugPort.printf("CESP32HeaterStorage::validatedLoad<uint16_t> invalid read %s=%d", key, val);
    DebugPort.printf(" validator(%d,%d) reset to %d\r\n", min, max, defVal);

    val = defVal;
    preferences.putUShort(key, val);
    return false;
  }
  return true;
}

bool
CESP32_NVStorage::validatedLoad(const char* key, long& val, long defVal, std::function<bool(long, long, long)> validator, long min, long max)
{
  val = preferences.getLong(key, defVal);
  if(!validator(val, min, max)) {

    DebugPort.printf("CESP32HeaterStorage::validatedLoad<long> invalid read %s=%ld", key, val);
    DebugPort.printf(" validator(%ld,%ld) reset to %ld\r\n", min, max, defVal);

    val = defVal;
    preferences.putLong(key, val);
    return false;
  }
  return true;
}

bool u8inBounds(uint8_t test, uint8_t minLim, uint8_t maxLim)
{
  return (test >= minLim) && (test <= maxLim);
}

bool s8inBounds(int8_t test, int8_t minLim, int8_t maxLim)
{
  return (test >= minLim) && (test <= maxLim);
}

bool u8Match2(uint8_t test, uint8_t test1, uint8_t test2)
{
  return (test == test1) || (test == test2);
}

bool u16inBounds(uint16_t test, uint16_t minLim, uint16_t maxLim)
{
  return (test >= minLim) && (test <= maxLim);
}

bool s32inBounds(long test, long minLim, long maxLim)
{
  return (test >= minLim) && (test <= maxLim);
}


