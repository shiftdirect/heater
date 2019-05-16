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
#include "NVStorage.h"
#include "DebugPort.h"
#include <driver/adc.h>


bool 
sNVStore::valid()
{
  bool retval = true;
  retval &= Heater.valid();
  retval &= Options.valid();
  for(int i=0; i<2; i++) {
    retval &= timer[i].valid();
  }
  retval &= MQTT.valid();
  retval &= Credentials.valid();
  return retval;  
}

void 
sNVStore::init()
{
  Heater.init();
  Options.init();
  for(int i=0; i<2; i++) {
    timer[i].init();
  }
  MQTT.init();
  Credentials.init();
}

CHeaterStorage::CHeaterStorage()
{
  _calValues.init();
}

float
CHeaterStorage::getPmin()
{
  return float(_calValues.Heater.Pmin) * 0.1f;
}

float
CHeaterStorage::getPmax()
{
  return float(_calValues.Heater.Pmax) * 0.1f;
}

unsigned short
CHeaterStorage::getFmin()
{
  return _calValues.Heater.Fmin;
}

unsigned short
CHeaterStorage::getFmax()
{
  return _calValues.Heater.Fmax;
}

unsigned char
CHeaterStorage::getDesiredTemperature()
{
  return _calValues.Heater.setTemperature;
}

unsigned char
CHeaterStorage::getThermostatMode()
{
  return _calValues.Heater.ThermostatMode;
}

unsigned char 
CHeaterStorage::getThermostatMethodMode()
{
  return _calValues.Options.ThermostatMethod & 0x03;
}

float
CHeaterStorage::getThermostatMethodWindow()
{
  return float((_calValues.Options.ThermostatMethod >> 2) & 0x3f) * 0.1f;  // top 5 bits / 10, then / 2
}

void
CHeaterStorage::setPmin(float val)
{
  uint8_t cVal = (uint8_t)(val * 10.f + 0.5f);
  _calValues.Heater.Pmin = cVal;
}

void
CHeaterStorage::setPmax(float val)
{
  uint8_t cVal = (uint8_t)(val * 10.f + 0.5f);
  _calValues.Heater.Pmax = cVal;
}

void
CHeaterStorage::setFmin(unsigned short val)
{
  _calValues.Heater.Fmin = val;
}

void
CHeaterStorage::setFmax(unsigned short val)
{
  _calValues.Heater.Fmax = val;
}

void
CHeaterStorage::setDesiredTemperature(unsigned char val)
{
  _calValues.Heater.setTemperature = val;
}

void
CHeaterStorage::setThermostatMode(unsigned char val)
{
  _calValues.Heater.ThermostatMode = val;
}

void
CHeaterStorage::setThermostatMethodMode(unsigned char val)
{
  _calValues.Options.ThermostatMethod &= ~0x03;
  _calValues.Options.ThermostatMethod |= (val & 0x03);
}

void
CHeaterStorage::setThermostatMethodWindow(float val)
{
  _calValues.Options.ThermostatMethod &= 0x03;
  int nVal = int(val * 10 + 0.5);
  _calValues.Options.ThermostatMethod |= ((nVal & 0x3F) << 2);
}


void 
CHeaterStorage::setSystemVoltage(float fVal)
{
  int val = int(fVal * 10.0);
  if(val == 120 || val == 240) {
    _calValues.Heater.sysVoltage = val;
  }
}

unsigned char
CHeaterStorage::getSysVoltage()
{
  return _calValues.Heater.sysVoltage;
}

void
CHeaterStorage::setFanSensor(unsigned char val)
{
  if(val == 2)
    _calValues.Heater.fanSensor = 2;
  else 
    _calValues.Heater.fanSensor = 1;
}

unsigned char
CHeaterStorage::getFanSensor()
{
  return _calValues.Heater.fanSensor;
}

void
CHeaterStorage::setGlowDrive(unsigned char val)
{
  if(val >=1 && val <= 6)
    _calValues.Heater.glowDrive = val;
  else 
    _calValues.Heater.glowDrive = 5;
}

unsigned char
CHeaterStorage::getGlowDrive()
{
  return _calValues.Heater.glowDrive;
}

