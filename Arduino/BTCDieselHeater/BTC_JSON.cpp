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

#include "BTC_JSON.h"
#include "DebugPort.h"
#include "helpers.h"
#include "NVstorage.h"
#include "BTCDateTime.h"


char defaultJSONstr[64];

void decodeTimerDays(int timerID, const char* str);
void decodeTimerTime(int ID, int stop, const char*);
void decodeTimerRepeat(int ID, int state);


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
		else if(strcmp("NVsave", it->key) == 0) {
      if(it->value.as<int>() == 8861)
			  saveNV();
		}
		else if(strcmp("DateTime", it->key) == 0) {
      setDateTime(it->value.as<const char*>());
		}
		else if(strcmp("Date", it->key) == 0) {
      setDate(it->value.as<const char*>());
		}
		else if(strcmp("Time", it->key) == 0) {
      setTime(it->value.as<const char*>());
		}
		else if(strcmp("PumpPrime", it->key) == 0) {
      reqPumpPrime(it->value.as<unsigned char>());
		}
		else if(strcmp("Refresh", it->key) == 0) {
      resetWebModerator();
      resetBTModerator();
		}
		else if(strcmp("Timer1Days", it->key) == 0) {
      decodeTimerDays(0, it->value.as<const char*>());
		}
		else if(strcmp("Timer1Start", it->key) == 0) {
      decodeTimerTime(0, 0, it->value.as<const char*>());
		}
		else if(strcmp("Timer1Stop", it->key) == 0) {
      decodeTimerTime(0, 1, it->value.as<const char*>());
		}
		else if(strcmp("Timer1Repeat", it->key) == 0) {
      decodeTimerRepeat(0, it->value.as<unsigned char>());
		}
		else if(strcmp("Timer2Days", it->key) == 0) {
      decodeTimerDays(1, it->value.as<const char*>());
		}
		else if(strcmp("Timer2Start", it->key) == 0) {
      decodeTimerTime(1, 0, it->value.as<const char*>());
		}
		else if(strcmp("Timer2Stop", it->key) == 0) {
      decodeTimerTime(1, 1, it->value.as<const char*>());
		}
		else if(strcmp("Timer2Repeat", it->key) == 0) {
      decodeTimerRepeat(1, it->value.as<unsigned char>());
		}
	}
}



bool makeJsonString(CModerator& moderator, char* opStr, int len)
{
  StaticJsonBuffer<512> jsonBuffer;               // create a JSON buffer on the stack
  JsonObject& root = jsonBuffer.createObject();   // create object to add JSON commands to

	bool bSend = false;  // reset should send flag

  float tidyTemp = int(getActualTemperature() * 10) * 0.1f;  // round to 0.1 resolution 
	bSend |= moderator.addJson("TempCurrent", tidyTemp, root); 
	bSend |= moderator.addJson("TempDesired", getHeaterInfo().getTemperature_Desired(), root); 
	bSend |= moderator.addJson("TempMin", getHeaterInfo().getTemperature_Min(), root); 
	bSend |= moderator.addJson("TempMax", getHeaterInfo().getTemperature_Max(), root); 
	bSend |= moderator.addJson("TempBody", getHeaterInfo().getTemperature_HeatExchg(), root); 
	bSend |= moderator.addJson("RunState", getHeaterInfo().getRunState(), root); 
	bSend |= moderator.addJson("ErrorState", getHeaterInfo().getErrState(), root );
	bSend |= moderator.addJson("Thermostat", getHeaterInfo().isThermostat(), root );
	bSend |= moderator.addJson("PumpFixed", getHeaterInfo().getPump_Fixed(), root );
	bSend |= moderator.addJson("PumpMin", getHeaterInfo().getPump_Min(), root );
	bSend |= moderator.addJson("PumpMax", getHeaterInfo().getPump_Max(), root );
	bSend |= moderator.addJson("PumpActual", getHeaterInfo().getPump_Actual(), root );
	bSend |= moderator.addJson("FanMin", getHeaterInfo().getFan_Min(), root );
	bSend |= moderator.addJson("FanMax", getHeaterInfo().getFan_Max(), root );
	bSend |= moderator.addJson("FanRPM", getHeaterInfo().getFan_Actual(), root );
	bSend |= moderator.addJson("FanVoltage", getHeaterInfo().getFan_Voltage(), root );
	bSend |= moderator.addJson("SystemVoltage", getHeaterInfo().getBattVoltage(), root );
	bSend |= moderator.addJson("GlowVoltage", getHeaterInfo().getGlow_Voltage(), root );
	bSend |= moderator.addJson("GlowCurrent", getHeaterInfo().getGlow_Current(), root );

  if(bSend) {
		root.printTo(opStr, len);
  }

  return bSend;
}


void decodeTimerDays(int ID, const char* str)
{
  sTimer timer;
  NVstore.getTimerInfo(ID, timer);
  unsigned char days = 0;
  if(strstr(str, "Next"))  {
    days = 0x80;
  }
  else {
    for(int i=0; i< 7; i++) {
      int mask = 0x01 << i;
      if(strstr(str, daysOfTheWeek[i]))  
        days |= mask;
    }
  }
  timer.enabled = days;
  NVstore.setTimerInfo(ID, timer);
}

void decodeTimerTime(int ID, int stop, const char* str)
{
  sTimer timer;
  NVstore.getTimerInfo(ID, timer);
  int hour, minute;
  if(2 == sscanf(str, "%d:%d", &hour, &minute)) {
    if(stop) {
      timer.stop.hour = hour;
      timer.stop.min = minute;
    }
    else {
      timer.start.hour = hour;
      timer.start.min = minute;
    }
    NVstore.setTimerInfo(ID, timer);
  }
}

void decodeTimerRepeat(int ID, int state)
{
  sTimer timer;
  NVstore.getTimerInfo(ID, timer);
  timer.repeat = state;
  NVstore.setTimerInfo(ID, timer);
}