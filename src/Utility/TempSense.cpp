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

CTempSense::CTempSense()
{
  _owb = NULL;
  _nNumSensors = 0;
  for(int i=0; i< MAX_DS18B20_DEVICES; i++)
    _Sensors[i] = NULL;
  for(int i=0; i<3; i++)
  	_sensorMap[i] = -1;
}

#ifdef SINGLE_DS18B20_SENSOR
void CTempSense::begin(int pin)
{
  // initialise DS18B20 sensor interface
  // create one wire bus interface, using RMT peripheral
  pinMode(pin, INPUT_PULLUP);
  _owb = owb_rmt_initialize(&_rmt_driver_info, pin, RMT_CHANNEL_1, RMT_CHANNEL_0);
  owb_use_crc(_owb, true);  // enable CRC check for ROM code

//  bool found = find();
  delay(10);  // couple of glitches take place during pin setup - give a bit of space to avoid no sensor issues?

  bool found = readROMcode();

  // Create DS18B20 device on the 1-Wire bus
  if(found) {
    attach();
  }
}
#else
void CTempSense::begin(int pin)
{
  // initialise DS18B20 sensor interface

  // create one wire bus interface, using RMT peripheral
  _owb = owb_rmt_initialize(&_rmt_driver_info, pin, RMT_CHANNEL_1, RMT_CHANNEL_0);
  owb_use_crc(_owb, true);  // enable CRC check for ROM code

  find();
}
#endif

#ifdef SINGLE_DS18B20_SENSOR
bool
CTempSense::readSensors(float& tempReading)
{
  if(_Sensors[0] == NULL) {

//    bool found = find();
    bool found = readROMcode();

    if(found) {
      DebugPort.println("Found DS18B20 device");

//      readROMcode();

      attach();

      startConvert();  // request a new conversion,
      waitConvertDone();
    }
  }

  if(_Sensors[0] != NULL) {
    DS18B20_ERROR error = ds18b20_read_temp(_Sensors[0], &tempReading);
//          DebugPort.printf(">>>> DS18B20 = %f, error=%d\r\n", fTemperature, error);

    if(error == DS18B20_OK) {
      return true;
    }
    else {
      DebugPort.println("\007DS18B20 sensor removed?");
      ds18b20_free(&_Sensors[0]);
    }
  }

  return false;
}
#else
bool
CTempSense::readSensors()
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
      _Errors[i] = DS18B20_ERROR_UNKNOWN;
    }

    for (int i = 0; i < _nNumSensors; ++i) {
      _Errors[i] = ds18b20_read_temp(_Sensors[i], &_Readings[i]);   
    }

#ifdef REPORT_READINGS
    DebugPort.println("\nTemperature readings (degrees C)");
