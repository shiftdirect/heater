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

#include "../../lib/ArduinoJson/ArduinoJson.h"
#include "Moderator.h"

extern char defaultJSONstr[64];

void updateJSONclients(bool report);
void initJSONMQTTmoderator();
void initJSONIPmoderator();
void initJSONTimermoderator();
void initJSONSysModerator();
void triggerJSONTimeUpdate();
void resetAllJSONmoderators();
void resetJSONmoderator(const char* name);
void resetJSONTimerModerator(int timerID);
void resetJSONIPmoderator();
void resetJSONSysModerator();
void resetJSONMQTTmoderator();
void validateTimer(int ID);
void doJSONreboot(uint16_t code);
void sendJSONtext(const char* JSONstr);

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
