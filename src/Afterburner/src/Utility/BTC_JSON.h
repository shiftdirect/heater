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

#ifndef __BTC_JSON_H__
#define __BTC_JSON_H__

#include "../Libraries/ArduinoJson/ArduinoJson.h"
#include "Moderator.h"

extern char defaultJSONstr[64];

bool makeJSONString(CModerator& moderator, char* opStr, int len);
bool makeJSONStringEx(CModerator& moderator, char* opStr, int len);
bool makeJSONTimerString(int channel, char* opStr, int len);
bool makeJSONStringGPIO( CModerator& moderator, char* opStr, int len);
void updateJSONclients(bool report);
bool makeJSONStringMQTT(CModerator& moderator, char* opStr, int len);
bool makeJSONStringIP(CModerator& moderator, char* opStr, int len);
void initMQTTJSONmoderator();
void initIPJSONmoderator();
void initTimerJSONmoderator();

template<class T>
const char* createJSON(const char* name, T value)
{
  StaticJsonBuffer<64> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();  // create object to add JSON commands to

	root.set(name, value);
	root.printTo(defaultJSONstr);

  return defaultJSONstr;
}


#endif
