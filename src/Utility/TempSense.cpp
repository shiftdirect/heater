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

#include <Arduino.h>

#include "TempSense.h"
#include "DebugPort.h"
#include "macros.h"
#include "NVStorage.h"

CSensor::CSensor() 
{
  _reading = -100;
  _holdoff = 2;
  _filter.setBounds(-50, 80);
}

bool 
CSensor::update(float val)
{
  if(_holdoff) {
    _holdoff--;
    _reading = -100;
    if(_holdoff == 0)
      _filter.reset(val);
    return false;
  }
  else {
    _filter.update(val);
    _reading = val;
    return true;
  }
}

void
CSensor::reset()
{
  _holdoff = 2;
  _reading = -100;
  _filter.reset(_reading);
}

bool 
CSensor::getTemperature(float& tempReading, bool filtered)
{
  if(_holdoff) {
    tempReading = -100;
  }
  else {
    if(filtered)
      tempReading = _filter.getValue();
    else
      tempReading = _reading;
  }

  return _holdoff == 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DS18B20 probe support
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDS18B20probe::CDS18B20probe() : CSensor()
{
  pSensorInfo = NULL;
  init();
}

void
CDS18B20probe::init()
{
  release();
  reset();
  error = DS18B20_ERROR_UNKNOWN;
} 

void
CDS18B20probe::release()
{
  if(pSensorInfo)
    ds18b20_free(&pSensorInfo);
  pSensorInfo = NULL;
}

bool
CDS18B20probe::readSensor()
{
  bool retval = false;
  float temperature;

  error = ds18b20_read_temp(pSensorInfo, &temperature);   
  if(error == DS18B20_OK) {
    retval = update(temperature);
    if(!retval) {
      error = DS18B20_ERROR_DEVICE;  // holdoff still active, avoid allowing initial readings
    }
  }
  else {
    reset();
  }

  return retval;
}

float 
CDS18B20probe::getReading(bool filtered) 
{
  float temperature;
  CSensor::getTemperature(temperature, filtered);
  return temperature;
}

OneWireBus_ROMCode 
CDS18B20probe::getROMcode() const 
{ 
  if(pSensorInfo)
    return pSensorInfo->rom_code; 
  else {
    OneWireBus_ROMCode nullROM = {0};
    return nullROM;
  }
}

bool 
CDS18B20probe::matchROMcode(uint8_t test[8])
{
  if(pSensorInfo)
    return memcmp(pSensorInfo->rom_code.bytes, test, 8) == 0;
  return false;
}




CDS18B20SensorSet::CDS18B20SensorSet()
{
  _owb = NULL;
  _nNumSensors = 0;
  for(int i=0; i< MAX_DS18B20_DEVICES; i++)
    _Sensors[i].init();

  for(int i=0; i<3; i++)
  	_sensorMap[i] = -1;

  _bReportFind = true;
}

void 
CDS18B20SensorSet::begin(int pin)
{
  // initialise DS18B20 sensor interface

  // create one wire bus interface, using RMT peripheral
  _owb = owb_rmt_initialize(&_rmt_driver_info, pin, RMT_CHANNEL_1, RMT_CHANNEL_0);
  owb_use_crc(_owb, true);  // enable CRC check for ROM code

  _bReportFind = true;

  find();
}

bool
CDS18B20SensorSet::readSensors()
{
  bool retval = false;

  if(_nNumSensors == 0) {

    bool found = find();

    if(found) {
      DebugPort.println("Found DS18B20 device(s)");

      startConvert();  // request a new conversion,
      waitConvertDone();
    }
  }

  if(_nNumSensors) {
    for (int i = 0; i < MAX_DS18B20_DEVICES; ++i) {
      _Sensors[i].setError(DS18B20_ERROR_UNKNOWN);
    }

    for (int i = 0; i < _nNumSensors; ++i) {
      _Sensors[i].readSensor();
    }

#ifdef REPORT_READINGS
    DebugPort.println("\nTemperature readings (degrees C)");
#endif
    for (int i = 0; i < _nNumSensors; ++i) {
      if(_Sensors[i].OK()) {
#ifdef REPORT_READINGS
        DebugPort.printf("  %d: %.1f    OK\r\n", i, _Readings[i]);
#endif
        retval = true;  // at least one sensor read OK
      }
      else {
#ifdef REPORT_READINGS
        DebugPort.printf("\007  %d: DS18B20 sensor removed?\r\n", i);
#endif
      }
    }
  }

  return retval;
}

bool 
CDS18B20SensorSet::find()
{
  // Find all connected devices
  if(_bReportFind)
    DebugPort.println("Finding one wire bus devices...");
  OneWireBus_ROMCode rom_codes[MAX_DS18B20_DEVICES];
  memset(&rom_codes, 0, sizeof(rom_codes));

  _nNumSensors = 0;
  OneWireBus_SearchState search_state = {0};

  bool found = false;
  owb_search_first(_owb, &search_state, &found);
  while(found) {
    char rom_code_s[17];
    owb_string_from_rom_code(search_state.rom_code, rom_code_s, sizeof(rom_code_s));
    if(_bReportFind)
      DebugPort.printf("  %d : %s\r\n", _nNumSensors, rom_code_s);

    rom_codes[_nNumSensors] = search_state.rom_code;
    _nNumSensors++;
    owb_search_next(_owb, &search_state, &found);
  }
  if(_bReportFind)
    DebugPort.printf("Found %d DS18B20 device%s\r\n", _nNumSensors, _nNumSensors==1 ? "" : "s");

  // Create DS18B20 devices on the 1-Wire bus
  for (int i = 0; i < MAX_DS18B20_DEVICES; ++i) {
    _Sensors[i].release();
  }
  for (int i = 0; i < _nNumSensors; ++i)
  {
    DS18B20_Info * ds18b20_info = ds18b20_malloc();  // heap allocation
    _Sensors[i].assign(ds18b20_info);

    if (_nNumSensors == 1)
    {
      DebugPort.print("DS18B20 Single device optimisations enabled\n");
      ds18b20_init_solo(ds18b20_info, _owb);          // only one device on bus
      ds18b20_info->rom_code = rom_codes[0];  // added, for GUI setup!!
    }
    else
    {
      ds18b20_init(ds18b20_info, _owb, rom_codes[i]); // associate with bus and device
    }
    ds18b20_use_crc(ds18b20_info, true);           // enable CRC check for temperature readings
    ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION_12_BIT);
  }

  _bReportFind = false;

  return found;
}

