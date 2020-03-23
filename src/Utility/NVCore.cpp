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
#include <functional>
#include <string.h>
#include <math.h>

#define INBOUNDS(TST, MIN, MAX) (((TST) >= (MIN)) && ((TST) <= (MAX)))


bool 
CESP32_NVStorage::validatedLoad(const char* key, char* val, int maxlen, const char* defVal)
{
  bool retval = true;
  if(!preferences.hasString(key)) {
    preferences.putString(key, defVal);
    DebugPort.printf("CESP32HeaterStorage::validatedLoad<char*> default installed %s=%s\r\n", key, defVal);
  }
  preferences.getString(key, val, maxlen);
  val[maxlen] = 0;  // ensure null terminated
  return retval;
}

bool 
CESP32_NVStorage::validatedLoad(const char* key, uint8_t* val, int len)
{
  bool retval = true;
  if(!preferences.hasBytes(key)) {
    DebugPort.printf("CESP32HeaterStorage::validatedLoad<uint8_t*> default installed for %s\r\n", key);
    preferences.putBytes(key, val, len);
  }
  len = preferences.getBytes(key, val, len);
  if(len == 0) {
    retval = false;
  }
  return retval;
}

bool
CESP32_NVStorage::validatedLoad(const char* key, uint8_t& val, uint8_t defVal, std::function<bool(uint8_t, uint8_t, uint8_t)> validator, uint8_t min, uint8_t max, uint8_t mask)
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
CESP32_NVStorage::validatedLoad(const char* key, int8_t& val, int8_t defVal, std::function<bool(int8_t, int8_t, int8_t)> validator, int8_t min, int8_t max)
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
CESP32_NVStorage::validatedLoad(const char* key, uint16_t& val, uint16_t defVal, std::function<bool(uint16_t, uint16_t, uint16_t)> validator, uint16_t min, uint16_t max)
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
CESP32_NVStorage::validatedLoad(const char* key, long& val, long defVal, long min, long max)
{
  val = preferences.getLong(key, defVal);
  if(!INBOUNDS(val, min, max)) {

    DebugPort.printf("CESP32HeaterStorage::validatedLoad<long> invalid read %s=%ld", key, val);
    DebugPort.printf(" validator(%ld,%ld) reset to %ld\r\n", min, max, defVal);

    val = defVal;
    preferences.putLong(key, val);
    return false;
  }
  return true;
}

bool
CESP32_NVStorage::validatedLoad(const char* key, uint32_t& val, uint32_t defVal, uint32_t min, uint32_t max)
{
  val = preferences.getULong(key, defVal);
  if(!INBOUNDS(val, min, max)) {

    DebugPort.printf("CESP32HeaterStorage::validatedLoad<unsigned long> invalid read %s=%d", key, val);
    DebugPort.printf(" validator(%d,%d) reset to %d\r\n", min, max, defVal);

    val = defVal;
    preferences.putULong(key, val);
    return false;
  }
  return true;
}


bool
CESP32_NVStorage::validatedLoad(const char* key, float& val, float defVal, float min, float max)
{
// preferences.getFloat() does not do a default value for use
// use some skull duggery via unsigned long to get one installed
  unsigned long* pUL = (unsigned long*)&defVal;  // point to bytes of float default value as a long
  unsigned long ULVal = *pUL;     // copy as an unsigned long

  unsigned long tmpVal = preferences.getULong(key, ULVal);

//  pUL = (unsigned long*)&val;   // point to val we exchange, as an usigned long
  float* ptmpVal = (float*)&tmpVal;  // create a pointer to flaot, that was our UL returned value
  float* pVal = &val;                // point to our FP exchange value
  *pVal = *ptmpVal;                  // copy as a float

//  val = preferences.getFloat(key, defVal);
  if(!INBOUNDS(val, min, max)) {

    DebugPort.printf("CESP32HeaterStorage::validatedLoad<float> invalid read %s=%f", key, val);
    DebugPort.printf(" validator(%f,%f) reset to %f\r\n", min, max, defVal);

    val = defVal;
//    preferences.putFloat(key, val);
    pUL = (unsigned long*)&val;  // point to bytes of float default value as a long
    ULVal = *pUL;     // copy as an unsigned long
    preferences.putULong(key, ULVal);
    return false;
  }
  return true;
}

size_t 
CESP32_NVStorage::saveFloat(const char* key, float val)
{
  unsigned long* pUL = (unsigned long*)&val;  // point to bytes of float default value as a long
  unsigned long ULVal = *pUL;     // copy as an unsigned long
  return preferences.putULong(key, ULVal);
}

bool finBounds(float test, float minLim, float maxLim)
{
  return INBOUNDS(test, minLim, maxLim);
}

bool u8inBounds(uint8_t test, uint8_t minLim, uint8_t maxLim)
{
  return INBOUNDS(test, minLim, maxLim);
}

bool u8inBoundsOrZero(uint8_t test, uint8_t minLim, uint8_t maxLim)
{
  return INBOUNDS(test, minLim, maxLim) || (test == 0);
}


bool s8inBounds(int8_t test, int8_t minLim, int8_t maxLim)
{
  return INBOUNDS(test, minLim, maxLim);
}

bool u8Match2(uint8_t test, uint8_t test1, uint8_t test2)
{
  return (test == test1) || (test == test2);
}

bool u16inBounds(uint16_t test, uint16_t minLim, uint16_t maxLim)
{
  return INBOUNDS(test, minLim, maxLim);
}

