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
#include "../Protocol/Protocol.h"
#include "UtilClasses.h"
#include "NVStorage.h"
#include "HourMeter.h"
#include "macros.h"
#include "BTC_JSON.h"


// a class to track the blue wire receive / transmit states
// class CommStates 

void 
CommStates::set(eCS eState) 
{
  _State = eState;
  _Count = 0;
  if(_report) {
   static const char* stateNames[] = { 
     "Idle", "OEMCtrlRx", "OEMCtrlValidate", "HeaterRx1", "HeaterValidate1", "HeaterReport1", 
     "BTC_Tx", "HeaterRx2", "HeaterValidate2", "HeaterReport2", "TemperatureRead" 
    };
    if(_State == Idle) DebugPort.println("");  // clear screen
    DebugPort.printf("State: %s\r\n", stateNames[_State]);
  }
}

bool 
CommStates::collectData(CProtocol& Frame, uint8_t val, int limit) {   // returns true when buffer filled
  Frame.Data[_Count++] = val;
  return _Count >= limit;
}

bool 
CommStates::collectDataEx(CProtocol& Frame, uint8_t val, int limit) {   // returns true when buffer filled
  // guarding against rogue rx kernel buffer stutters....
  if((_Count == 0) && (val != 0x76)) {
    DebugPort.println("First heater byte not 0x76 - SKIPPING");
    return false;
  }
  Frame.Data[_Count++] = val;
  return _Count >= limit;
}

bool 
CommStates::checkValidStart(uint8_t val)
{
  if(_Count) 
    return true;
  else 
    return val == 0x76;
}

void
CommStates::setDelay(int ms)
{
  _delay = millis() + ms;
}

bool 
CommStates::delayExpired()
{
  long test = millis() - _delay;
  return(test >= 0);
}

CProfile::CProfile()
{
  tStart = millis();
}

unsigned long 
CProfile::elapsed(bool reset/* = false*/)
{
  unsigned long now = millis();
  unsigned long retval = now - tStart;
  if(reset)
    tStart = now;

  return retval;
}

void DecodeCmd(const char* cmd, String& payload) 
{
  if(strcmp("TempDesired", cmd) == 0) {
    if( !reqDemand(payload.toInt(), false) ) {  // this request is blocked if OEM controller active
      resetJSONmoderator("TempDesired");
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
      resetJSONmoderator("ThermoStat");   
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
    triggerJSONTimeUpdate();
  }
  else if(strcmp("Date", cmd) == 0) {
    setDate(payload.c_str());
    triggerJSONTimeUpdate();
  }
  else if(strcmp("Time", cmd) == 0) {
    setTime(payload.c_str());
    triggerJSONTimeUpdate();
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
    resetAllJSONmoderators();
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
    resetJSONTimerModerator(payload.toInt());
  }
  else if(strcmp("FanSensor", cmd) == 0) {
    setFanSensor(payload.toInt());
  }
  else if(strcmp("IQuery", cmd) == 0) {
    resetJSONIPmoderator();   // force IP params to be sent
  }
  // system info
  else if(strcmp("SQuery", cmd) == 0) {
    resetJSONSysModerator();   // force system params to be sent
  }
  // MQTT parameters
  else if(strcmp("MQuery", cmd) == 0) {
    resetJSONMQTTmoderator();  // force MQTT params to be sent
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
    resetAllJSONmoderators();
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
  else if(strcmp("SMenu", cmd) == 0) {
    sUserSettings us = NVstore.getUserSettings();
    us.menuMode = payload.toInt();
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
  else if(strcmp("SysHourMeters", cmd) == 0) {
    pHourMeter->resetHard();
  }
}