void 
CDS18B20SensorSet::startConvert()
{
  // kick off the initial temperature conversion
  if(_Sensors[0].getSensorInfo())
    ds18b20_convert_all(_owb);
}

void
CDS18B20SensorSet::waitConvertDone()
{
  if(_Sensors[0].getSensorInfo())
    ds18b20_wait_for_conversion(_Sensors[0].getSensorInfo());
}

int 
CDS18B20SensorSet::checkNumSensors() const
{
  long start = millis();
  bool found = false;
  int numSensors = 0;
  OneWireBus_SearchState search_state = {0};
  owb_search_first(_owb, &search_state, &found);
  while(found) {
    numSensors++;
    owb_search_next(_owb, &search_state, &found);
  }
  DebugPort.printf("Found %d one-wire device%s\r\n", numSensors, numSensors==1 ? "" : "s");
  long tDelta = millis() - start;
  DebugPort.printf("checkNumSensors: %ldms\r\n", tDelta);
  return numSensors;
}

bool 
CDS18B20SensorSet::mapSensor(int idx, OneWireBus_ROMCode romCode)
{
  if(idx == -1) {
    _sensorMap[0] = _sensorMap[1] = _sensorMap[2] = -1;
    return false;
  }
  if(idx == -2) {
      DebugPort.printf("Sensor Map: %d %d %d\r\n",
                       _sensorMap[0], _sensorMap[1], _sensorMap[2]);
    return false;
  }

  if(!INBOUNDS(idx, 0, 2))
    return false;

  for(int i = 0; i < _nNumSensors; i++) {
    if(_Sensors[i].matchROMcode(romCode.bytes)) {
      _sensorMap[idx] = i;
      DebugPort.printf("Mapped DS18B20 %02X:%02X:%02X:%02X:%02X:%02X as role %d\r\n",
                        romCode.fields.serial_number[5], 
                        romCode.fields.serial_number[4], 
                        romCode.fields.serial_number[3], 
                        romCode.fields.serial_number[2], 
                        romCode.fields.serial_number[1], 
                        romCode.fields.serial_number[0], 
                        idx
                      );
      return true;
    }
  }
  return false;
}

bool
CDS18B20SensorSet::getTemperature(int usrIdx, float& temperature, bool filtered) 
{
  int snsIdx = _sensorMap[usrIdx];
  if(snsIdx < 0) 
    snsIdx = 0;  // default to sensor 0 if not mapped

  return getTemperatureIdx(snsIdx, temperature, filtered);  
}

bool
CDS18B20SensorSet::getTemperatureIdx(int snsIdx, float& temperature, bool filtered) 
{
  return _Sensors[snsIdx].getTemperature(temperature, filtered);
}

