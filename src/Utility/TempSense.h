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
#include "../../lib/Adafruit_BME280_Library/Adafruit_BME280.h"
#include "DataFilter.h"

//#define SINGLE_DS18B20_SENSOR

const int MAX_DS18B20_DEVICES = 3;

class CSensor {
  float _reading;
  CExpMean _filter;
  int _holdoff;
protected:
  bool update(float val);
  void reset();
public:
  CSensor();
  bool getTemperature(float& tempReading, bool filtered);
  const char* getID();
};

class CDS18B20probe : public CSensor {
  DS18B20_Info * pSensorInfo;
  DS18B20_ERROR error;
public:
  CDS18B20probe();
  void init();
  void assign(DS18B20_Info *pInfo) { pSensorInfo = pInfo; };
  void release();
  bool readSensor();
  void setError(DS18B20_ERROR err) { error = err; };
  bool OK() { return error == DS18B20_OK; };
  OneWireBus_ROMCode getROMcode() const;
  DS18B20_Info* getSensorInfo() { return pSensorInfo; };
  float getReading(bool filtered);
  bool matchROMcode(uint8_t test[8]);
};

class CDS18B20SensorSet  {
  OneWireBus * _owb;
  owb_rmt_driver_info _rmt_driver_info;

  CDS18B20probe _Sensors[MAX_DS18B20_DEVICES];
  int _nNumSensors;
  bool _bReportFind;
  int _sensorMap[MAX_DS18B20_DEVICES];
  bool _discover();

public:
  CDS18B20SensorSet();
  void begin(int pin);
  bool find();
  bool readSensors();
  void startConvert();
  void waitConvertDone();
  int  checkNumSensors() const;
  bool getTemperature(int mapIdx, float& tempReading, bool filtered);
  bool getTemperatureIdx(int sensIdx, float& tempReading, bool filtered) ;      // index is sensor discovery order on one-wire bus
  bool getRomCodeIdx(int sensIdx, OneWireBus_ROMCode& romCode) const; // index is sensor discovery order on one-wire bus
  bool mapSensor(int idx, OneWireBus_ROMCode romCode = { 0 } );
  int  getNumSensors() const { return _nNumSensors; };
  const char* getID();
};

class CBME280Sensor : public CSensor {
  Adafruit_BME280 _bme; // I2C
  long _lastSampleTime;
  float _fAltitude;
  float _fHumidity;
  int _count;
public:
  CBME280Sensor();
  bool begin(int ID);
  bool getTemperature(float& tempReading, bool filtered) ;
  bool getAltitude(float& reading, bool fresh=false);
  bool getHumidity(float& reading, bool fresh=false);
  int getAllReadings(bme280_readings& readings);
  const char* getID();
  int getCount() const { return _count; };
};

class CTempSense {

  CDS18B20SensorSet DS18B20;
  CBME280Sensor BME280;
  
  bool _discover();
public:
  CTempSense();
  void begin(int oneWirePin, int I2CID);
  bool readSensors();
  void startConvert();
  int getSensorType(int usrIdx);
  const char* getID(int usrIdx);
  bool getTemperature(int usrIdx, float& tempReading, bool filtered=true) ;   // indexed as mapped by user
  float getOffset(int usrIdx);
  void setOffset(int usrIdx, float offset);
  bool getTemperatureBME280(float& tempReading) ;      // index is sensor discovery order on one-wire bus
  bool getTemperatureDS18B20Idx(int sensIdx, float& tempReading) ;      // index is sensor discovery order on one-wire bus
  int  getNumSensors() const;
  bool getAltitude(float& reading, bool fresh=false);
  bool getHumidity(float& reading, bool fresh=false);
  CBME280Sensor& getBME280() { return BME280; };
  CDS18B20SensorSet& getDS18B20() { return DS18B20; };
  static void format(char* msg, float fTemp);
};

#endif