#endif
    for (int i = 0; i < _nNumSensors; ++i) {
      if(_Errors[i] == DS18B20_OK) {
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
#endif

#ifdef SINGLE_DS18B20_SENSOR
bool 
CTempSense::find()
{
  // Find all connected devices
//  DebugPort.printf("Finding one wire bus devices...");
  _nNumSensors = 0;
  OneWireBus_SearchState search_state = {0};
  bool found = false;
  owb_search_first(_owb, &search_state, &found);
  if(found) {
    _nNumSensors = 1;
    DebugPort.println("Found a one wire device");
  }
  else 
    DebugPort.println("No one wire devices found!!");

  return found;
}
#else
bool 
CTempSense::find()
{
  // Find all connected devices
  DebugPort.println("Finding one wire bus devices...");
  memset(_device_rom_codes, 0, sizeof(_device_rom_codes));
  _nNumSensors = 0;
  OneWireBus_SearchState search_state = {0};

  bool found = false;
  owb_search_first(_owb, &search_state, &found);
  while(found) {
    char rom_code_s[17];
    owb_string_from_rom_code(search_state.rom_code, rom_code_s, sizeof(rom_code_s));
    DebugPort.printf("  %d : %s\r\n", _nNumSensors, rom_code_s);

    _device_rom_codes[_nNumSensors] = search_state.rom_code;
    _nNumSensors++;
    owb_search_next(_owb, &search_state, &found);
  }
  DebugPort.printf("Found %d device%s\r\n", _nNumSensors, _nNumSensors==1 ? "" : "s");

  // Create DS18B20 devices on the 1-Wire bus
  for (int i = 0; i < MAX_DS18B20_DEVICES; ++i) {
    if(_Sensors[i]) {
      ds18b20_free(&_Sensors[i]);
    }
    _Sensors[i] = NULL;
  }
  for (int i = 0; i < _nNumSensors; ++i)
  {
    DS18B20_Info * ds18b20_info = ds18b20_malloc();  // heap allocation
    _Sensors[i] = ds18b20_info;

    if (_nNumSensors == 1)
    {
        printf("Single device optimisations enabled\n");
        ds18b20_init_solo(ds18b20_info, _owb);          // only one device on bus
        ds18b20_info->rom_code = _device_rom_codes[i];  // added, for GUI setup!!
    }
    else
    {
        ds18b20_init(ds18b20_info, _owb, _device_rom_codes[i]); // associate with bus and device
    }
    ds18b20_use_crc(ds18b20_info, true);           // enable CRC check for temperature readings
    ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION_12_BIT);
  }

  return found;
}
#endif

#ifdef SINGLE_DS18B20_SENSOR
bool 
CTempSense::readROMcode()
{
  // For a single device only:
  OneWireBus_ROMCode rom_code;
  owb_status status = owb_read_rom(_owb, &rom_code);
  if (status == OWB_STATUS_OK)
  {
    char rom_code_s[OWB_ROM_CODE_STRING_LENGTH];
    owb_string_from_rom_code(rom_code, rom_code_s, sizeof(rom_code_s));
    DebugPort.printf("Device %s present\r\n", rom_code_s);
    return true;
  }
  else
  {
    DebugPort.printf("Error #%d occurred attempting to read ROM code\r\n", status);
    return false;
  }
}

bool
CTempSense::attach()
{
  if(_Sensors[0] == NULL) {
    _Sensors[0] = ds18b20_malloc();  // heap allocation

    DebugPort.printf("Single device optimisations enabled\r\n");
    ds18b20_init_solo(_Sensors[0], _owb);          // only one device on bus
    ds18b20_use_crc(_Sensors[0], true);           // enable CRC check for temperature readings
    ds18b20_set_resolution(_Sensors[0], DS18B20_RESOLUTION_12_BIT);
  }
  return true;
}
#endif

void 
CTempSense::startConvert()
{
  // kick off the initial temperature conversion
  if(_Sensors[0])
    ds18b20_convert_all(_owb);
}

void
CTempSense::waitConvertDone()
{
  if(_Sensors[0])
    ds18b20_wait_for_conversion(_Sensors[0]);
}

int 
CTempSense::checkNumSensors() const
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
CTempSense::mapSensor(int idx, OneWireBus_ROMCode romCode)
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
    if(memcmp(_Sensors[i]->rom_code.bytes, romCode.bytes, 8) == 0) {
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
CTempSense::getTemperature(int mapIdx, float& temperature)
{
  int snsIdx = _sensorMap[mapIdx];
  if(snsIdx < 0) 
    snsIdx = 0;  // default to sensor 0 if not mapped

  return getTemperatureIdx(snsIdx, temperature);  
}

bool
CTempSense::getTemperatureIdx(int snsIdx, float& temperature)
{
  temperature = _Readings[snsIdx];
  return _Errors[snsIdx] == DS18B20_OK;
}

bool 
CTempSense::getRomCodeIdx(int snsIdx, OneWireBus_ROMCode& romCode)
{
  if(snsIdx >= _nNumSensors)
    return false;
  romCode = _Sensors[snsIdx]->rom_code;
  return true;
}

