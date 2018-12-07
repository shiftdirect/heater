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
  uint8_t enabled;     // timer enabled
  uint8_t repeat;      // repeating timer
  sTimer() {
    enabled = false;
    repeat = false;
  };
  sTimer& operator=(const sTimer& rhs) {
    start = rhs.start;
    stop = rhs.stop;
    enabled = rhs.enabled;
    repeat = rhs.repeat;
  };
};

// the actual data stored to NV memory
struct sNVStore {
  sHeater Heater;
  sTimer timer[2];
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
    unsigned char getTemperature();
    unsigned char getThermostatMode();

    void setPmin(float);
    void setPmax(float);
    void setFmin(unsigned short val);
    void setFmax(unsigned short val);
    void setTemperature(unsigned char val);
    void setThermostatMode(unsigned char val);

    void getTimerInfo(int idx, sTimer& timerInfo);
    void setTimerInfo(int idx, const sTimer& timerInfo);
};


#ifdef ESP32

#include <Preferences.h>

class CESP32HeaterStorage : public CHeaterStorage {
  Preferences preferences;
public:
  CESP32HeaterStorage();
  virtual ~CESP32HeaterStorage();
  void init();
  void load();
  void save();
};

#endif

extern CHeaterStorage& NVstore;

#endif // __BTC_NV_STORAGE_H__

