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
#include "NVstorage.h"
#include "../RTC/BTCDateTime.h"
#include "../RTC/Timers.h"
#include "../RTC/TimerManager.h"
#include "../Bluetooth/BluetoothAbstract.h"
#include "../WiFi/BTCWebServer.h"
#include "../cfg/BTCConfig.h"
#include "macros.h"
#include "../Protocol/Protocol.h"


char defaultJSONstr[64];
CModerator JSONmoderator;
CTimerModerator TimerModerator;
int timerConflict = 0;
CModerator MQTTmoderator;
CModerator GPIOmoderator;

void validateTimer(int ID);
void Expand(std::string& str);

void interpretJsonCommand(char* pLine)
{
  if(strlen(pLine) == 0)
    return;

  DebugPort.printf("JSON parse %s...", pLine);

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
      if( !reqTemp(it->value.as<unsigned char>(), false) ) {  // this request is blocked if OEM controller active
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
      sCyclicThermostat cyclic = NVstore.getCyclicMode();
      cyclic.Stop = it->value.as<char>();
      if(cyclic.Stop > 1) cyclic.Stop--;   // internal uses a 1 offset
			NVstore.setCyclicMode(cyclic);
		}
		else if(strcmp("ThermostatUndertemp", it->key) == 0) {
      sCyclicThermostat cyclic = NVstore.getCyclicMode();
      cyclic.Start = it->value.as<char>();
			NVstore.setCyclicMode(cyclic);
		}
		else if(strcmp("ThermostatMethod", it->key) == 0) {
      sUserSettings settings = NVstore.getUserSettings();
      uint8_t val = it->value.as<uint8_t>();
      if(val <= 2)
        settings.ThermostatMethod = val;
			NVstore.setUserSettings(settings);
		}
		else if(strcmp("ThermostatWindow", it->key) == 0) {
      sUserSettings settings = NVstore.getUserSettings();
      float val = it->value.as<float>();
      if(INBOUNDS(val, 0.2f, 10.f))
        settings.ThermostatWindow = val;
			NVstore.setUserSettings(settings);
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
    // request specific timer refresh
		else if((strcmp("TQuery", it->key) == 0) || (strcmp("TimerRefresh", it->key) == 0) ) {
      int timerID = it->value.as<int>();
      if(timerID)
        TimerModerator.reset(timerID-1);
      else 
        TimerModerator.reset();
		}
		else if(strcmp("FanSensor", it->key) == 0) {
      setFanSensor(it->value.as<unsigned char>());
		}
    // MQTT parameters
		else if(strcmp("MQuery", it->key) == 0) {
      MQTTmoderator.reset();   // force MQTT params to be sent
    }
		else if(strcmp("MEn", it->key) == 0) {
      sMQTTparams info = NVstore.getMQTTinfo();
      info.enabled = it->value.as<unsigned char>();
			NVstore.setMQTTinfo(info);
    }
		else if(strcmp("MPort", it->key) == 0) {
      sMQTTparams info = NVstore.getMQTTinfo();
      info.port = it->value.as<unsigned short>();
			NVstore.setMQTTinfo(info);
    }
		else if(strcmp("MHost", it->key) == 0) {
      sMQTTparams info = NVstore.getMQTTinfo();
      strncpy(info.host, it->value.as<const char*>(), 127);
      info.host[127] = 0;
			NVstore.setMQTTinfo(info);
    }
		else if(strcmp("MUser", it->key) == 0) {
      sMQTTparams info = NVstore.getMQTTinfo();
      strncpy(info.username, it->value.as<const char*>(), 31);
      info.username[31] = 0;
			NVstore.setMQTTinfo(info);
    }
		else if(strcmp("MPasswd", it->key) == 0) {
      sMQTTparams info = NVstore.getMQTTinfo();
      strncpy(info.password, it->value.as<const char*>(), 31);
      info.password[31] = 0;
			NVstore.setMQTTinfo(info);
    }
		else if(strcmp("UploadSize", it->key) == 0) {
      setUploadSize(it->value.as<long>());
		}
    else if(strcmp("GPout1", it->key) == 0) {
      setGPIOout(0, it->value.as<unsigned char>() ? true : false);
    }
    else if(strcmp("GPout2", it->key) == 0) {
      setGPIOout(1, it->value.as<unsigned char>() ? true : false);
    }
    else if(strcmp("GPin1", it->key) == 0) {
      simulateGPIOin(it->value.as<unsigned char>() ? 0x01 : 0x00);  // simulate key 1 press
    }
    else if(strcmp("GPin2", it->key) == 0) {
      simulateGPIOin(it->value.as<unsigned char>() ? 0x02 : 0x00);  // simulate key 2 press
    }
    else if(strcmp("JSONpack", it->key) == 0) {
      sUserSettings us = NVstore.getUserSettings();
      uint8_t packed = it->value.as<unsigned char>() ? 0x00 : 0x01;
      us.JSON.LF = packed;
      us.JSON.padding = packed;
      us.JSON.singleElement = packed;
      NVstore.setUserSettings(us);
      NVstore.save();
      resetJSONmoderator();
    }
  }
}

