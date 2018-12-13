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

#include <ArduinoJson.h>
#include "BTC_JSON.h"
#include "DebugPort.h"
#include "helpers.h"


char defaultJSONstr[64];

void interpretJsonCommand(char* pLine)
{
  if(strlen(pLine) == 0)
    return;

  DebugPort.print("JSON parse... "); DebugPort.print(pLine);

  StaticJsonBuffer<512> jsonBuffer;   // create a JSON buffer on the heap
	JsonObject& obj = jsonBuffer.parseObject(pLine);
	if(!obj.success()) {
		DebugPort.println(" FAILED");
		return;
	}
	DebugPort.println(" OK"); 

	JsonObject::iterator it;
	for(it = obj.begin(); it != obj.end(); ++it) {

		if(strcmp("DesiredTemp", it->key) == 0) {
      reqTemp(it->value.as<unsigned char>());
		}
		else if(strcmp("RunState", it->key) == 0) {
			if(it->value.as<unsigned char>()) {
	      requestOn();
			}
			else {
	      requestOff();
			}
		}
		else if(strcmp("PumpMin", it->key) == 0) {
			setPumpMin(it->value.as<float>());
		}
		else if(strcmp("PumpMax", it->key) == 0) {
			setPumpMax(it->value.as<float>());
		}
		else if(strcmp("FanMin", it->key) == 0) {
			setFanMin(it->value.as<short>());
		}
		else if(strcmp("FanMax", it->key) == 0) {
			setFanMax(it->value.as<short>());
		}
		else if(strcmp("Thermostat", it->key) == 0) {
			setThermostatMode(it->value.as<unsigned char>());
		}
	}
}

const char* createJSON(const char* name, float value, char* jsonToSend)
{
  StaticJsonBuffer<64> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();  // create object to add JSON commands to

	root.set(name, value);
	root.printTo(defaultJSONstr);
  strcat(defaultJSONstr, "\n");

  return defaultJSONstr;
}

const char* createJSON(const char* name, unsigned char value, char*jsonToSend)
{
  StaticJsonBuffer<64> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();  // create object to add JSON commands to

	root.set(name, value);
	root.printTo(defaultJSONstr);
  strcat(defaultJSONstr, "\n");

  return defaultJSONstr;
}

const char* createJSON(const char* name, int value, char* jsonToSend)
{
  StaticJsonBuffer<64> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();  // create object to add JSON commands to

	root.set(name, value);
	root.printTo(defaultJSONstr);
  strcat(defaultJSONstr, "\n");

  return defaultJSONstr;
}
