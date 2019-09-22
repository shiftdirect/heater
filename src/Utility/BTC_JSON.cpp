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
#include "Utility/TempSense.h"

extern CTempSense TempSensor;
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

void Expand(std::string& str);
bool makeJSONString(CModerator& moderator, char* opStr, int len);
bool makeJSONStringEx(CModerator& moderator, char* opStr, int len);
bool makeJSONTimerString(int channel, char* opStr, int len);
bool makeJSONStringGPIO( CModerator& moderator, char* opStr, int len);
bool makeJSONStringSysInfo(CModerator& moderator, char* opStr, int len);
bool makeJSONStringMQTT(CModerator& moderator, char* opStr, int len);
bool makeJSONStringIP(CModerator& moderator, char* opStr, int len);
void DecodeCmd(const char* cmd, String& payload);

void triggerJSONTimeUpdate()
{
  bTriggerDateTime = true;
}

void resetJSONTimerModerator(int timerID)
{
  if(timerID)
    TimerModerator.reset(timerID-1);
  else 
    TimerModerator.reset();
}

void resetJSONIPmoderator()
{
  IPmoderator.reset();   // force IP params to be sent
}

void resetJSONmoderator(const char* name)
{
  if(name) 
    JSONmoderator.reset(name);
  else
    JSONmoderator.reset();
}

void resetJSONSysModerator()
{
  SysModerator.reset();   // force MQTT params to be sent
  bTriggerSysParams = true;
}

void resetJSONMQTTmoderator()   
{
  MQTTJSONmoderator.reset();   // force MQTT params to be sent
}

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
  if(TempSensor.getNumSensors() > 1) {
    TempSensor.getTemperature(1, tidyTemp);
    tidyTemp += NVstore.getHeaterTuning().tempProbe[1].offset;
    tidyTemp = int(tidyTemp * 10 + 0.5) * 0.1f;  // round to 0.1 resolution 
    if(tidyTemp > -80) {
	    bSend |= moderator.addJson("Temp2Current", tidyTemp, root); 
    }
    if(TempSensor.getNumSensors() > 2) {
      TempSensor.getTemperature(2, tidyTemp);
      tidyTemp += NVstore.getHeaterTuning().tempProbe[2].offset;
      tidyTemp = int(tidyTemp * 10 + 0.5) * 0.1f;  // round to 0.1 resolution 
      if(tidyTemp > -80) {
	      bSend |= moderator.addJson("Temp3Current", tidyTemp, root); 
      }
    }
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
    bSend |= moderator.addJson("LowVoltCutout", NVstore.getHeaterTuning().getLVC(), root); // low voltage cutout
  }
  bSend |= moderator.addJson("TempOffset", NVstore.getHeaterTuning().tempProbe[0].offset, root);     // degC offset
  if(TempSensor.getNumSensors() > 1) {
    bSend |= moderator.addJson("Temp2Offset", NVstore.getHeaterTuning().tempProbe[1].offset, root);     // degC offset
    if(TempSensor.getNumSensors() > 2) 
      bSend |= moderator.addJson("Temp3Offset", NVstore.getHeaterTuning().tempProbe[2].offset, root);     // degC offset
  }

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


void resetAllJSONmoderators()
{
  JSONmoderator.reset();
#ifdef SALWAYS_SEND_TIMERS
  TimerModerator.reset();
#else
  initJSONTimermoderator();
#endif
  initJSONMQTTmoderator();
  initJSONIPmoderator();
  initJSONSysModerator();
  GPIOmoderator.reset();
}

void initJSONMQTTmoderator()
{
  char jsonStr[800];
  makeJSONStringMQTT(MQTTJSONmoderator, jsonStr, sizeof(jsonStr));
}

void initJSONIPmoderator()
{
  char jsonStr[800];
  makeJSONStringIP(IPmoderator, jsonStr, sizeof(jsonStr));
}

void initJSONSysModerator()
{
  char jsonStr[800];
  makeJSONStringSysInfo(SysModerator, jsonStr, sizeof(jsonStr));
}

void initJSONTimermoderator()
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