void validateTimer(int ID)
{
  ID--;  // supplied as +1
  if(!INBOUNDS(ID, 0, 13))
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
	bSend |= moderator.addJson("TempMode", NVstore.getUserSettings().degF, root); 

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

  bSend |= moderator.addJson("ThermostatMethod", NVstore.getUserSettings().ThermostatMethod, root); 
  bSend |= moderator.addJson("ThermostatWindow", NVstore.getUserSettings().ThermostatWindow, root); 
  int stop = NVstore.getCyclicMode().Stop;
  if(stop) stop++;  // deliver effective threshold, not internal working value
  bSend |= moderator.addJson("ThermostatOvertemp", stop, root); 
  bSend |= moderator.addJson("ThermostatUndertemp", NVstore.getCyclicMode().Start, root); 

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
	bool bSend = false;  // reset should send flag
  StaticJsonBuffer<800> jsonBuffer;               // create a JSON buffer on the stack
  JsonObject& root = jsonBuffer.createObject();   // create object to add JSON commands to


  sTimer timerInfo;
  NVstore.getTimerInfo(channel, timerInfo);
  bSend |= TimerModerator.addJson(channel, timerInfo, root );

  if(bSend) {
		root.printTo(opStr, len);
  }

  return bSend;
}

bool makeJSONStringGPIO(CModerator& moderator, char* opStr, int len)
{
  StaticJsonBuffer<800> jsonBuffer;               // create a JSON buffer on the stack
  JsonObject& root = jsonBuffer.createObject();   // create object to add JSON commands to

	bool bSend = false;  // reset should send flag

  sGPIO info;
  getGPIOinfo(info);

  bSend |= moderator.addJson("GPin1", info.inState[0], root); 
  bSend |= moderator.addJson("GPin2", info.inState[1], root); 
  bSend |= moderator.addJson("GPout1", info.outState[0], root); 
  bSend |= moderator.addJson("GPout2", info.outState[1], root); 
  bSend |= moderator.addJson("GPanlg", info.algVal * 100 / 4096, root); 
  bSend |= moderator.addJson("GPmodeIn", GPIOinNames[info.inMode], root); 
  bSend |= moderator.addJson("GPmodeOut", GPIOoutNames[info.outMode], root); 
  bSend |= moderator.addJson("GPmodeAnlg", GPIOalgNames[info.algMode], root); 

  if(bSend) {
		root.printTo(opStr, len);
  }

  return bSend;
}

