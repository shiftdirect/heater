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

#ifndef __BTC_NV_STORAGE_H__
#define __BTC_NV_STORAGE_H__

#include "BTC_GPIO.h"
#include "NVCore.h"
#include "helpers.h"
#include "TempSense.h"

#include "../RTC/Timers.h"   // for sTimer

void toggle(bool& ref);
void toggle(uint8_t& ref);

struct sDS18B20ProbeTuning { 
  float offset;
  OneWireBus_ROMCode romCode;
};

struct sBM280tuning {
  float offset;
  uint8_t bPrimary;
};

struct sHeaterTuning : public CESP32_NVStorage {
  uint8_t   Pmin;
  uint8_t   Pmax;
  uint16_t  Fmin;
  uint16_t  Fmax;
  uint8_t   sysVoltage;  // x10 
  uint8_t   fanSensor;
  uint8_t   glowDrive;
  uint8_t   lowVolts;    // x10
  float     pumpCal;
  sDS18B20ProbeTuning DS18B20probe[3];   // [0],[1],[2] - Primary, Secondary, Tertiary
  sBM280tuning  BME280probe;

  bool valid() {
    bool retval = true;
    retval &= Pmin < 100;
    retval &= Pmax < 150;
    retval &= Fmin < 5000;
    retval &= Fmax < 6000;
    retval &= sysVoltage == 120 || sysVoltage == 240;
    retval &= fanSensor == 1 || fanSensor == 2;
    retval &= INBOUNDS(glowDrive, 1, 6);
    retval &= INBOUNDS(pumpCal, 0.001, 1.0);
    if(sysVoltage == 120)
      retval &= INBOUNDS(lowVolts, 100, 125) || (lowVolts == 0);
    else 
      retval &= INBOUNDS(lowVolts, 200, 250 || (lowVolts == 0));
    retval &= INBOUNDS(DS18B20probe[0].offset, -10, +10);
    retval &= INBOUNDS(DS18B20probe[1].offset, -10, +10);
    retval &= INBOUNDS(DS18B20probe[2].offset, -10, +10);
    retval &= INBOUNDS(BME280probe.offset, -10, +10);
    return retval;
  };
  void init() {
    Pmin = 14;
    Pmax = 45;
    Fmin = 1500;
    Fmax = 4500;
    sysVoltage = 120;
    fanSensor = 1;
    glowDrive = 5;
    pumpCal = 0.02;
    lowVolts = 115;
    DS18B20probe[0].offset = 0;
    DS18B20probe[1].offset = 0;
    DS18B20probe[2].offset = 0;
    memset(DS18B20probe[0].romCode.bytes, 0, sizeof(DS18B20probe[0].romCode));
    memset(DS18B20probe[1].romCode.bytes, 0, sizeof(DS18B20probe[1].romCode));
    memset(DS18B20probe[2].romCode.bytes, 0, sizeof(DS18B20probe[2].romCode));
    BME280probe.offset = 0;
    BME280probe.bPrimary = false;
  };
  void load();
  void save();
  sHeaterTuning& operator=(const sHeaterTuning& rhs) {
    Pmin = rhs.Pmin;
    Pmax = rhs.Pmax;
    Fmin = rhs.Fmin;
    Fmax = rhs.Fmax;
    sysVoltage = rhs.sysVoltage;
    fanSensor = rhs.fanSensor;
    glowDrive = rhs.glowDrive;
    pumpCal = rhs.pumpCal;
    lowVolts = rhs.lowVolts;
    DS18B20probe[0].offset = rhs.DS18B20probe[0].offset;
    DS18B20probe[1].offset = rhs.DS18B20probe[1].offset;
    DS18B20probe[2].offset = rhs.DS18B20probe[2].offset;
    memcpy(DS18B20probe[0].romCode.bytes, rhs.DS18B20probe[0].romCode.bytes, 8);
    memcpy(DS18B20probe[1].romCode.bytes, rhs.DS18B20probe[1].romCode.bytes, 8);
    memcpy(DS18B20probe[2].romCode.bytes, rhs.DS18B20probe[2].romCode.bytes, 8);
    BME280probe.offset = rhs.BME280probe.offset;
    BME280probe.bPrimary = rhs.BME280probe.bPrimary;
    return *this;
  }
  float getPmin() const;
  float getPmax() const;
  void setPmin(float val);
  void setPmax(float val);
  void setSysVoltage(float val);
  float getLVC() const;
};

struct sHomeMenuActions {
  uint8_t onTimeout;
  uint8_t onStart;
  uint8_t onStop;
  bool valid() {
    bool retval = true;
    retval &= onTimeout < 4;
    retval &= onStart < 4;
    retval &= onStop < 4;
    return retval;  
  }
  void init() {
    onTimeout = 0;
    onStart = 0;
    onStop = 0;
  }
  sHomeMenuActions& operator=(const sHomeMenuActions& rhs) {
    onTimeout = rhs.onTimeout;
    onStart = rhs.onStart;
    onStop = rhs.onStop;
    return *this;
  }
};

