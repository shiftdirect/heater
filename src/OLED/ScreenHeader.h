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

#include "Screen.h"

struct sScreenholdoff {
  int  holdon;
  int  holdoff;
  sScreenholdoff() {
    reset();
  }
  void reset() {
    holdon = 0;
    holdoff = 0;
  }
  void set(int hldon = 2, int hldoff = 8) {
    holdon = hldon;
    holdoff = hldoff;
  }
};

class CScreenHeader : public CScreen {
  sScreenholdoff _UpAnnotation;
  sScreenholdoff _DnAnnotation;
  bool _colon;
  bool _hdrDetail;
  uint8_t  _animateCount;
  uint8_t  _batteryCount;
  uint8_t  _batteryWarn;
protected:
  void showBTicon();
  void showWifiIcon();
  void showBatteryIcon(float voltage);
  int  showTimers();
  virtual void showTime();  // x location depends upon how many timers are active
  void showHeaderDetail(bool state) { _hdrDetail = state; };
public:
  CScreenHeader(C128x64_OLED& disp, CScreenManager& mgr); 
  bool show(bool erase);
  bool animate();
};

#endif // __SCREEN_HEADER_H__