bool 
CDS18B20SensorSet::getRomCodeIdx(int snsIdx, OneWireBus_ROMCode& romCode) const
{
  if(snsIdx >= _nNumSensors)
    return false;
  romCode = _Sensors[snsIdx].getROMcode();
  return true;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// BME-280 probe support
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



CBME280Sensor::CBME280Sensor() : CSensor()
{
  _count = 0;
}

bool
CBME280Sensor::begin(int ID)
{
  _count = 0;
  bool status = _bme.begin(ID);  
  if (!status) {
    DebugPort.println("Could not find a valid BME280 sensor, check wiring!");
    return false;
  }
  _count = 1;
  _bme.setSampling(Adafruit_BME280::MODE_FORCED, 
                  Adafruit_BME280::SAMPLING_X1,
                  Adafruit_BME280::SAMPLING_X1,
                  Adafruit_BME280::SAMPLING_X1,
                  Adafruit_BME280::FILTER_OFF,
                  Adafruit_BME280::STANDBY_MS_1000);
  
  _lastSampleTime = millis();

  return true;
}

bool 
CBME280Sensor::getTemperature(float& tempReading, bool filtered) 
{
  if(_count == 0) {
    reset();
    CSensor::getTemperature(tempReading, filtered);
    return false;
  }

  long tDelta = millis() - _lastSampleTime;
  if(tDelta >= 0) {
    _bme.takeForcedMeasurement();
    float temperature = _bme.readTemperature();
    update(temperature);
    _lastSampleTime = millis() + 1000;
  }

  CSensor::getTemperature(tempReading, filtered);
  tempReading += NVstore.getHeaterTuning().BME280probe.offset;;

  return true;
}

const char* 
CBME280Sensor::getID()
{
  return "BME-280";
}


CTempSense::CTempSense()
{
}

void
CTempSense::begin(int oneWirePin, int I2CID)
{
  DS18B20.begin(oneWirePin);
  BME280.begin(I2CID);
}

int  
CTempSense::getNumSensors() const
{
  int retval = 0;

  retval += DS18B20.getNumSensors();
  retval += BME280.getCount();

  return retval;
}

void 
CTempSense::startConvert()
{
  DS18B20.startConvert();
}

bool
CTempSense::readSensors()
{
  return DS18B20.readSensors();
}

float
CTempSense::getOffset(int usrIdx) 
{
  switch(getSensorType(usrIdx)) {
    case 0:  // BME280
      return NVstore.getHeaterTuning().BME280probe.offset;
    case 2:  // DS18B20 - AFTER BME280 - bump index down
      usrIdx--;
    case 1:  // DS18B20
      if(INBOUNDS(usrIdx, 0, 2)) {
        return NVstore.getHeaterTuning().DS18B20probe[usrIdx].offset;
      }
      break;
  }
  return 0;

}

void 
CTempSense::setOffset(int usrIdx, float offset)
{
  if(!INBOUNDS(offset, -10.0, +10.0)) {
    return;
  }

  sHeaterTuning ht = NVstore.getHeaterTuning();

  if(BME280.getCount() == 0) {
    // no BME280 present - simply apply to DS18B20 list
    if(INBOUNDS(usrIdx, 0, 2))
      ht.DS18B20probe[usrIdx].offset = offset;
  }
  else {
    // BME280 present
    // need to change behvaiour depending if the BME is primary
    if(NVstore.getHeaterTuning().BME280probe.bPrimary) {
      // BME is primary - usrIdx 0 is BME, 1 is first DS18B20
      if(usrIdx == 0) {
        ht.BME280probe.offset = offset;
      }
      else {
        usrIdx--;
        if(INBOUNDS(usrIdx, 0, 2))
          ht.DS18B20probe[usrIdx].offset = offset;
      }
    }
    else {
      // BME is after DS18B20s
      // note the index for the BME depends upon how many DS18B20s are connected!
      if(usrIdx >= DS18B20.getNumSensors()) {
        // assume any more than connected DS18B20 is the BME
        ht.BME280probe.offset = offset;
      }
      else {
        if(INBOUNDS(usrIdx, 0, 2))
          ht.DS18B20probe[usrIdx].offset = offset;
      }
    }
  }

  NVstore.setHeaterTuning(ht);
}


bool
CTempSense::getTemperature(int usrIdx, float& temperature, bool filtered) 
{
  switch(getSensorType(usrIdx)) {
    case 0:
      return BME280.getTemperature(temperature, filtered);
    case 1:
      return DS18B20.getTemperature(usrIdx, temperature, filtered);  
    case 2:
      return DS18B20.getTemperature(usrIdx-1, temperature, filtered);  
    default:
      return false;
  }
}

int 
CTempSense::getSensorType(int usrIdx)
{
  if(BME280.getCount() == 0)
    return 1;  

  if(NVstore.getHeaterTuning().BME280probe.bPrimary) {
    if(usrIdx == 0)
      return 0;
    return 2;  
  }
  else {
    if(usrIdx >= DS18B20.getNumSensors()) {
      return 0;
    }
    return 1;  
  }
}

const char*
CTempSense::getID(int usrIdx)
{
  if(getSensorType(usrIdx))
    return "DS18B20";
  else 
    return "BME280";
}

void
CTempSense::format(char* msg, float fTemp) 
{
  if(NVstore.getUserSettings().degF) {
    fTemp = fTemp * 9 / 5 + 32;
    sprintf(msg, "%.1f`F", fTemp);
  }
  else {
    sprintf(msg, "%.1f`C", fTemp);
  }
}
