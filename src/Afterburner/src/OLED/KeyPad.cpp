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
#include "keypad.h"
#include "../cfg/pins.h"

CKeyPad::CKeyPad()
{
  // handler
  _lastKey = 0;
  _lastHoldTime = 0;
  _holdTimeout = 0;
  _keyCallback = NULL;
}

void
CKeyPad::begin(int Lkey, int Rkey, int Ckey, int Ukey, int Dkey)
{
  _Debounce.addPin(Lkey);
  _Debounce.addPin(Rkey);
  _Debounce.addPin(Ckey);
  _Debounce.addPin(Ukey);
  _Debounce.addPin(Dkey);
}

void
CKeyPad::setCallback(void (*callback)(uint8_t event))
{
  _keyCallback = callback;
}

uint8_t
CKeyPad::update()
{
  uint8_t newKey = _Debounce.manage();

  // determine edge events
  uint8_t keyChange = newKey ^ _lastKey;
  uint8_t Press = keyChange & newKey;     // bits set upon intial press, ONLY
  uint8_t Release = keyChange & ~newKey;  // bits set upon intial release, ONLY
  uint8_t Repeat = 0;  

  _lastKey = newKey;

  if(Press) {
#ifdef DBG_KEYPAD
    DebugPort.println("PRESS");
#endif
    _lastHoldTime = millis();
    _holdTimeout = 350;                 // initial hold delay
  }

  if(Release) {
#ifdef DBG_KEYPAD
    DebugPort.println("RELEASE");
#endif
    _holdTimeout = 0;                   // cancel repeat
  }

  long tDelta = millis() - _lastHoldTime;
  if(_holdTimeout && (tDelta > _holdTimeout)) {
#ifdef DBG_KEYPAD
    DebugPort.println("REPEAT");
#endif
    _holdTimeout = 150;                 // repeat delay
    _lastHoldTime += _holdTimeout;
    Repeat = newKey;
  }

  if(Press) {
    if(_keyCallback != NULL) 
      _keyCallback(keyPressed | Press | newKey);
    return keyPressed | Press;
  }

  if(Release) {
    if(_keyCallback != NULL) 
      _keyCallback(keyReleased | Release);
    return (keyReleased | Release);
  }

  if(Repeat) {
    if(_keyCallback != NULL) 
      _keyCallback(keyRepeat | Repeat);
    return (keyRepeat | Repeat);
  }

  return 0;
}
