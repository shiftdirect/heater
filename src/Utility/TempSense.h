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

#ifndef __BTC_TEMPSENSE_H__
#define __BTC_TEMPSENSE_H__

#include "../../lib/esp32-ds18b20/ds18b20.h"

class CTempSense {

  OneWireBus * _owb;
  owb_rmt_driver_info _rmt_driver_info;
  DS18B20_Info * _TempSensor = NULL;
  
  bool _discover();
public:
  CTempSense();
  void begin(int pin);
  bool find();
  bool readROMcode();
  bool attach();
  bool readTemperature(float& tempReading);
  void startConvert();
  void waitConvertDone();
};

#endif
