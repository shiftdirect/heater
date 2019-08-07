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

CTempSense::CTempSense()
{
  _TempSensor = NULL;
  _owb = NULL;
}

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
/*void CTempSense::begin(int pin)
{
  // initialise DS18B20 sensor interface
  // create one wire bus interface, using RMT peripheral
  _owb = owb_rmt_initialize(&_rmt_driver_info, pin, RMT_CHANNEL_1, RMT_CHANNEL_0);
  owb_use_crc(_owb, true);  // enable CRC check for ROM code

  bool found = find();

  readROMcode();

  // Create DS18B20 device on the 1-Wire bus
  if(found) {
    attach();
  }
}
*/
bool
CTempSense::readTemperature(float& tempReading)
{
  if(_TempSensor == NULL) {

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

  if(_TempSensor != NULL) {
    DS18B20_ERROR error = ds18b20_read_temp(_TempSensor, &tempReading);
//          DebugPort.printf(">>>> DS18B20 = %f, error=%d\r\n", fTemperature, error);

    if(error == DS18B20_OK) {
      return true;
    }
    else {
      DebugPort.println("\007DS18B20 sensor removed?");
      ds18b20_free(&_TempSensor);
    }
  }

  return false;
}
/*bool
CTempSense::readTemperature(float& tempReading)
{
  if(_TempSensor == NULL) {

    bool found = find();

    if(found) {
      DebugPort.println("Found DS18B20 device");

      readROMcode();

      attach();

      startConvert();  // request a new conversion,
      waitConvertDone();
    }
  }

  if(_TempSensor != NULL) {
    DS18B20_ERROR error = ds18b20_read_temp(_TempSensor, &tempReading);
//          DebugPort.printf(">>>> DS18B20 = %f, error=%d\r\n", fTemperature, error);

    if(error == DS18B20_OK) {
      return true;
    }
    else {
      DebugPort.println("\007DS18B20 sensor removed?");
      ds18b20_free(&_TempSensor);
    }
  }

  return false;
}
*/
bool 
CTempSense::find()
{
  // Find all connected devices
//  DebugPort.printf("Finding one wire bus devices...");
  OneWireBus_SearchState search_state = {0};
  bool found = false;
  owb_search_first(_owb, &search_state, &found);
  if(found)
    DebugPort.println("Found a one wire device");
  else 
    DebugPort.println("No one wire devices found!!");

  return found;
}

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
  if(_TempSensor == NULL) {
    _TempSensor = ds18b20_malloc();  // heap allocation

    DebugPort.printf("Single device optimisations enabled\r\n");
    ds18b20_init_solo(_TempSensor, _owb);          // only one device on bus
    ds18b20_use_crc(_TempSensor, true);           // enable CRC check for temperature readings
    ds18b20_set_resolution(_TempSensor, DS18B20_RESOLUTION_12_BIT);
  }
  return true;
}

void 
CTempSense::startConvert()
{
  // kick off the initial temperature conversion
  if(_TempSensor)
    ds18b20_convert(_TempSensor);
}

void
CTempSense::waitConvertDone()
{
  if(_TempSensor)
    ds18b20_wait_for_conversion(_TempSensor);
}
