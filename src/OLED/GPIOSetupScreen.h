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

#ifndef __GPIOSETUPSCREEN_H__
#define __GPIOSETUPSCREEN_H__

#include <stdint.h>
#include "UIEditScreen.h"
#include "../Utility/BTC_GPIO.h"

class C128x64_OLED;
class CScreenManager;

class CGPIOSetupScreen : public CUIEditScreen
{
  void _adjust(int dir);
  sGPIOparams _GPIOparams;
  unsigned long _ExtHold;
  int _scrollChar;
  int _repeatCount;
protected:
  void _saveNV();
public:
  CGPIOSetupScreen(C128x64_OLED& display, CScreenManager& mgr);
  bool show();
  bool animate();
  bool keyHandler(uint8_t event);
  bool onSelect();
};

#endif
