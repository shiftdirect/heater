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

#include "GPIO.h"
#include "../Protocol/helpers.h"

CGPIOin::CGPIOin()
{
  _Mode = GPIOinNone;
  _pins[0] = 0;
  _pins[1] = 0;
  _prevPins = 0;
  _lastDebounceTime = 0;
  _lastKey = 0;
  _debounceDelay = 50;
}

void 
CGPIOin::begin(int pin1, int pin2, GPIOinModes mode)
{
  _pins[0] = pin1;
  _pins[1] = pin2;
  pinMode(pin1, INPUT_PULLUP);   // GPIO input pin #1
  pinMode(pin2, INPUT_PULLUP);   // GPIO input pin #1

  setMode(mode);
}

void 
CGPIOin::manageGPIO() 
{
  switch (_Mode) {
    case GPIOinNone:
      break;
    case GPIOinOn1Off2:
      _doOn1Off2();
      break;
    case GPIOinOnHold1:
      _doOnHold1();
      break;
    case GPIOinOn1Off1:
      _doOn1Off1();
      break;
  }
}

void 
CGPIOin::_doOn1Off2()
{
  uint8_t newKey = _scanInputs();
  // determine edge events
  uint8_t keyChange = newKey ^ _lastKey;
  _lastKey = newKey;

  if(keyChange) {
    if(newKey & 0x01) {
      requestOn();
    }
    if(newKey & 0x02) {
      requestOff();
    }
  }
}

// mode where heater runs if input 1 is shorted
// stops when open
void 
CGPIOin::_doOnHold1()
{
  uint8_t newKey = _scanInputs();
  // determine edge events
  uint8_t keyChange = newKey ^ _lastKey;
  _lastKey = newKey;

  if(keyChange) {
    if(newKey & 0x01) {
      requestOn();
    }
    else {
      requestOff();
    }
  }
}

// mode where heater runs if input 1 is shorted
// stops when open
void 
CGPIOin::_doOn1Off1()
{
  uint8_t newKey = _scanInputs();
  // determine edge events
  uint8_t keyChange = newKey ^ _lastKey;
  _lastKey = newKey;

  if(keyChange) {
    if(newKey & 0x01) {
      if(getHeaterInfo().getRunStateEx())
        requestOff();
      else 
        requestOn();
    }
  }
}

  
uint8_t 
CGPIOin::_scanInputs()
{
  uint8_t newPins = 0;
  if(_pins[0] && digitalRead(_pins[0]) == HIGH) newPins |= 0x01;
  if(_pins[1] && digitalRead(_pins[1]) == HIGH) newPins |= 0x02;

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
