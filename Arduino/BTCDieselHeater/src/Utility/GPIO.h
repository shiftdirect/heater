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

#include <stdint.h>

enum GPIOinModes { 
  GPIOinNone, 
  GPIOinOn1Off2,   // input 1 closure, heater starts; input2 closure, heater stops
  GPIOinOnHold1,   // hold input 1 closure, heater runs; input 1 open, heater stops
  GPIOinOn1Off1    // alternate input 1 closures start or stop the heater 
};

class CGPIOin {
  GPIOinModes _Mode;
  void _doOn1Off2();
  void _doOnHold1();
  void _doOn1Off1();
  int _pins[2];
  uint8_t _scanInputs();
  uint8_t _prevPins;
  uint8_t _debouncedPins;
  uint8_t _lastKey;
  unsigned long _lastDebounceTime;
  unsigned long _debounceDelay;
public:
  CGPIOin();
  void setMode(GPIOinModes mode) { _Mode = mode; };
  void begin(int pin1, int pin2, GPIOinModes mode);
  void manageGPIO();
};