struct sHourMeter : public CESP32_NVStorage {
  uint32_t RunTime;
  uint32_t GlowTime;
  void init() {
    load();
  }
  void load();
  void save();
  sHourMeter& operator=(const sHourMeter& rhs) {
    RunTime = rhs.RunTime;
    GlowTime = rhs.GlowTime;
    return *this;
  }
  bool operator!=(const sHourMeter& rhs) const {
    bool retval = false;
    retval |= RunTime != rhs.RunTime;
    retval |= GlowTime != rhs.GlowTime;
    return retval;
  }
};

struct sCyclicThermostat {
  int8_t Stop;
  int8_t Start;
  bool valid() {
    bool retval = true;
    retval &= INBOUNDS(Start, -10, 0);
    retval &= INBOUNDS(Stop, 0, 10);
    return retval;  
  }
  void init() {
    Start = -1;
    Stop = 0;
  }
  bool isEnabled() const {
    return Stop != 0;
  }
  sCyclicThermostat& operator=(const sCyclicThermostat& rhs) {
    Stop = rhs.Stop;
    Start = rhs.Start;
    return *this;
  }
};


struct sJSONoptions {
  uint8_t singleElement;
  uint8_t LF;
  uint8_t padding;
  bool valid() {
    bool retval = true;
    retval &= singleElement <= 1;
    retval &= LF <= 1;
    retval &= padding <= 1;
    return retval;  
  }
  void init() {
    singleElement = 0;
    LF = 0;
    padding = 0;
  }
  sJSONoptions& operator=(const sJSONoptions& rhs) {
    singleElement = rhs.singleElement;
    LF = rhs.LF;
    padding = rhs.padding;
    return *this;
  }
};


struct sCredentials : public CESP32_NVStorage {
  char SSID[32];
  char APpassword[32];
  char webUpdateUsername[32];
  char webUpdatePassword[32];
  void init() {
    strcpy(SSID, "Afterburner");
    strcpy(APpassword, "thereisnospoon");
    strcpy(webUpdateUsername, "Afterburner");
    strcpy(webUpdatePassword, "BurnBabyBurn");
  };
  void load();
  void save();
  bool valid();
  sCredentials& operator=(const sCredentials& rhs) {
    strcpy(SSID, rhs.SSID);
    strcpy(APpassword, rhs.APpassword);
    strcpy(webUpdateUsername, rhs.webUpdateUsername);
    strcpy(webUpdatePassword, rhs.webUpdatePassword);
    return *this;
  }
};

struct sMQTTparams : public CESP32_NVStorage {
  uint8_t enabled;
  uint16_t  port;
  uint8_t qos;
  char host[128];
  char username[32];
  char password[32];
  char topicPrefix[32];
  void init() {
    enabled = false;
    port = 1883;
    qos = 0;
    memset(host, 0, 128);
    memset(username, 0, 32);
    memset(password, 0, 32);
    memset(topicPrefix, 0, 32);
  }
  sMQTTparams& operator=(const sMQTTparams& rhs) {
    enabled = rhs.enabled;
    port = rhs.port;
    qos = rhs.qos;
    memcpy(host, rhs.host, 128);
    memcpy(username, rhs.username, 32);
    memcpy(password, rhs.password, 32);
    memcpy(topicPrefix, rhs.topicPrefix, 32);
    host[127] = 0;
    username[31] = 0;
    password[31] = 0;
    topicPrefix[31] = 0;
    return *this;
  }
  void load();
  void save();
  bool valid();
};

struct sUserSettings : public CESP32_NVStorage {
  long dimTime;
  long menuTimeout;
  long ExtThermoTimeout;
  uint8_t degF;
  uint8_t ThermostatMethod;  // 0: standard heater, 1: Narrow Hysterisis, 2:Managed Hz mode
  float   ThermostatWindow;   
  uint8_t FrostOn;
  uint8_t FrostRise;
  uint8_t useThermostat;
//  uint8_t enableWifi;
  uint8_t enableOTA;
  uint8_t wifiMode;  
  uint16_t FrameRate;
  sCyclicThermostat cyclic;
  sHomeMenuActions HomeMenu;
  sGPIOparams GPIO;
  sJSONoptions JSON;
  uint8_t menuMode;  // 0 normal, 1, basic, 2 no heater
  uint8_t clock12hr;
  uint8_t holdPassword;
  uint8_t humidityStart;

