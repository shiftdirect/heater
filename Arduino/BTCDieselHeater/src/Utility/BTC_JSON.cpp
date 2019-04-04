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
#include "../Protocol/helpers.h"
#include "NVstorage.h"
#include "../RTC/BTCDateTime.h"
#include "../RTC/Timers.h"
#include "../RTC/TimerManager.h"
#include "../Bluetooth/BluetoothAbstract.h"
#include "../WiFi/BTCWebServer.h"
#include "../cfg/BTCConfig.h"


char defaultJSONstr[64];
CModerator JSONmoderator;
CTimerModerator TimerModerator;
int timerConflict = 0;

void validateTimer(int ID);

void interpretJsonCommand(char* pLine)
{
  if(strlen(pLine) == 0)
    return;

  DebugPort.print("JSON parse... "); DebugPort.print(pLine);
/*  for(int i=0; i<strlen(pLine); i++) {
    char msg[8];
    sprintf(msg, "%02X ", pLine[i]);
    DebugPort.print(msg);
  }*/

  StaticJsonBuffer<512> jsonBuffer;   // create a JSON buffer on the heap
	JsonObject& obj = jsonBuffer.parseObject(pLine);
	if(!obj.success()) {
		DebugPort.println(" FAILED");
		return;
	}
	DebugPort.println(" OK"); 

	JsonObject::iterator it;
	for(it = obj.begin(); it != obj.end(); ++it) {

		if(strcmp("TempDesired", it->key) == 0) {
      if(!reqTemp(it->value.as<unsigned char>())) {  // this request is blocked if OEM controller active
        JSONmoderator.reset("TempDesired");
      }
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
		else if(strcmp("ThermostatOvertemp", it->key) == 0) {
			NVstore.setCyclicMode(it->value.as<unsigned char>());
		}
		else if(strcmp("ThermostatMethod", it->key) == 0) {
			NVstore.setThermostatMethodMode(it->value.as<unsigned char>());
		}
		else if(strcmp("ThermostatWindow", it->key) == 0) {
			NVstore.setThermostatMethodWindow(it->value.as<float>());
		}
		else if(strcmp("Thermostat", it->key) == 0) {
			if(!setThermostatMode(it->value.as<unsigned char>())) {  // this request is blocked if OEM controller active
        JSONmoderator.reset("ThermoStat");   
      }
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
      resetJSONmoderator();
		}
		else if(strcmp("SystemVoltage", it->key) == 0) {
      setSystemVoltage(it->value.as<float>());
		}
		else if(strcmp("TimerDays", it->key) == 0) {
      // value encoded as "ID Days,Days"
      decodeJSONTimerDays(it->value.as<const char*>());
		}
		else if(strcmp("TimerStart", it->key) == 0) {
      // value encoded as "ID HH:MM"
      decodeJSONTimerTime(0, it->value.as<const char*>());
		}
		else if(strcmp("TimerStop", it->key) == 0) {
      // value encoded as "ID HH:MM"
      decodeJSONTimerTime(1, it->value.as<const char*>());
		}
		else if(strcmp("TimerRepeat", it->key) == 0) {
      // value encoded as "ID val"
      decodeJSONTimerNumeric(0, it->value.as<const char*>());
		}
		else if(strcmp("TimerTemp", it->key) == 0) {
      decodeJSONTimerNumeric(1, it->value.as<const char*>());
		}
		else if(strcmp("TimerConflict", it->key) == 0) {
      validateTimer(it->value.as<int>());
		}
		else if(strcmp("TimerRefresh", it->key) == 0) {
      TimerModerator.reset();
		}
		else if(strcmp("FanSensor", it->key) == 0) {
      setFanSensor(it->value.as<unsigned char>());
		}
	}
}

void validateTimer(int ID)
{
  ID--;  // supplied as +1
  if(!(ID >= 0 && ID < 14))
    return;

  timerConflict = CTimerManager::conflictTest(ID);  // check targeted timer against other timers

  TimerModerator.reset(ID);  // ensure we update client with our (real) version of the selected timer
}

bool makeJSONString(CModerator& moderator, char* opStr, int len)
{
  StaticJsonBuffer<800> jsonBuffer;               // create a JSON buffer on the stack
  JsonObject& root = jsonBuffer.createObject();   // create object to add JSON commands to

	bool bSend = false;  // reset should send flag

  float tidyTemp = getTemperatureSensor();
  tidyTemp = int(tidyTemp * 10) * 0.1f;  // round to 0.1 resolution 
  if(tidyTemp > -80) {
	  bSend |= moderator.addJson("TempCurrent", tidyTemp, root); 
  }
  bSend |= moderator.addJson("TempDesired", getTemperatureDesired(), root); 
	bSend |= moderator.addJson("TempMin", getHeaterInfo().getTemperature_Min(), root); 
	bSend |= moderator.addJson("TempMax", getHeaterInfo().getTemperature_Max(), root); 
	bSend |= moderator.addJson("TempBody", getHeaterInfo().getTemperature_HeatExchg(), root); 
//	bSend |= moderator.addJson("RunState", getHeaterInfo().getRunState(), root);
	bSend |= moderator.addJson("RunState", getHeaterInfo().getRunStateEx(), root);
  bSend |= moderator.addJson("RunString", getHeaterInfo().getRunStateStr(), root); // verbose it up!
	bSend |= moderator.addJson("ErrorState", getHeaterInfo().getErrState(), root );
  bSend |= moderator.addJson("ErrorString", getHeaterInfo().getErrStateStrEx(), root); // verbose it up!
	bSend |= moderator.addJson("Thermostat", getThermostatModeActive(), root );
	bSend |= moderator.addJson("PumpFixed", getHeaterInfo().getPump_Fixed(), root );
	bSend |= moderator.addJson("PumpMin", getHeaterInfo().getPump_Min(), root );
	bSend |= moderator.addJson("PumpMax", getHeaterInfo().getPump_Max(), root );
	bSend |= moderator.addJson("PumpActual", getHeaterInfo().getPump_Actual(), root );
	bSend |= moderator.addJson("FanMin", getHeaterInfo().getFan_Min(), root );
	bSend |= moderator.addJson("FanMax", getHeaterInfo().getFan_Max(), root );
	bSend |= moderator.addJson("FanRPM", getHeaterInfo().getFan_Actual(), root );
	bSend |= moderator.addJson("FanVoltage", getHeaterInfo().getFan_Voltage(), root );
	bSend |= moderator.addJson("FanSensor", getHeaterInfo().getFan_Sensor(), root );
	bSend |= moderator.addJson("InputVoltage", getHeaterInfo().getBattVoltage(), root );
	bSend |= moderator.addJson("SystemVoltage", getHeaterInfo().getSystemVoltage(), root );
	bSend |= moderator.addJson("GlowVoltage", getHeaterInfo().getGlow_Voltage(), root );
	bSend |= moderator.addJson("GlowCurrent", getHeaterInfo().getGlow_Current(), root );
  bSend |= moderator.addJson("BluewireStat", getBlueWireStatStr(), root );
	bSend |= moderator.addJson("TempMode", NVstore.getDegFMode(), root); 

  if(bSend) {
		root.printTo(opStr, len);
  }

  return bSend;
}

bool makeJSONStringEx(CModerator& moderator, char* opStr, int len)
{
  StaticJsonBuffer<800> jsonBuffer;               // create a JSON buffer on the stack
  JsonObject& root = jsonBuffer.createObject();   // create object to add JSON commands to

	bool bSend = false;  // reset should send flag

  bSend |= moderator.addJson("ThermostatMethod", NVstore.getThermostatMethodMode(), root); 
  bSend |= moderator.addJson("ThermostatWindow", NVstore.getThermostatMethodWindow(), root); 
  bSend |= moderator.addJson("ThermostatOvertemp", NVstore.getCyclicMode(), root); 

  if(bSend) {
		root.printTo(opStr, len);
  }

  return bSend;
}

// the way the JSON timer strings are crafted, we have to iterate over each timer's parameters
// individually, the JSON name is always the same for each timer, the payload IDs the specific
// timer
// Only timer parameters that have changed will be sent, after reset the typical string will be
// {"TimerStart":XX:XX,"TimerStop":XX:XX,"TimerDays":XX,"TimerRepeat":X}
bool makeJSONTimerString(int channel, char* opStr, int len)
{
  StaticJsonBuffer<800> jsonBuffer;               // create a JSON buffer on the stack
  JsonObject& root = jsonBuffer.createObject();   // create object to add JSON commands to

	bool bSend = false;  // reset should send flag

  sTimer timerInfo;
  NVstore.getTimerInfo(channel, timerInfo);
  bSend |= TimerModerator.addJson(channel, timerInfo, root );

  if(bSend) {
		root.printTo(opStr, len);
  }

  return bSend;
}



void updateJSONclients(bool report)
{
  // update general parameters
  char jsonStr[800];
  {
    if(makeJSONString(JSONmoderator, jsonStr, sizeof(jsonStr))) {
      if (report) {
        DebugPort.print("JSON send: "); DebugPort.println(jsonStr);
      }
      getBluetoothClient().send( jsonStr );
      sendWebServerString( jsonStr );
    }
  }
  // update extended params
  {
    if(makeJSONStringEx(JSONmoderator, jsonStr, sizeof(jsonStr))) {
      if (report) {
        DebugPort.print("JSON send: "); DebugPort.println(jsonStr);
      }
      getBluetoothClient().send( jsonStr );
      sendWebServerString( jsonStr );
    }
  }
  // update timer parameters
  bool bNewTimerInfo = false;
  for(int tmr=0; tmr<14; tmr++) 
  {
    if(makeJSONTimerString(tmr, jsonStr, sizeof(jsonStr))) {
      if (report) { 
        DebugPort.print("JSON send: "); DebugPort.println(jsonStr);
      }
      getBluetoothClient().send( jsonStr );
      sendWebServerString( jsonStr );
      bNewTimerInfo = true;
    }
  }
  // request timer refesh upon clients
  if(bNewTimerInfo) {
    StaticJsonBuffer<800> jsonBuffer;               // create a JSON buffer on the stack
    JsonObject& root = jsonBuffer.createObject();   // create object to add JSON commands to

    if(timerConflict) {
      root.set("TimerConflict", timerConflict);
      timerConflict = 0;
    }
    root.set("TimerRefresh", 1);
    root.printTo(jsonStr, 800);

    DebugPort.print("JSON send: "); DebugPort.println(jsonStr);
    getBluetoothClient().send( jsonStr );
    sendWebServerString( jsonStr );
  }
}


void resetJSONmoderator()
{
  JSONmoderator.reset();
  TimerModerator.reset();
}



