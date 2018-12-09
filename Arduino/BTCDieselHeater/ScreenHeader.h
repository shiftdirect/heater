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

#ifndef __SCREEN_HEADER_H__
#define __SCREEN_HEADER_H__

#include <Arduino.h>
#include "FontTypes.h"
#include "UtilClasses.h"
#include "Screen.h"


class CScreenHeader : public CScreen {
  bool _clearUpAnimation;
  bool _clearDnAnimation;
  bool _colon;
protected:
  void showBTicon();
  void showWifiIcon();
  void showBatteryIcon(float voltage);
  int  showTimers();
  virtual void showTime(int numTimers);  // x location depends upon how many timers are active
public:
  CScreenHeader(C128x64_OLED& disp, CScreenManager& mgr); 
  virtual void show();
  virtual bool animate();
};

#endif // __SCREEN_HEADER_H__
