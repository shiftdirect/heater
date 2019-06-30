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
#include "Debounce.h"

CDebounce::CDebounce()
{
  _pinActiveState = LOW;
  _prevPins = 0;
  _lastDebounceTime = 0;
  _debounceDelay = 50;
}

void 
CDebounce::addPin(int pin)
{
  if(pin && (_pins.size() < 8)) {
    _pins.push_back(pin);
    pinMode(pin, INPUT_PULLUP);   // GPIO input pin #1
  }
}

void
CDebounce::setActiveState(int activeState)
{
  _pinActiveState = activeState;
}

uint8_t 
CDebounce::getState() 
{ 
  return _debouncedPins; 
}

uint8_t 
CDebounce::manage() 
{
  return _scanInputs();
}

uint8_t 
CDebounce::_scanInputs()
{
  uint8_t newPins = 0;
  uint8_t mask = 0x01;
  for(int i = 0; i < _pins.size(); i++) {
    if(digitalRead(_pins[i]) == _pinActiveState) 
      newPins |= mask;
    mask <<= 1;
  }

  if(newPins != _prevPins) {
    _lastDebounceTime = millis();
    _prevPins = newPins;
  }

  long elapsed = millis() - _lastDebounceTime;
  if (elapsed > _debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    _debouncedPins = newPins;
  }

  return _debouncedPins;
}