  bool valid() {
    bool retval = true;
    retval &= INBOUNDS(dimTime, -600000, 600000);  // +/- 10 mins
    retval &= INBOUNDS(menuTimeout, 0, 300000);  // 5 mins
    retval &= INBOUNDS(ExtThermoTimeout, 0, 3600000); // 1 hour
    retval &= (degF == 0) || (degF == 1);
    retval &= ThermostatMethod <= 3;  // only modes 0, 1 or 2, 3
    retval &= INBOUNDS(ThermostatWindow, 0.2f, 10.f);
    retval &= useThermostat < 2;
//    retval &= (enableWifi == 0) || (enableWifi == 1);
    retval &= INBOUNDS(wifiMode, 0, 3);
    retval &= (enableOTA == 0) || (enableOTA == 1);
    retval &= GPIO.in1Mode < 4;
    retval &= GPIO.in2Mode < 3;
    retval &= GPIO.out1Mode < 3;
    retval &= GPIO.out2Mode < 2;
    retval &= INBOUNDS(FrameRate, 300, 1500);
    retval &= cyclic.valid();
    retval &= HomeMenu.valid();
    retval &= JSON.valid();
    return retval;  
  }
  void init() {
    dimTime = 60000;
    menuTimeout = 60000;
    ExtThermoTimeout = 0;
    degF = 0;
    ThermostatMethod = 0;
    ThermostatWindow = 1.0;
    FrostOn = 0;
    FrostRise = 5;
    useThermostat = 1;
//    enableWifi = 1;
    wifiMode = 1;
    enableOTA = 0;
    GPIO.in1Mode = CGPIOin1::Disabled;
    GPIO.in2Mode = CGPIOin2::Disabled;
    GPIO.out1Mode = CGPIOout1::Disabled;
    GPIO.out2Mode = CGPIOout2::Disabled;
    GPIO.algMode = CGPIOalg::Disabled;
    GPIO.thresh[0] = 0;
    GPIO.thresh[1] = 0;
    FrameRate = 1000;
    cyclic.init();
    HomeMenu.init();
    JSON.init();
    menuMode = 0;
    clock12hr = 0;
    holdPassword = 0;
    humidityStart = 0;
  };
  void load();
  void save();
  sUserSettings& operator=(const sUserSettings& rhs) {
    dimTime = rhs.dimTime;
    menuTimeout = rhs.menuTimeout;
    ExtThermoTimeout = rhs.ExtThermoTimeout;
    degF = rhs.degF;
    ThermostatMethod = rhs.ThermostatMethod;
    ThermostatWindow = rhs.ThermostatWindow;
    FrostOn = rhs.FrostOn;
    FrostRise = rhs.FrostRise;
    useThermostat = rhs.useThermostat;
//    enableWifi = rhs.enableWifi;
    wifiMode = rhs.wifiMode;
    enableOTA = rhs.enableOTA;
    GPIO.in1Mode = rhs.GPIO.in1Mode;
    GPIO.in2Mode = rhs.GPIO.in2Mode;
    GPIO.out1Mode = rhs.GPIO.out1Mode;
    GPIO.out2Mode = rhs.GPIO.out2Mode;
    GPIO.algMode = rhs.GPIO.algMode;
    GPIO.thresh[0] = rhs.GPIO.thresh[0];
    GPIO.thresh[1] = rhs.GPIO.thresh[1];
    FrameRate = rhs.FrameRate;
    cyclic = rhs.cyclic;
    HomeMenu = rhs.HomeMenu;
    JSON = rhs.JSON;
    menuMode = rhs.menuMode;
    clock12hr = rhs.clock12hr;
    holdPassword = rhs.holdPassword;
    humidityStart = rhs.humidityStart;
    return *this;
  }
};

// the actual data stored to NV memory
struct sNVStore {
  sHeaterTuning heaterTuning;
  sUserSettings userSettings;
  sTimer timer[14];
  sMQTTparams MQTT;
  sCredentials Credentials;
  sHourMeter hourMeter;
  bool valid();
  void init();
  sNVStore& operator=(const sNVStore& rhs) {
    heaterTuning = rhs.heaterTuning;
    userSettings = rhs.userSettings;
    for(int i = 0; i < 14; i++)
      timer[i] = rhs.timer[i];
    MQTT = rhs.MQTT;
    Credentials = rhs.Credentials;
    hourMeter = rhs.hourMeter;
    return *this;
  }
};


class CHeaterStorage /*: public CESP32_NVStorage*/ {
protected:
  sNVStore _calValues;
public:
  CHeaterStorage();
  virtual ~CHeaterStorage() {};

// TODO: These are only here to allow building without fully 
// fleshing out NV storage for Due, Mega etc
// these should be removed once complete (pure virtual)
  virtual void init() {};
  virtual void load() {};
  virtual void save() {};
  virtual void doSave() {}


  const sMQTTparams& getMQTTinfo() const;
  const sCredentials& getCredentials() const;
  const sUserSettings& getUserSettings() const;
  const sHeaterTuning& getHeaterTuning() const;
  const sHourMeter& getHourMeter() const;

  void getTimerInfo(int idx, sTimer& timerInfo);
  void setTimerInfo(int idx, const sTimer& timerInfo);
  void setMQTTinfo(const sMQTTparams& info);
  void setCredentials(const sCredentials& info);
  CHeaterStorage& operator=(const CHeaterStorage& rhs) {
    _calValues = rhs._calValues;
    return *this;
  }
  void setUserSettings(const sUserSettings& info);
  void setHeaterTuning(const sHeaterTuning& info);
  bool setHourMeter(const sHourMeter& info);
};



class CESP32HeaterStorage : public CHeaterStorage {
  bool _bShouldSave;
public:
  CESP32HeaterStorage();
  virtual ~CESP32HeaterStorage();
  void init();
  void load();
  void save();
  void doSave();
};

extern CHeaterStorage& NVstore;

#endif // __BTC_NV_STORAGE_H__