void 
CHeaterStorage::getTimerInfo(int idx, sTimer& timerInfo)
{
  if(idx >= 0 && idx < 14) {
    timerInfo = _calValues.timer[idx];
  }
}

void 
CHeaterStorage::setTimerInfo(int idx, const sTimer& timerInfo)
{
  if(idx >= 0 && idx < 14) {
    _calValues.timer[idx] = timerInfo;
  }
}

long 
CHeaterStorage::getDimTime()
{
  return _calValues.Options.dimTime;
}

void 
CHeaterStorage::setDimTime(long val)
{
  _calValues.Options.dimTime = val;
}

long 
CHeaterStorage::getMenuTimeout()
{
  return _calValues.Options.menuTimeout;
}

void 
CHeaterStorage::setMenuTimeout(long val)
{
  _calValues.Options.menuTimeout = val;
}

unsigned char 
CHeaterStorage::getDegFMode()
{
  return _calValues.Options.degF;
}

void 
CHeaterStorage::setDegFMode(unsigned char val)
{
  _calValues.Options.degF = val;
  save();
}

unsigned char 
CHeaterStorage::getWifiEnabled()
{
  return _calValues.Options.enableWifi;
}

void 
CHeaterStorage::setWifiEnabled(unsigned char val)
{
  _calValues.Options.enableWifi = val;
  save();
}

unsigned char 
CHeaterStorage::getOTAEnabled()
{
  return _calValues.Options.enableOTA;
}

void 
CHeaterStorage::setOTAEnabled(unsigned char val)
{
  _calValues.Options.enableOTA = val;
  save();
}

const sCyclicThermostat&
CHeaterStorage::getCyclicMode() const
{
  return _calValues.Options.cyclic;
}

void 
CHeaterStorage::setCyclicMode(const sCyclicThermostat& val)
{
  _calValues.Options.cyclic = val;
  save();
}

GPIOinModes
CHeaterStorage::getGPIOinMode()
{
  GPIOinModes inMode = GPIOinNone;
  switch(_calValues.Options.GPIOinMode) {
    case 0: inMode = GPIOinNone; break;
    case 1: inMode = GPIOinOn1Off2; break;
    case 2: inMode = GPIOinOnHold1; break;
    case 3: inMode = GPIOinOn1Off1; break;
  }
  return inMode;
}

void 
CHeaterStorage::setGPIOinMode(unsigned char val)
{
  _calValues.Options.GPIOinMode = val;
}

GPIOoutModes
CHeaterStorage::getGPIOoutMode()
{
  GPIOoutModes outMode = GPIOoutNone;
  switch(_calValues.Options.GPIOoutMode) {
    case 0: outMode = GPIOoutNone; break;
    case 1: outMode = GPIOoutStatus; break;
    case 2: outMode = GPIOoutUser; break;
  }
  return outMode;
}

void 
CHeaterStorage::setGPIOoutMode(unsigned char val)
{
  _calValues.Options.GPIOoutMode = val;
}

GPIOalgModes
CHeaterStorage::getGPIOalgMode()
{
  GPIOalgModes algMode = GPIOalgNone;
  switch (_calValues.Options.GPIOalgMode) {
    case 0: algMode = GPIOalgNone; break;
    case 1: algMode = GPIOalgHeatDemand; break;
  }
  return algMode;
}

void 
CHeaterStorage::setGPIOalgMode(unsigned char val)
{
  _calValues.Options.GPIOalgMode = val;
}

uint16_t
CHeaterStorage::getFrameRate()
{
  return _calValues.Options.FrameRate;
}

void
CHeaterStorage::setFrameRate(uint16_t val)
{
  _calValues.Options.FrameRate = val;
}

const sHomeMenuActions&
CHeaterStorage::getHomeMenu() const
{
  return _calValues.Options.HomeMenu;
}

void
CHeaterStorage::setHomeMenu(sHomeMenuActions val)
{
  _calValues.Options.HomeMenu = val;
}

// MQTT parameter read/save
const sMQTTparams& 
CHeaterStorage::getMQTTinfo() const
{
  return _calValues.MQTT;
}

void
CHeaterStorage::setMQTTinfo(const sMQTTparams& info)
{
  _calValues.MQTT = info;
}

// credentials read/save
const sCredentials& 
CHeaterStorage::getCredentials() const
{
  return _calValues.Credentials;
}

