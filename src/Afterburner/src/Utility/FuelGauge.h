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

#ifndef BTC_FUELGAUGE_H_
#define BTC_FUELGAUGE_H_

#include <stdint.h>

class CFuelGauge {
  float _pumpStrokes;
  unsigned long _lasttime;
  float _pumpCal;
  float _lastStoredVal;
public:
  CFuelGauge();
  void init(float fuelUsed = 0);
  void reset();
  void Integrate(float Hz);
  float Used_ml();
  float Used_strokes();
};

extern CFuelGauge FuelGauge;

#endif
