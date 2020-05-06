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

#include "Protocol.h"

class CSmartError {
  uint8_t _prevRunState;
  uint8_t _prevPumpHz;
  uint8_t _Error;
  bool _bInhibit;
  void _monitorPriming(uint8_t runState, uint8_t pumpHz);
  void _monitor(uint8_t runstate);
public: 
  CSmartError();
  void reset();
  void inhibit(bool reseterror=false);
  void monitor(const CProtocol& heaterFrame);
  int checkVolts(float volts, float plugI, bool throwfault=true);  // 0 = OK, 1 = within 0.5V of LVC, 2 = under LVC
  int checkfuelUsage(bool throwfault=true);
  uint8_t getError();
};

extern CSmartError SmartError;
