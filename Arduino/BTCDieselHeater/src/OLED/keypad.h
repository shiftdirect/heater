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

#ifndef __BTC_KEYPAD_H__
#define __BTC_KEYPAD_H__

#include <stdint.h>


const uint8_t key_Left     = 0x01;
const uint8_t key_Right    = 0x02;
const uint8_t key_Centre   = 0x04;
const uint8_t key_Up       = 0x08;
const uint8_t key_Down     = 0x10;
const uint8_t keyPressed   = 0x20;   // action flag
const uint8_t keyReleased  = 0x40;   // action flag
const uint8_t keyRepeat    = 0x80;   // action flag

class CKeyPad {
private:
  void (*_keyCallback)(uint8_t event);
  uint8_t _pins[5];  //0:L, 1:R, 2:C, 3:U, 4:D
  // pin scanning usage
  uint8_t _prevPins;
  uint8_t _debouncedPins;
  // handler usage
  uint8_t _lastKey;
  unsigned long _lastHoldTime;
  unsigned long _holdTimeout;
  uint8_t scanPins();
  unsigned long _lastDebounceTime;
  unsigned long _debounceDelay;
public:
  CKeyPad();
  void begin(int Lkey, int Rkey, int Ckey, int Ukey, int Dkey);
  uint8_t update();
	void setCallback(void (*Callback)(uint8_t event));
};

extern CKeyPad KeyPad;

#endif