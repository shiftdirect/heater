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

#ifndef __DEBOUNCE_H__
#define __DEBOUNCE_H__

#include <stdint.h>
#include <vector>

class CDebounce {
  int _pinActiveState;
  std::vector<int> _pins;
  uint8_t _prevPins;
  uint8_t _debouncedPins;
  unsigned long _lastDebounceTime;
  unsigned long _debounceDelay;
  uint8_t _scanInputs();
public:
  CDebounce();
  void addPin(int pin);
  void setActiveState(int state);
  uint8_t manage();
  uint8_t getState();
};

#endif
