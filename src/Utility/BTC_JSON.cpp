/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2019  Ray Jones <ray@mrjones.id.au>
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
#include "NVStorage.h"
#include "../RTC/Clock.h"
#include "../RTC/RTCStore.h"
#include "../RTC/BTCDateTime.h"
#include "../RTC/Timers.h"
#include "../RTC/TimerManager.h"
#include "../Bluetooth/BluetoothAbstract.h"
#include "../WiFi/BTCWebServer.h"
#include "../WiFi/BTCWifi.h"
#include "../WiFi/ABmqtt.h"
#include "../cfg/BTCConfig.h"
#include "macros.h"
#include "../Protocol/Protocol.h"
#include <string.h>
#include "HourMeter.h"

extern CModerator MQTTmoderator;

char defaultJSONstr[64];
CModerator JSONmoderator;
CTimerModerator TimerModerator;
int timerConflict = 0;
CModerator MQTTJSONmoderator;
CModerator IPmoderator;
CModerator GPIOmoderator;
CModerator SysModerator;
bool bTriggerSysParams = false;
bool bTriggerDateTime = false;

void validateTimer(int ID);
void Expand(std::string& str);
bool makeJSONString(CModerator& moderator, char* opStr, int len);
bool makeJSONStringEx(CModerator& moderator, char* opStr, int len);
bool makeJSONTimerString(int channel, char* opStr, int len);
bool makeJSONStringGPIO( CModerator& moderator, char* opStr, int len);
bool makeJSONStringSysInfo(CModerator& moderator, char* opStr, int len);
bool makeJSONStringMQTT(CModerator& moderator, char* opStr, int len);
bool makeJSONStringIP(CModerator& moderator, char* opStr, int len);
void DecodeCmd(const char* cmd, String& payload);

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

    String payload(it->value.as<const char*>());
    DecodeCmd(it->key, payload);