void
CHeaterStorage::setCredentials(const sCredentials& info)
{
  _calValues.Credentials = info;
}


///////////////////////////////////////////////////////////////////////////////////////
//          ESP32
//
//#ifdef ESP32

CESP32HeaterStorage::CESP32HeaterStorage()
{
  init();
}

CESP32HeaterStorage::~CESP32HeaterStorage()
{
}

void
CESP32HeaterStorage::init()
{
  for(int i=0; i<14; i++) {
    _calValues.timer[i].timerID = i;   // each instance needs a unique ID
  }
}

void 
CESP32HeaterStorage::load()
{
  DebugPort.println("Reading from NV storage");
  _calValues.Heater.load();
  for(int i=0; i<14; i++) {
    _calValues.timer[i].load();
  }
  _calValues.Options.load();
  _calValues.MQTT.load();
  _calValues.Credentials.load();
}

void 
CESP32HeaterStorage::save()
{
  DebugPort.println("Saving to NV storage");
  _calValues.Heater.save();
  for(int i=0; i<14; i++) {
    _calValues.timer[i].save();
  }
  _calValues.Options.save();
  _calValues.MQTT.save();
  _calValues.Credentials.save();
}

void 
sHeater::load()
{
  // section for heater calibration params
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("Calibration", false);
  validatedLoad("minPump", Pmin, 14, u8inBounds, 4, 100);
  validatedLoad("maxPump", Pmax, 45, u8inBounds, 4, 150);
  validatedLoad("minFan", Fmin, 1500, u16inBounds, 100, 5000);
  validatedLoad("maxFan", Fmax, 4500, u16inBounds, 100, 6000);
  validatedLoad("thermostat", ThermostatMode, 1, u8inBounds, 0, 1);
  validatedLoad("setTemperature", setTemperature, 22, u8inBounds, 0, 40);
  validatedLoad("systemVoltage", sysVoltage, 120, u8Match2, 120, 240);
  validatedLoad("fanSensor", fanSensor, 1, u8inBounds, 1, 2);
  validatedLoad("glowDrive", glowDrive, 5, u8inBounds, 1, 6);
  preferences.end();    
}

void 
sHeater::save()
{
  // section for heater calibration params
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("Calibration", false);
  preferences.putUChar("minPump", Pmin);
  preferences.putUChar("maxPump", Pmax);
  preferences.putUShort("minFan", Fmin);
  preferences.putUShort("maxFan", Fmax);
  preferences.putUChar("thermostat", ThermostatMode);
  preferences.putUChar("setTemperature", setTemperature);
  preferences.putUChar("systemVoltage", sysVoltage);
  preferences.putUChar("fanSensor", fanSensor);
  preferences.putUChar("glowDrive", glowDrive);
  preferences.end();    
}

void 
sTimer::load() 
{
  // **** MAX LENGTH is 15 for names ****
  char SectionName[16];
  sprintf(SectionName, "timer%d", timerID+1);
  preferences.begin(SectionName, false);
  validatedLoad("startHour", start.hour, 0, s8inBounds, 0, 23);
  validatedLoad("startMin", start.min, 0, s8inBounds, 0, 59);
  validatedLoad("stopHour", stop.hour, 0, s8inBounds, 0, 23);
  validatedLoad("stopMin", stop.min, 0, s8inBounds, 0, 59);
  validatedLoad("enabled", enabled, 0, u8inBounds, 0, 255);  // all 8 bits used!
  validatedLoad("repeat", repeat, 0, u8inBounds, 0, 1);
  validatedLoad("temperature", temperature, 22, u8inBounds, 8, 35);
  preferences.end();    
}

void 
sTimer::save() 
{
  // **** MAX LENGTH is 15 for names ****
  char SectionName[16];
  sprintf(SectionName, "timer%d", timerID+1);
  preferences.begin(SectionName, false);
  preferences.putChar("startHour", start.hour);
  preferences.putChar("startMin", start.min);
  preferences.putChar("stopHour", stop.hour);
  preferences.putChar("stopMin", stop.min);
  preferences.putUChar("enabled", enabled);
  preferences.putUChar("repeat", repeat);
  preferences.putUChar("temperature", temperature);
  preferences.end();    
}

