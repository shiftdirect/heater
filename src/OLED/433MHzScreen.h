/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2020  Ray Jones <ray@mrjones.id.au>
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

#ifndef __433MHZSCREEN_H__
#define __433MHZSCREEN_H__

#include <stdint.h>
#include "UIEditScreen.h"
#include "../RTC/BTCDateTime.h"

class C128x64_OLED;
class CScreenManager;
class CProtocol;

class C433MHzScreen : public CUIEditScreen {
  void _initUI();
  // void _decode(int idx);
  // int  _encode(int idx);
  void _saveNV();
  unsigned long _code;
  // unsigned long _savedCodes[3];
  unsigned long _rawCodes[3][4];
  unsigned long _ID;
  uint8_t _defined;
  uint8_t _keyCode;
  uint8_t _repeatCount;
public:
  C433MHzScreen(C128x64_OLED& display, CScreenManager& mgr);
  bool onSelect();
  void onExit();
  bool show();
  bool animate();
  bool keyHandler(uint8_t event);
};

#endif