/*

		if(strcmp("TempDesired", it->key) == 0) {
      if( !reqDemand(it->value.as<uint8_t>(), false) ) {  // this request is blocked if OEM controller active
        JSONmoderator.reset("TempDesired");
      }
		}
		else if(strcmp("RunState", it->key) == 0) {
			if(it->value.as<uint8_t>()) {
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
			setFanMin(it->value.as<uint16_t>());
		}
		else if(strcmp("FanMax", it->key) == 0) {
			setFanMax(it->value.as<uint16_t>());
		}
    else if(strcmp("CyclicTemp", it->key) == 0) {
      setDemandDegC(it->value.as<uint8_t>());  // directly set demandDegC
    }
		else if((strcmp("CyclicOff", it->key) == 0) || (strcmp("ThermostatOvertemp", it->key) == 0)) {
      sUserSettings us = NVstore.getUserSettings();
      us.cyclic.Stop = it->value.as<int8_t>();
      if(INBOUNDS(us.cyclic.Stop, 0, 10)) {
        if(us.cyclic.Stop > 1) 
          us.cyclic.Stop--;   // internal uses a 1 offset
			  NVstore.setUserSettings(us);
      }
		}
		else if((strcmp("CyclicOn", it->key) == 0) || (strcmp("ThermostatUndertemp", it->key) == 0)) {
      sUserSettings us = NVstore.getUserSettings();
      us.cyclic.Start = it->value.as<int8_t>();
      if(INBOUNDS(us.cyclic.Start, -20, 0)) 
  			NVstore.setUserSettings(us);
		}
		else if(strcmp("ThermostatMethod", it->key) == 0) {
      sUserSettings settings = NVstore.getUserSettings();
      settings.ThermostatMethod = it->value.as<uint8_t>();
      if(INBOUNDS(settings.ThermostatMethod, 0, 3))
  			NVstore.setUserSettings(settings);
		}
		else if(strcmp("ThermostatWindow", it->key) == 0) {
      sUserSettings settings = NVstore.getUserSettings();
      settings.ThermostatWindow = it->value.as<float>();
      if(INBOUNDS(settings.ThermostatWindow, 0.2f, 10.f))
  			NVstore.setUserSettings(settings);
		}
		else if(strcmp("Thermostat", it->key) == 0) {
			if(!setThermostatMode(it->value.as<uint8_t>())) {  // this request is blocked if OEM controller active
        JSONmoderator.reset("ThermoStat");   
      }
		}
    else if(strcmp("ExtThermoTmout", it->key) == 0) {
      sUserSettings us = NVstore.getUserSettings();
      us.ExtThermoTimeout = it->value.as<long>();
      if(INBOUNDS(us.ExtThermoTimeout, 0, 3600000))
        NVstore.setUserSettings(us);
    }
		else if(strcmp("NVsave", it->key) == 0) {
      if(it->value.as<int>() == 8861)
			  saveNV();
		}
    else if(strcmp("Watchdog", it->key) == 0) {
      doJSONwatchdog(it->value.as<int>());
    }
		else if(strcmp("DateTime", it->key) == 0) {
      setDateTime(it->value.as<const char*>());
      bTriggerDateTime = true;
		}
		else if(strcmp("Date", it->key) == 0) {
      setDate(it->value.as<const char*>());
      bTriggerDateTime = true;
		}
		else if(strcmp("Time", it->key) == 0) {
      setTime(it->value.as<const char*>());
      bTriggerDateTime = true;
		}
		else if(strcmp("Time12hr", it->key) == 0) {
      sUserSettings us = NVstore.getUserSettings();
      us.clock12hr = it->value.as<uint8_t>() ? 1 : 0;
      NVstore.setUserSettings(us);
      NVstore.save();
		}
		else if(strcmp("PumpPrime", it->key) == 0) {
      reqPumpPrime(it->value.as<uint8_t>());
		}
		else if(strcmp("Refresh", it->key) == 0) {
      resetJSONmoderator();
      refreshMQTT();
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
      setFanSensor(it->value.as<uint8_t>());
		}
		else if(strcmp("IQuery", it->key) == 0) {
      IPmoderator.reset();   // force IP params to be sent
    }
    // system info
		else if(strcmp("SQuery", it->key) == 0) {
      SysModerator.reset();   // force MQTT params to be sent
      bTriggerSysParams = true;
    }
    // MQTT parameters
		else if(strcmp("MQuery", it->key) == 0) {
      MQTTJSONmoderator.reset();   // force MQTT params to be sent
    }
		else if(strcmp("MEn", it->key) == 0) {
      sMQTTparams info = NVstore.getMQTTinfo();
      info.enabled = it->value.as<uint8_t>();
			NVstore.setMQTTinfo(info);
    }
		else if(strcmp("MPort", it->key) == 0) {
      sMQTTparams info = NVstore.getMQTTinfo();
      info.port = it->value.as<uint16_t>();
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
		else if(strcmp("MQoS", it->key) == 0) {
      sMQTTparams info = NVstore.getMQTTinfo();
      info.qos = it->value.as<uint8_t>();
      if(INBOUNDS(info.qos, 0, 2)) {
  			NVstore.setMQTTinfo(info);
      }
    }
		else if(strcmp("MTopic", it->key) == 0) {
      sMQTTparams info = NVstore.getMQTTinfo();
      strncpy(info.topic, it->value.as<const char*>(), 31);
      info.topic[31] = 0;
			NVstore.setMQTTinfo(info);
    }
		else if(strcmp("UploadSize", it->key) == 0) {
      setUploadSize(it->value.as<long>());
		}
    else if(strcmp("GPout1", it->key) == 0) {
      setGPIOout(0, it->value.as<uint8_t>() ? true : false);
    }
    else if(strcmp("GPout2", it->key) == 0) {
      setGPIOout(1, it->value.as<uint8_t>() ? true : false);
    }
    else if(strcmp("GPin1", it->key) == 0) {
      simulateGPIOin(it->value.as<uint8_t>() ? 0x01 : 0x00);  // simulate key 1 press
    }
    else if(strcmp("GPin2", it->key) == 0) {
      simulateGPIOin(it->value.as<uint8_t>() ? 0x02 : 0x00);  // simulate key 2 press
    }
    else if(strcmp("JSONpack", it->key) == 0) {
      sUserSettings us = NVstore.getUserSettings();
      uint8_t packed = it->value.as<uint8_t>() ? 0x00 : 0x01;
      us.JSON.LF = packed;
      us.JSON.padding = packed;
      us.JSON.singleElement = packed;
      NVstore.setUserSettings(us);
      NVstore.save();
      resetJSONmoderator();
    }
    else if(strcmp("TempMode", it->key) == 0) {
      sUserSettings us = NVstore.getUserSettings();
      us.degF = it->value.as<uint8_t>() ? 0x01 : 0x00;
      NVstore.setUserSettings(us);
      NVstore.save();
    }
    else if(strcmp("PumpCount", it->key) == 0) {  // reset fuel gauge
      int Count = it->value.as<int>();
      if(Count == 0) {
        resetFuelGauge();
      }
    }
    else if(strcmp("PumpCal", it->key) == 0) {
      sHeaterTuning ht = NVstore.getHeaterTuning();
      ht.pumpCal = it->value.as<float>();
      if(INBOUNDS(ht.pumpCal, 0.001, 1)) {
        NVstore.setHeaterTuning(ht);
      }
    }
    else if(strcmp("TempOffset", it->key) == 0) {
      sHeaterTuning ht = NVstore.getHeaterTuning();
      ht.tempOfs = it->value.as<float>();
      if(INBOUNDS(ht.tempOfs, -10.0, +10.0)) {
        NVstore.setHeaterTuning(ht);
      }
    }
    else if(strcmp("LowVoltCutout", it->key) == 0) {
      float fCal = it->value.as<float>();
      bool bOK = false;
      if(NVstore.getHeaterTuning().sysVoltage == 120)
        bOK |= (fCal == 0) || INBOUNDS(fCal, 10.0, 12.5);
      else
        bOK |= (fCal == 0) || INBOUNDS(fCal, 20.0, 25.0);
      if(bOK) {
        sHeaterTuning ht = NVstore.getHeaterTuning();
        ht.lowVolts = uint8_t(fCal * 10);
        NVstore.setHeaterTuning(ht);
      }
    }*/
    else if(strcmp("SMenu", it->key) == 0) {
      sUserSettings us = NVstore.getUserSettings();
      us.menuMode = it->value.as<uint8_t>();
      if(us.menuMode <=2) {
        NVstore.setUserSettings(us);
        NVstore.save();
        switch(us.menuMode) {
          case 0: DebugPort.println("Invoking Full menu control mode"); break;
          case 1: DebugPort.println("Invoking Basic menu mode"); break;
          case 2: DebugPort.println("Invoking cut back No Heater mode"); break;
        }
        reloadScreens();
      }
    }
    else if(strcmp("SysHourMeters", it->key) == 0) {
      pHourMeter->resetHard();
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
  tidyTemp = int(tidyTemp * 10 + 0.5) * 0.1f;  // round to 0.1 resolution 
  if(tidyTemp > -80) {
	  bSend |= moderator.addJson("TempCurrent", tidyTemp, root); 
  }
  bSend |= moderator.addJson("TempDesired", getTemperatureDesired(), root); 
	bSend |= moderator.addJson("TempMode", NVstore.getUserSettings().degF, root); 
  if(NVstore.getUserSettings().menuMode < 2) {
    bSend |= moderator.addJson("TempMin", getHeaterInfo().getTemperature_Min(), root); 
    bSend |= moderator.addJson("TempMax", getHeaterInfo().getTemperature_Max(), root); 
    bSend |= moderator.addJson("TempBody", getHeaterInfo().getTemperature_HeatExchg(), root); 
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
    bSend |= moderator.addJson("FanRPM", getFanSpeed(), root );
    bSend |= moderator.addJson("FanVoltage", getHeaterInfo().getFan_Voltage(), root );
    bSend |= moderator.addJson("FanSensor", getHeaterInfo().getFan_Sensor(), root );
    bSend |= moderator.addJson("InputVoltage", getBatteryVoltage(false), root );
    bSend |= moderator.addJson("SystemVoltage", getHeaterInfo().getSystemVoltage(), root );
    bSend |= moderator.addJson("GlowVoltage", getGlowVolts(), root );
    bSend |= moderator.addJson("GlowCurrent", getGlowCurrent(), root );
    bSend |= moderator.addJson("BluewireStat", getBlueWireStatStr(), root );
  }

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

  if(NVstore.getUserSettings().menuMode < 2) {
    bSend |= moderator.addJson("ThermostatMethod", NVstore.getUserSettings().ThermostatMethod, root); 
    bSend |= moderator.addJson("ThermostatWindow", NVstore.getUserSettings().ThermostatWindow, root); 
    int stop = NVstore.getUserSettings().cyclic.Stop;
    if(stop) stop++;  // deliver effective threshold, not internal working value
    bSend |= moderator.addJson("ThermostatOvertemp", stop, root); 
    bSend |= moderator.addJson("ThermostatUndertemp", NVstore.getUserSettings().cyclic.Start, root); 
    bSend |= moderator.addJson("CyclicTemp", getDemandDegC(), root);             // actual pivot point for cyclic mode
    bSend |= moderator.addJson("CyclicOff", stop, root);                         // threshold of over temp for cyclic mode
    bSend |= moderator.addJson("CyclicOn", NVstore.getUserSettings().cyclic.Start, root);  // threshold of under temp for cyclic mode
    bSend |= moderator.addJson("PumpCount", RTC_Store.getFuelGauge(), root);               // running count of pump strokes
    bSend |= moderator.addJson("PumpCal", NVstore.getHeaterTuning().pumpCal, root);        // mL/stroke
    bSend |= moderator.addJson("LowVoltCutout", NVstore.getHeaterTuning().getLVC(), root); // low volatge cutout
  }
  bSend |= moderator.addJson("TempOffset", NVstore.getHeaterTuning().tempOfs, root);     // degC offset

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
  bSend |= moderator.addJson("GPmodeIn1", GPIOin1Names[info.in1Mode], root); 
  bSend |= moderator.addJson("GPmodeIn2", GPIOin2Names[info.in2Mode], root); 
  bSend |= moderator.addJson("GPmodeOut1", GPIOout1Names[info.out1Mode], root); 
  bSend |= moderator.addJson("GPmodeOut2", GPIOout2Names[info.out2Mode], root); 
  bSend |= moderator.addJson("GPmodeAnlg", GPIOalgNames[info.algMode], root); 
  bSend |= moderator.addJson("ExtThermoTmout", (uint32_t)NVstore.getUserSettings().ExtThermoTimeout, root); 

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
  bSend |= moderator.addJson("MOnline", isMQTTconnected(), root); 
  bSend |= moderator.addJson("MHost", info.host, root); 
  bSend |= moderator.addJson("MPort", info.port, root); 
  bSend |= moderator.addJson("MUser", info.username, root); 
  bSend |= moderator.addJson("MPasswd", info.password, root); 
  bSend |= moderator.addJson("MQoS", info.qos, root); 
  bSend |= moderator.addJson("MTopic", info.topic, root); 

  if(bSend) {
		root.printTo(opStr, len);
  }

  return bSend;
}


