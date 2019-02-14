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

#ifndef __BTC_NV_STORAGE_H__
#define __BTC_NV_STORAGE_H__

struct sHeater { 
  uint8_t   Pmin;
  uint8_t   Pmax;
  uint16_t  Fmin;
  uint16_t  Fmax;
  uint8_t   ThermostatMode;
  uint8_t   setTemperature;
  uint8_t   sysVoltage;
  uint8_t   fanSensor;
  uint8_t   glowDrive;

  bool valid() {
    bool retval = true;
    retval &= Pmin < 100;
    retval &= Pmax < 150;
    retval &= Fmin < 5000;
    retval &= Fmax < 6000;
    retval &= ThermostatMode < 2;
    retval &= setTemperature < 40;
    retval &= sysVoltage == 120 || sysVoltage == 240;
    retval &= fanSensor == 1 || fanSensor == 2;
    retval &= glowDrive >= 1 && glowDrive <= 6;
    return retval;
  };
  void init() {
    Pmin = 14;
    Pmax = 45;
    Fmin = 1500;
    Fmax = 4500;
    ThermostatMode = 1;
    setTemperature = 23;
    sysVoltage = 120;
    fanSensor = 1;
    glowDrive = 5;
  };
};

struct sHourMin {
  int8_t hour;
  int8_t min;
  sHourMin() {
    hour = 0;
    min = 0;
  };
  sHourMin& operator=(const sHourMin& rhs) {
    hour = rhs.hour;
    min = rhs.min;
  };
};

struct sTimer {
  sHourMin start;      // start time
  sHourMin stop;       // stop time
  uint8_t enabled;     // timer enabled - each bit is a day of week flag
  uint8_t repeat;      // repeating timer
  uint8_t temperature;
  sTimer() {
    enabled = 0;     
    repeat = false;
    temperature = 22;
  };
  sTimer& operator=(const sTimer& rhs) {
    start = rhs.start;
    stop = rhs.stop;
    enabled = rhs.enabled;
    repeat = rhs.repeat;
    temperature = rhs.temperature;
  };
  void init() {
    start.hour = 0;
    start.min = 0;
    stop.hour = 0;
    stop.min = 0;
    enabled = 0;
    repeat = 0;
    temperature = 22;
  };
  bool valid() {
    bool retval = true;
    retval &= (start.hour >= 0 && start.hour < 24);
    retval &= (start.min >= 0 && start.min < 60);
    retval &= (stop.hour >= 0 && stop.hour < 24);
    retval &= (stop.min >= 0 && stop.min < 60);
    retval &= repeat <= 2;
    retval &= (temperature >= 8 && temperature <= 35);
    return retval;
  };
};

// the actual data stored to NV memory
struct sNVStore {
  sHeater Heater;
  long DimTime;
  uint8_t degF;
  sTimer timer[14];
  bool valid();
  void init();
};

class CNVStorage {
  public:
    CNVStorage() {};
    virtual ~CNVStorage() {};
    virtual void init() = 0;
    virtual void load() = 0;
    virtual void save() = 0;
};


class CHeaterStorage : public CNVStorage {
protected:
  sNVStore _calValues;
public:
    CHeaterStorage();
    virtual ~CHeaterStorage() {};

  // TODO: These are only here to allow building without fully 
  // fleshing out NV storage for Due, Mega etc
  // these should be removed once complete (pure virtual)
  void init() {};
  void load() {};
  void save() {};


    float getPmin();
    float getPmax();
    unsigned short getFmin();
    unsigned short getFmax();
    unsigned char getDesiredTemperature();
    unsigned char getThermostatMode();
    unsigned char getSysVoltage();
    unsigned char getFanSensor();
    unsigned char getGlowDrive();
    unsigned long getDimTime();
    unsigned char getDegFMode();

    void setPmin(float);
    void setPmax(float);
    void setFmin(unsigned short val);
    void setFmax(unsigned short val);
    void setDesiredTemperature(unsigned char val);
    void setThermostatMode(unsigned char val);
    void setSystemVoltage(float fVal);
    void setFanSensor(unsigned char val);
    void setGlowDrive(unsigned char val);
    void setDimTime(unsigned long val);
    void setDegFMode(unsigned char val);

    void getTimerInfo(int idx, sTimer& timerInfo);
    void setTimerInfo(int idx, const sTimer& timerInfo);
};


#ifdef ESP32

#include <Preferences.h>
#include <functional>

class CESP32HeaterStorage : public CHeaterStorage {
  Preferences preferences;
public:
  CESP32HeaterStorage();
  virtual ~CESP32HeaterStorage();
  void init();
  void load();
  void save();
  void loadHeater();
  void saveHeater();
  void loadTimer(int idx);
  void saveTimer(int idx);
  void loadUI();
  void saveUI();
  bool validatedLoad(const char* key, int8_t& val, int defVal, std::function<bool(int8_t, int8_t, int8_t)> validator, int min, int max);
  bool validatedLoad(const char* key, uint8_t& val, int defVal, std::function<bool(uint8_t, uint8_t, uint8_t)> validator, int min, int max);
  bool validatedLoad(const char* key, uint16_t& val, int defVal, std::function<bool(uint16_t, uint16_t, uint16_t)> validator, int min, int max);
  bool validatedLoad(const char* key, long& val, long defVal, std::function<bool(long, long, long)> validator, long min, long max);
};

#endif

extern CHeaterStorage& NVstore;

#endif // __BTC_NV_STORAGE_H__