bool makeJSONStringMQTT(CModerator& moderator, char* opStr, int len)
{
  StaticJsonBuffer<800> jsonBuffer;               // create a JSON buffer on the stack
  JsonObject& root = jsonBuffer.createObject();   // create object to add JSON commands to

	bool bSend = false;  // reset should send flag
  sMQTTparams info = NVstore.getMQTTinfo();

  bSend |= moderator.addJson("MEn", info.enabled, root); 
  bSend |= moderator.addJson("MPort", info.port, root); 
  bSend |= moderator.addJson("MHost", info.host, root); 
  bSend |= moderator.addJson("MUser", info.username, root); 
  bSend |= moderator.addJson("MPasswd", info.password, root); 

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
        DebugPort.printf("JSON send: %s\r\n", jsonStr);
      }
      sendWebServerString( jsonStr );
      std::string expand = jsonStr;
      Expand(expand);
      getBluetoothClient().send( expand.c_str() );
    }
  }
  // update extended params
  {
    if(makeJSONStringEx(JSONmoderator, jsonStr, sizeof(jsonStr))) {
      if (report) {
        DebugPort.printf("JSON send: %s\r\n", jsonStr);
      }
      sendWebServerString( jsonStr );
      std::string expand = jsonStr;
      Expand(expand);
      getBluetoothClient().send( expand.c_str() );
    }
  }
  // update timer parameters
  bool bNewTimerInfo = false;
  for(int tmr=0; tmr<14; tmr++) 
  {
    unsigned long tStart = millis();
    if(makeJSONTimerString(tmr, jsonStr, sizeof(jsonStr))) {
      unsigned long tJSON = millis() - tStart;
      if (report) { 
        DebugPort.printf("JSON send: %s\r\n", jsonStr);
      }
      tStart = millis();
      sendWebServerString( jsonStr );
      unsigned long tWF = millis() - tStart;
      tStart = millis();
      std::string expand = jsonStr;
      Expand(expand);
      getBluetoothClient().send( expand.c_str() );
      unsigned long tBT = millis() - tStart;
      bNewTimerInfo = true;
      DebugPort.printf("JSON times : %ld,%ld,%ld\r\n", tJSON, tBT, tWF); 
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

    DebugPort.printf("JSON send: %s\r\n", jsonStr);
    sendWebServerString( jsonStr );
    std::string expand = jsonStr;
    Expand(expand);
    getBluetoothClient().send( expand.c_str() );
  }

  // report MQTT params
  {
    if(makeJSONStringMQTT(MQTTmoderator, jsonStr, sizeof(jsonStr))) {
      if (report) {
        DebugPort.printf("JSON send: %s\r\n", jsonStr);
      }
      sendWebServerString( jsonStr );
      std::string expand = jsonStr;
      Expand(expand);
      getBluetoothClient().send( expand.c_str() );
    }
  }

  {
    if(makeJSONStringGPIO(GPIOmoderator, jsonStr, sizeof(jsonStr))) {
      if (report) {
        DebugPort.printf("JSON send: %s\r\n", jsonStr);
      }
      sendWebServerString( jsonStr );
      std::string expand = jsonStr;
      Expand(expand);
      getBluetoothClient().send( expand.c_str() );
    }
  }

}


void resetJSONmoderator()
{
  JSONmoderator.reset();
#ifdef SALWAYS_SEND_TIMERS
  TimerModerator.reset();
#else
  initTimerJSONmoderator();
#endif
  initMQTTJSONmoderator();
  GPIOmoderator.reset();
}

void initMQTTJSONmoderator()
{
  char jsonStr[800];
  makeJSONStringMQTT(MQTTmoderator, jsonStr, sizeof(jsonStr));
}

void initTimerJSONmoderator()
{
  char jsonStr[800];
  for(int tmr=0; tmr<14; tmr++) 
    makeJSONTimerString(tmr, jsonStr, sizeof(jsonStr));
}


void Expand(std::string& str)
{
  const sUserSettings& userOptions = NVstore.getUserSettings();

  if(userOptions.JSON.singleElement) {
    size_t pos = str.find(",\"");
    while(pos != std::string::npos) {
      if(userOptions.JSON.LF)
        str.replace(pos, 2, "}\n{\"");  // converts {"name":value,"name2":value"} to {"name":value}\n{"name2":value}
      else
        str.replace(pos, 2, "}{\"");   // converts {"name":value,"name2":value"} to {"name":value}{"name2":value}
      pos = str.find(",\"");
    }
    if(userOptions.JSON.padding) {    // converts {"name":value} to {"name": value}
      pos = str.find("\":");
      while(pos != std::string::npos) {
        str.replace(pos, 2, "\": ");
        pos = str.find("\":", pos+1);
      }
    }
    if(userOptions.JSON.LF)
      str.append("\n");
  }
}