bool makeJSONStringSysInfo(CModerator& moderator, char* opStr, int len)
{
	bool bSend = false;  // reset should send flag

  if(bTriggerSysParams || bTriggerDateTime) {

    StaticJsonBuffer<800> jsonBuffer;               // create a JSON buffer on the stack
    JsonObject& root = jsonBuffer.createObject();   // create object to add JSON commands to

    const BTCDateTime& now = Clock.get();

    char str[32];
    sprintf(str, "%d/%d/%d %02d:%02d:%02d", now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());
    bSend |= moderator.addJson("DateTime", str, root); 
    bSend |= moderator.addJson("Time12hr", NVstore.getUserSettings().clock12hr, root); 
    if(bTriggerSysParams) {
      bSend |= moderator.addJson("SysUpTime", sysUptime(), root); 
      bSend |= moderator.addJson("SysVer", getVersionStr(), root); 
      bSend |= moderator.addJson("SysDate", getVersionDate(), root); 
      bSend |= moderator.addJson("SysFreeMem", ESP.getFreeHeap(), root); 
      if(NVstore.getUserSettings().menuMode < 2) {
        bSend |= moderator.addJson("SysRunTime", pHourMeter->getRunTime(), root); 
        bSend |= moderator.addJson("SysGlowTime", pHourMeter->getGlowTime(), root); 
      }
    }
    if(bSend) {
	  	root.printTo(opStr, len);
    }
  }

  bTriggerSysParams = false;
  bTriggerDateTime = false;

  return bSend;
}