void 
sBTCoptions::load()
{
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("user", false);
  validatedLoad("dimTime", dimTime, 60000, s32inBounds, -600000, 600000);
  validatedLoad("menuTimeout", menuTimeout, 60000, s32inBounds, 0, 300000);
  validatedLoad("degF", degF, 0, u8inBounds, 0, 1);
  validatedLoad("thermoMethod", ThermostatMethod, (10 << 2), u8inBounds, 0, 2, 0x03);
  validatedLoad("enableWifi", enableWifi, 1, u8inBounds, 0, 1);
  validatedLoad("enableOTA", enableOTA, 1, u8inBounds, 0, 1);
  validatedLoad("cyclicStop", cyclic.Stop, 0, s8inBounds, 0, 10);
  validatedLoad("cyclicStart", cyclic.Start, -1, s8inBounds, -20, 0);
  validatedLoad("GPIOinMode", GPIOinMode, 0, u8inBounds, 0, 3);
  validatedLoad("GPIOoutMode", GPIOoutMode, 0, u8inBounds, 0, 2);
  validatedLoad("GPIOalgMode", GPIOalgMode, 0, u8inBounds, 0, 2);
  validatedLoad("MenuonTimeout", HomeMenu.onTimeout, 0, u8inBounds, 0, 3);
  validatedLoad("MenuonStart", HomeMenu.onStart, 0, u8inBounds, 0, 3);
  validatedLoad("MenuonStop", HomeMenu.onStop, 0, u8inBounds, 0, 3);
  validatedLoad("FrameRate", FrameRate, 1000, u16inBounds, 300, 1500);
  preferences.end();    
}

void 
sBTCoptions::save()
{
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("user", false);
  preferences.putLong("dimTime", dimTime);
  preferences.putLong("menuTimeout", menuTimeout);
  preferences.putUChar("degF", degF);
  preferences.putUChar("thermoMethod", ThermostatMethod);
  preferences.putUChar("enableWifi", enableWifi);
  preferences.putUChar("enableOTA", enableOTA);
  preferences.putChar("cyclicStop", cyclic.Stop);
  preferences.putChar("cyclicStart",cyclic.Start);  
  preferences.putUChar("GPIOinMode", GPIOinMode);
  preferences.putUChar("GPIOoutMode", GPIOoutMode);
  preferences.putUChar("GPIOalgMode", GPIOalgMode);
  preferences.putUChar("MenuOnTimeout", HomeMenu.onTimeout);
  preferences.putUChar("MenuonStart", HomeMenu.onStart);
  preferences.putUChar("MenuonStop", HomeMenu.onStop);
  preferences.putUShort("FrameRate", FrameRate);
  preferences.end();    
}

void 
sMQTTparams::load()
{
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("mqtt", false);
  validatedLoad("enabled", enabled, 0, u8inBounds, 0, 1);
  validatedLoad("port", port, 0, u16inBounds, 0, 0xffff);
  validatedLoad("host", host, 127, "hostIP");
  validatedLoad("username", username, 31, "username");
  validatedLoad("password", password, 31, "password");
  preferences.end();    
}

void 
sMQTTparams::save()
{
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("mqtt", false);
  preferences.putUChar("enabled", enabled);
  preferences.putUShort("port", port);
  preferences.putString("host", host);
  preferences.putString("username", username);
  preferences.putString("password", password);
  preferences.end();    
}

bool 
sMQTTparams::valid()
{
  return true;
}

void 
sCredentials::load()
{
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("credentials", false);
  validatedLoad("SSID", SSID, 31, "Afterburner");
  validatedLoad("APpassword", APpassword, 31, "thereisnospoon");
  validatedLoad("webUpdateUser", webUpdateUsername, 31, "Afterburner");
  validatedLoad("webUpdatePass", webUpdatePassword, 31, "BurnBabyBurn");
  preferences.end();    
}

void 
sCredentials::save()
{
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("credentials", false);
  preferences.putString("SSID", SSID);
  preferences.putString("APpassword", APpassword);
  preferences.putString("webUpdateUser", webUpdateUsername);
  preferences.putString("webUpdatePass", webUpdatePassword);
  preferences.end();    
}

bool 
sCredentials::valid()
{
  return true;
}

