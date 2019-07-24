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

#ifndef __BTC_RTC_STORE_H__
#define __BTC_RTC_STORE_H__

#include <stdint.h>


class CRTC_Store {
  bool _accessed[4];  // [0] - bytes 0..3, [1] byte 4, [2] byte 5, [3] byte 6
  float _fuelgauge;
  uint8_t _demandDegC;
  uint8_t _demandPump;
  bool    _CyclicEngaged;
  bool    _bit6;
  uint8_t _RunTime;
  uint8_t _GlowTime;
  void    _ReadAndUnpackByte4();
  void    _PackAndSaveByte4();
  void    _ReadAndUnpackByte5();
  void    _PackAndSaveByte5();
  void    _ReadAndUnpackByte6();
  void    _PackAndSaveByte6();
public:
  CRTC_Store();
  void begin();
  void setFuelGauge(float val);
  void setDesiredTemp(uint8_t val);
  void setDesiredPump(uint8_t val);
  bool incRunTime();
  bool incGlowTime();
  void setCyclicEngaged(bool _CyclicEngaged);
  float getFuelGauge();
  uint8_t getDesiredTemp();
  uint8_t getDesiredPump();
  bool    getCyclicEngaged();
  int getRunTime();
  int getGlowTime();
  int getMaxGlowTime() const { return 8; };
  int getMaxRunTime() const { return 32; };
};

extern CRTC_Store RTC_Store;

#endif // __BTC_RTC_STORE_H__