bool makeJSONStringIP(CModerator& moderator, char* opStr, int len)
{
  StaticJsonBuffer<800> jsonBuffer;               // create a JSON buffer on the stack
  JsonObject& root = jsonBuffer.createObject();   // create object to add JSON commands to

	bool bSend = false;  // reset should send flag

  bSend |= moderator.addJson("IP_AP", getWifiAPAddrStr(), root); 
  bSend |= moderator.addJson("IP_APMAC", getWifiAPMACStr(), root); 
  bSend |= moderator.addJson("IP_STA", getWifiSTAAddrStr(), root); 
  bSend |= moderator.addJson("IP_STAMAC", getWifiSTAMACStr(), root); 
  bSend |= moderator.addJson("IP_STASSID", getSSID().c_str(), root); 
  bSend |= moderator.addJson("IP_OTA", NVstore.getUserSettings().enableOTA, root); 
  bSend |= moderator.addJson("BT_MAC", getBluetoothClient().getMAC(), root); 

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
      sendWebSocketString( jsonStr );
      mqttPublishJSON(jsonStr);
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
      sendWebSocketString( jsonStr );
      mqttPublishJSON(jsonStr);
      std::string expand = jsonStr;
      Expand(expand);
      getBluetoothClient().send( expand.c_str() );
    }
  }
  // update timer parameters
  bool bNewTimerInfo = false;
  for(int tmr=0; tmr<14; tmr++) 
  {
    if(makeJSONTimerString(tmr, jsonStr, sizeof(jsonStr))) {
      if (report) { 
        DebugPort.printf("JSON send: %s\r\n", jsonStr);
      }
      sendWebSocketString( jsonStr );
      mqttPublishJSON(jsonStr);
      std::string expand = jsonStr;
      Expand(expand);
      getBluetoothClient().send( expand.c_str() );
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

    if (report) {
      DebugPort.printf("JSON send: %s\r\n", jsonStr);
    }
    sendWebSocketString( jsonStr );
    mqttPublishJSON(jsonStr);
    std::string expand = jsonStr;
    Expand(expand);
    getBluetoothClient().send( expand.c_str() );
  }

  // report MQTT params
  {
    if(makeJSONStringMQTT(MQTTJSONmoderator, jsonStr, sizeof(jsonStr))) {
      if (report) {
        DebugPort.printf("JSON send: %s\r\n", jsonStr);
      }
      sendWebSocketString( jsonStr );
      mqttPublishJSON(jsonStr);
      std::string expand = jsonStr;
      Expand(expand);
      getBluetoothClient().send( expand.c_str() );
    }
  }

  // report IP params
  {
    if(makeJSONStringIP(IPmoderator, jsonStr, sizeof(jsonStr))) {
      if (report) {
        DebugPort.printf("JSON send: %s\r\n", jsonStr);
      }
      sendWebSocketString( jsonStr );
      mqttPublishJSON(jsonStr);
      std::string expand = jsonStr;
      Expand(expand);
      getBluetoothClient().send( expand.c_str() );
    }
  }

  // report System info
  {
    if(makeJSONStringSysInfo(SysModerator, jsonStr, sizeof(jsonStr))) {
      if (report) {
        DebugPort.printf("JSON send: %s\r\n", jsonStr);
      }
      sendWebSocketString( jsonStr );
      mqttPublishJSON(jsonStr);
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
      sendWebSocketString( jsonStr );
      mqttPublishJSON(jsonStr);
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
  initIPJSONmoderator();
  initSysModerator();
  GPIOmoderator.reset();
}

void initMQTTJSONmoderator()
{
  char jsonStr[800];
  makeJSONStringMQTT(MQTTJSONmoderator, jsonStr, sizeof(jsonStr));
}

void initIPJSONmoderator()
{
  char jsonStr[800];
  makeJSONStringIP(IPmoderator, jsonStr, sizeof(jsonStr));
}

void initSysModerator()
{
  char jsonStr[800];
  makeJSONStringSysInfo(SysModerator, jsonStr, sizeof(jsonStr));
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

void DecodeCmd(const char* cmd, String& payload) 
{
  if(strcmp("TempDesired", cmd) == 0) {
    if( !reqDemand(payload.toInt(), false) ) {  // this request is blocked if OEM controller active
      JSONmoderator.reset("TempDesired");
    }
  }
  else if(strcmp("Run", cmd) == 0) {
    refreshMQTT();
    if(payload == "1") {
      requestOn();
    }
    else if(payload == "0") {
      requestOff();
    }
  }
  else if(strcmp("RunState", cmd) == 0) {
    if(payload.toInt()) {
      requestOn();
    }
    else {
      requestOff();
    }
  }
  else if(strcmp("PumpMin", cmd) == 0) {
    setPumpMin(payload.toFloat());
  }
  else if(strcmp("PumpMax", cmd) == 0) {
    setPumpMax(payload.toFloat());
  }
  else if(strcmp("FanMin", cmd) == 0) {
    setFanMin(payload.toInt());
  }
  else if(strcmp("FanMax", cmd) == 0) {
    setFanMax(payload.toInt());
  }
  else if(strcmp("CyclicTemp", cmd) == 0) {
    setDemandDegC(payload.toInt());  // directly set demandDegC
  }
  else if((strcmp("CyclicOff", cmd) == 0) || (strcmp("ThermostatOvertemp", cmd) == 0)) {
    sUserSettings us = NVstore.getUserSettings();
    us.cyclic.Stop = payload.toInt();
    if(INBOUNDS(us.cyclic.Stop, 0, 10)) {
      if(us.cyclic.Stop > 1) 
        us.cyclic.Stop--;   // internal uses a 1 offset
      NVstore.setUserSettings(us);
    }
  }
  else if((strcmp("CyclicOn", cmd) == 0) || (strcmp("ThermostatUndertemp", cmd) == 0)) {
    sUserSettings us = NVstore.getUserSettings();
    us.cyclic.Start = payload.toInt();
    if(INBOUNDS(us.cyclic.Start, -20, 0)) 
      NVstore.setUserSettings(us);
  }
  else if(strcmp("ThermostatMethod", cmd) == 0) {
    sUserSettings settings = NVstore.getUserSettings();
    settings.ThermostatMethod = payload.toInt();
    if(INBOUNDS(settings.ThermostatMethod, 0, 3))
      NVstore.setUserSettings(settings);
  }
  else if(strcmp("ThermostatWindow", cmd) == 0) {
    sUserSettings settings = NVstore.getUserSettings();
    settings.ThermostatWindow = payload.toFloat();
    if(INBOUNDS(settings.ThermostatWindow, 0.2f, 10.f))
      NVstore.setUserSettings(settings);
  }
  else if(strcmp("Thermostat", cmd) == 0) {
    if(!setThermostatMode(payload.toInt())) {  // this request is blocked if OEM controller active
      JSONmoderator.reset("ThermoStat");   
    }
  }
  else if(strcmp("ExtThermoTmout", cmd) == 0) {
    sUserSettings us = NVstore.getUserSettings();
    us.ExtThermoTimeout = payload.toInt();
    if(INBOUNDS(us.ExtThermoTimeout, 0, 3600000))
      NVstore.setUserSettings(us);
  }
  else if(strcmp("NVsave", cmd) == 0) {
    if(payload.toInt() == 8861)
      saveNV();
  }
  else if(strcmp("Watchdog", cmd) == 0) {
    doJSONwatchdog(payload.toInt());
  }
  else if(strcmp("DateTime", cmd) == 0) {
    setDateTime(payload.c_str());
    bTriggerDateTime = true;
  }
  else if(strcmp("Date", cmd) == 0) {
    setDate(payload.c_str());
    bTriggerDateTime = true;
  }
  else if(strcmp("Time", cmd) == 0) {
    setTime(payload.c_str());
    bTriggerDateTime = true;
  }
  else if(strcmp("Time12hr", cmd) == 0) {
    sUserSettings us = NVstore.getUserSettings();
    us.clock12hr = payload.toInt() ? 1 : 0;
    NVstore.setUserSettings(us);
    NVstore.save();
  }
  else if(strcmp("PumpPrime", cmd) == 0) {
    reqPumpPrime(payload.toInt());
  }
  else if(strcmp("Refresh", cmd) == 0) {
    resetJSONmoderator();
    refreshMQTT();
  }
  else if(strcmp("SystemVoltage", cmd) == 0) {
    setSystemVoltage(payload.toFloat());
  }
  else if(strcmp("TimerDays", cmd) == 0) {
    // value encoded as "ID Days,Days"
    decodeJSONTimerDays(payload.c_str());
  }
  else if(strcmp("TimerStart", cmd) == 0) {
    // value encoded as "ID HH:MM"
    decodeJSONTimerTime(0, payload.c_str());
  }
  else if(strcmp("TimerStop", cmd) == 0) {
    // value encoded as "ID HH:MM"
    decodeJSONTimerTime(1, payload.c_str());
  }
  else if(strcmp("TimerRepeat", cmd) == 0) {
    // value encoded as "ID val"
    decodeJSONTimerNumeric(0, payload.c_str());
  }
  else if(strcmp("TimerTemp", cmd) == 0) {
    decodeJSONTimerNumeric(1, payload.c_str());
  }
  else if(strcmp("TimerConflict", cmd) == 0) {
    validateTimer(payload.toInt());
  }
  // request specific timer refresh
  else if((strcmp("TQuery", cmd) == 0) || (strcmp("TimerRefresh", cmd) == 0) ) {
    int timerID = payload.toInt();
    if(timerID)
      TimerModerator.reset(timerID-1);
    else 
      TimerModerator.reset();
  }
  else if(strcmp("FanSensor", cmd) == 0) {
    setFanSensor(payload.toInt());
  }
  else if(strcmp("IQuery", cmd) == 0) {
    IPmoderator.reset();   // force IP params to be sent
  }
  // system info
  else if(strcmp("SQuery", cmd) == 0) {
    SysModerator.reset();   // force MQTT params to be sent
    bTriggerSysParams = true;
  }
  // MQTT parameters
  else if(strcmp("MQuery", cmd) == 0) {
    MQTTJSONmoderator.reset();   // force MQTT params to be sent
  }
  else if(strcmp("MEn", cmd) == 0) {
    sMQTTparams info = NVstore.getMQTTinfo();
    info.enabled = payload.toInt();
    NVstore.setMQTTinfo(info);
  }
  else if(strcmp("MPort", cmd) == 0) {
    sMQTTparams info = NVstore.getMQTTinfo();
    info.port = payload.toInt();
    NVstore.setMQTTinfo(info);
  }
  else if(strcmp("MHost", cmd) == 0) {
    sMQTTparams info = NVstore.getMQTTinfo();
    strncpy(info.host, payload.c_str(), 127);
    info.host[127] = 0;
    NVstore.setMQTTinfo(info);
  }
  else if(strcmp("MUser", cmd) == 0) {
    sMQTTparams info = NVstore.getMQTTinfo();
    strncpy(info.username, payload.c_str(), 31);
    info.username[31] = 0;
    NVstore.setMQTTinfo(info);
  }
  else if(strcmp("MPasswd", cmd) == 0) {
    sMQTTparams info = NVstore.getMQTTinfo();
    strncpy(info.password, payload.c_str(), 31);
    info.password[31] = 0;
    NVstore.setMQTTinfo(info);
  }
  else if(strcmp("MQoS", cmd) == 0) {
    sMQTTparams info = NVstore.getMQTTinfo();
    info.qos = payload.toInt();
    if(INBOUNDS(info.qos, 0, 2)) {
      NVstore.setMQTTinfo(info);
    }
  }
  else if(strcmp("MTopic", cmd) == 0) {
    sMQTTparams info = NVstore.getMQTTinfo();
    strncpy(info.topic, payload.c_str(), 31);
    info.topic[31] = 0;
    NVstore.setMQTTinfo(info);
  }
  else if(strcmp("UploadSize", cmd) == 0) {
    setUploadSize(payload.toInt());
  }
  else if(strcmp("GPout1", cmd) == 0) {
    setGPIOout(0, payload.toInt() ? true : false);
  }
  else if(strcmp("GPout2", cmd) == 0) {
    setGPIOout(1, payload.toInt() ? true : false);
  }
  else if(strcmp("GPin1", cmd) == 0) {
    simulateGPIOin(payload.toInt() ? 0x01 : 0x00);  // simulate key 1 press
  }
  else if(strcmp("GPin2", cmd) == 0) {
    simulateGPIOin(payload.toInt() ? 0x02 : 0x00);  // simulate key 2 press
  }
  else if(strcmp("JSONpack", cmd) == 0) {
    sUserSettings us = NVstore.getUserSettings();
    uint8_t packed = payload.toInt() ? 0x00 : 0x01;
    us.JSON.LF = packed;
    us.JSON.padding = packed;
    us.JSON.singleElement = packed;
    NVstore.setUserSettings(us);
    NVstore.save();
    resetJSONmoderator();
  }
  else if(strcmp("TempMode", cmd) == 0) {
    sUserSettings us = NVstore.getUserSettings();
    us.degF = payload.toInt() ? 0x01 : 0x00;
    NVstore.setUserSettings(us);
    NVstore.save();
  }
  else if(strcmp("PumpCount", cmd) == 0) {  // reset fuel gauge
    int Count = payload.toInt();
    if(Count == 0) {
      resetFuelGauge();
    }
  }
  else if(strcmp("PumpCal", cmd) == 0) {
    sHeaterTuning ht = NVstore.getHeaterTuning();
    ht.pumpCal = payload.toFloat();
    if(INBOUNDS(ht.pumpCal, 0.001, 1)) {
      NVstore.setHeaterTuning(ht);
    }
  }
  else if(strcmp("TempOffset", cmd) == 0) {
    sHeaterTuning ht = NVstore.getHeaterTuning();
    ht.tempOfs = payload.toFloat();
    if(INBOUNDS(ht.tempOfs, -10.0, +10.0)) {
      NVstore.setHeaterTuning(ht);
    }
  }
  else if(strcmp("LowVoltCutout", cmd) == 0) {
    float fCal = payload.toFloat();
    bool bOK = false;
    if(NVstore.getHeaterTuning().sysVoltage == 120)
      bOK |= (fCal == 0) || INBOUNDS(fCal, 10.0, 12.5);
    else
      bOK |= (fCal == 0) || INBOUNDS(fCal, 20.0, 25.0);
    if(bOK) {
      sHeaterTuning ht = NVstore.getHeaterTuning();
      ht.lowVolts = uint8_t(fCal * 10);
      NVstore.setHeaterTuning(ht);
    }
  }
}