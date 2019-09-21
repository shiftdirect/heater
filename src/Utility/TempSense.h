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

//#define SINGLE_DS18B20_SENSOR

const int MAX_DS18B20_DEVICES = 3;

class CTempSense {

  OneWireBus * _owb;
  owb_rmt_driver_info _rmt_driver_info;
  DS18B20_Info * _Sensors[MAX_DS18B20_DEVICES];
  OneWireBus_ROMCode _device_rom_codes[MAX_DS18B20_DEVICES];
  int _nNumSensors;

  float _Readings[MAX_DS18B20_DEVICES];
  DS18B20_ERROR _Errors[MAX_DS18B20_DEVICES];
  
  bool _discover();
  int _sensorMap[3];
public:
  CTempSense();
  void begin(int pin);
  bool find();
#ifdef SINGLE_DS18B20_SENSOR
  bool readROMcode();
  bool attach();
#endif
  bool readSensors();
  void startConvert();
  void waitConvertDone();
  bool getTemperature(float& tempReading, int mapIdx=0);   // indexed as mapped by user
  bool getTemperatureIdx(float& tempReading, int selSensor=0);      // index is discovery order on one-wire bus
  bool getRomCodeIdx(OneWireBus_ROMCode& romCode, int selSensor=0); // index is discovery order on one-wire bus
  int  checkNumSensors() const;
  int  getNumSensors() const { return _nNumSensors; };
  bool mapSensor(int idx, OneWireBus_ROMCode romCode = { 0 } );
};

#endif
