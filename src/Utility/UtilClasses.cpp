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
#include "../WiFi/BTCWebServer.h"
#include "FuelGauge.h"
#include "DemandManager.h"

// a class to track the blue wire receive / transmit states
// class CommStates 

void 
CommStates::set(eCS eState) 
{
  _State = eState;
  _Count = 0;
  if(_report && _callback != NULL) {
   static const char* stateNames[] = { 
     "Idle", "OEMCtrlRx", "OEMCtrlValidate", "HeaterRx1", "HeaterValidate1", "TxStart", 
     "TxInterval", "HeaterRx2", "HeaterValidate2", "ExchangeComplete" 
    };
    if(_State == Idle) 
      _callback("\r\n");  
    char msg[32];
    sprintf(msg, "State: %s\r\n", stateNames[_State]);
    _callback(msg);
  }
}

bool 
CommStates::collectData(CProtocol& Frame, uint8_t val, int limit) {   // returns true when buffer filled
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
  int val;
  if(strcmp("TempDesired", cmd) == 0) {
    if( !CDemandManager::setDemand(payload.toInt()) ) {  // this request is blocked if OEM controller active
      resetJSONmoderator("TempDesired");
    }
  }
/*
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
*/
  else if((strcmp("RunState", cmd) == 0) || (strcmp("Run", cmd) == 0)) {
    refreshMQTT();
    if(payload.toInt()) {
      CDemandManager::eStartCode result = requestOn();
      switch(result) {
        case CDemandManager::eStartOK:       sendJSONtext("{\"StartString\":\"\"}", true); break;
        case CDemandManager::eStartTooWarm:  sendJSONtext("{\"StartString\":\"Ambient too warm!\"}", true); break;
        case CDemandManager::eStartSuspend:  sendJSONtext("{\"StartString\":\"Immediate Cyclic suspension!\"}", true); break;
        case CDemandManager::eStartLVC:      sendJSONtext("{\"StartString\":\"Battery below LVC!\"}", true); break;
        case CDemandManager::eStartLowFuel:  sendJSONtext("{\"StartString\":\"Fuel Empty!\"}", true); break;
      }
    }
    else {
      requestOff();
    }
  }
  else if(strcmp("PumpMin", cmd) == 0) {
    sHeaterTuning tuning = NVstore.getHeaterTuning();
    tuning.setPmin(payload.toFloat());
    NVstore.setHeaterTuning(tuning);
  }
  else if(strcmp("PumpMax", cmd) == 0) {
    sHeaterTuning tuning = NVstore.getHeaterTuning();
    tuning.setPmax(payload.toFloat());
    NVstore.setHeaterTuning(tuning);
  }
  else if(strcmp("FanMin", cmd) == 0) {
    sHeaterTuning tuning = NVstore.getHeaterTuning();
    if(INBOUNDS(payload.toInt(), 500, 5000))
    tuning.setFmin(payload.toInt());
    NVstore.setHeaterTuning(tuning);
  }
  else if(strcmp("FanMax", cmd) == 0) {
    sHeaterTuning tuning = NVstore.getHeaterTuning();
    if(INBOUNDS(payload.toInt(), 500, 5000))
      tuning.setFmax(payload.toInt());
    NVstore.setHeaterTuning(tuning);
  }
  else if(strcmp("CyclicTemp", cmd) == 0) {
    CDemandManager::setDegC(payload.toInt());  // directly set demandDegC
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
    if(INBOUNDS(settings.ThermostatMethod, 0, 4))
      NVstore.setUserSettings(settings);
  }
  else if(strcmp("ThermostatWindow", cmd) == 0) {
    sUserSettings settings = NVstore.getUserSettings();
    settings.ThermostatWindow = payload.toFloat();
    if(INBOUNDS(settings.ThermostatWindow, 0.2f, 10.f))
      NVstore.setUserSettings(settings);
  }
  else if(strcmp("Thermostat", cmd) == 0) {
    if(!CDemandManager::setThermostatMode(payload.toInt(), false)) {  // this request is blocked if OEM controller active
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
      NVstore.save();
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
    sHeaterTuning tuning = NVstore.getHeaterTuning();
    tuning.setSysVoltage(payload.toFloat());
    NVstore.setHeaterTuning(tuning);
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
    sHeaterTuning tuning = NVstore.getHeaterTuning();
    tuning.setFanSensor(payload.toInt());
    NVstore.setHeaterTuning(tuning);
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
#ifdef ALLOW_USER_TOPIC
  else if(strcmp("MTopic", cmd) == 0) {
    sMQTTparams info = NVstore.getMQTTinfo();
    strncpy(info.topicPrefix, payload.c_str(), 31);
    info.topicPrefix[31] = 0;
    NVstore.setMQTTinfo(info);
  }
#endif
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
  else if(strcmp("GPOutThr1", cmd) == 0) {
    sUserSettings us = NVstore.getUserSettings();
    us.GPIO.thresh[0] = payload.toInt();   // 0 = disable; < 0, active if less than abs(val); > 0, active if over val
    if(INBOUNDS(us.GPIO.thresh[0], -50, 50)) {
      NVstore.setUserSettings(us);
      NVstore.save();
    }
  }
  else if(strcmp("GPOutThr2", cmd) == 0) {
    sUserSettings us = NVstore.getUserSettings();
    us.GPIO.thresh[1] = payload.toInt();   // 0 = disable; < 0, active if less than abs(val); > 0, active if over val
    if(INBOUNDS(us.GPIO.thresh[1], -50, 50)) {
      NVstore.setUserSettings(us);
      NVstore.save();
    }
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
    getTempSensor().setOffset(0, payload.toFloat());
/*    sHeaterTuning ht = NVstore.getHeaterTuning();
    ht.DS18B20probe[0].offset = payload.toFloat();
    if(INBOUNDS(ht.DS18B20probe[0].offset, -10.0, +10.0)) {
      NVstore.setHeaterTuning(ht);
    }*/
  }
  else if(strcmp("Temp2Offset", cmd) == 0) {
    getTempSensor().setOffset(1, payload.toFloat());
/*    sHeaterTuning ht = NVstore.getHeaterTuning();
    ht.DS18B20probe[1].offset = payload.toFloat();
    if(INBOUNDS(ht.DS18B20probe[1].offset, -10.0, +10.0)) {
      NVstore.setHeaterTuning(ht);
    }*/
  }
  else if(strcmp("Temp3Offset", cmd) == 0) {
    getTempSensor().setOffset(2, payload.toFloat());
/*    sHeaterTuning ht = NVstore.getHeaterTuning();
    ht.DS18B20probe[2].offset = payload.toFloat();
    if(INBOUNDS(ht.DS18B20probe[2].offset, -10.0, +10.0)) {
      NVstore.setHeaterTuning(ht);
    }*/
  }
  else if(strcmp("Temp4Offset", cmd) == 0) {
    getTempSensor().setOffset(3, payload.toFloat());
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
  else if(strcmp("FrostOn", cmd) == 0) {
    sUserSettings us = NVstore.getUserSettings();
    us.FrostOn = payload.toInt();
    if(INBOUNDS(us.FrostOn, 0, 30)) {
      NVstore.setUserSettings(us);
      NVstore.save();
    }
  }
  else if(strcmp("FrostRise", cmd) == 0) {
    sUserSettings us = NVstore.getUserSettings();
    us.FrostRise = payload.toInt();
    if(INBOUNDS(us.FrostRise, 0, 30)) {
      NVstore.setUserSettings(us);
      NVstore.save();
    }
  }
  else if(strcmp("HumidStart", cmd) == 0) {
    sUserSettings us = NVstore.getUserSettings();
    us.humidityStart = payload.toInt();
    if((us.humidityStart == 0) || INBOUNDS(us.humidityStart, 50, 100)) {
      NVstore.setUserSettings(us);
      NVstore.save();
    }
  }
  else if(strcmp("Reboot", cmd) == 0) {
    int16_t code = payload.toInt();
    doJSONreboot(code);
  }
  else if(strcmp("LoadWebContent", cmd) == 0) {
    getWebContent(true);
  }
  // TESTO hook - make sure removed for production
  else if(strcmp("testo", cmd) == 0) {
    val = payload.toInt();
    FuelGauge.init(val);
    DebugPort.printf("Set Fuel usage to %d => %f\r\n", val, FuelGauge.Used_mL());
  }
}

void setHoldoff(unsigned long& holdoff, unsigned long period)
{
  holdoff = (millis() + period) | 1;
}

void hexDump(uint8_t* pData, int len, int wrap) 
{
  for(int i=0; i<len; ) {
    for(int j=0; j<wrap && i<len; j++) {
      DebugPort.printf("%02X ", pData[i++]);
    }
    DebugPort.println("");
  }
}
