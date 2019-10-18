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

#include <Arduino.h>
#include "NVStorage.h"
#include "DebugPort.h"
#include <driver/adc.h>


bool 
sNVStore::valid()
{
  bool retval = true;
  retval &= heaterTuning.valid();
  retval &= userSettings.valid();
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
  heaterTuning.init();
  userSettings.init();
  
  for(int i=0; i<14; i++) {
    timer[i].init(i);
  }

  MQTT.init();
  Credentials.init();
  hourMeter.init();
}

CHeaterStorage::CHeaterStorage()
{
  _calValues.init();
}

float
sHeaterTuning::getPmin() const
{
  return float(Pmin) * 0.1f;
}

float
sHeaterTuning::getPmax() const
{
  return float(Pmax) * 0.1f;
}


void
sHeaterTuning::setPmin(float val)
{
  BOUNDSLIMIT(val, 0.4, 5.0);
  uint8_t cVal = (uint8_t)(val * 10.f + 0.5f);
  Pmin = cVal;
}

void
sHeaterTuning::setPmax(float val)
{
  BOUNDSLIMIT(val, 1.0, 10.0);
  uint8_t cVal = (uint8_t)(val * 10.f + 0.5f);
  Pmax = cVal;
}


void 
sHeaterTuning::setSysVoltage(float fVal)
{
  int val = int(fVal * 10.0);
  if(val == 120 || val == 240) {
    sysVoltage = val;
  }
}

float  
sHeaterTuning::getLVC() const
{
  return lowVolts * 0.1;
}

void 
CHeaterStorage::getTimerInfo(int idx, sTimer& timerInfo)
{
  if(INBOUNDS(idx, 0, 13)) {
    timerInfo = _calValues.timer[idx];
  }
}

void 
CHeaterStorage::setTimerInfo(int idx, const sTimer& timerInfo)
{
  if(INBOUNDS(idx, 0, 13)) {
    _calValues.timer[idx] = timerInfo;
  }
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

const sUserSettings& 
CHeaterStorage::getUserSettings() const
{
  return _calValues.userSettings;
}

void 
CHeaterStorage::setUserSettings(const sUserSettings& info) {
  _calValues.userSettings = info;
}

const sHeaterTuning& 
CHeaterStorage::getHeaterTuning() const
{
  return _calValues.heaterTuning;
}

void 
CHeaterStorage::setHeaterTuning(const sHeaterTuning& info)
{
  _calValues.heaterTuning = info;
}

const sHourMeter&
CHeaterStorage::getHourMeter() const
{
  return _calValues.hourMeter;
}

bool
CHeaterStorage::setHourMeter(const sHourMeter& newVals)
{
//  if(_calValues.hourMeter != newVals) {
  if(newVals != _calValues.hourMeter) {
    _calValues.hourMeter = newVals;  // stage changes
    save();                       // request save to NV
    return true;
  };
  return false;
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
  _bShouldSave = false;
  _calValues.init();
}

void 
CESP32HeaterStorage::load()
{
  DebugPort.println("Reading from NV storage");
  _calValues.heaterTuning.load();
  for(int i=0; i<14; i++) {
    _calValues.timer[i].load();
  }
  _calValues.userSettings.load();
  _calValues.MQTT.load();
  _calValues.Credentials.load();
  _calValues.hourMeter.load();
}

void 
CESP32HeaterStorage::doSave()
{
  if(_bShouldSave) {
    _bShouldSave = false;
    DebugPort.println("Saving to NV storage");
    _calValues.heaterTuning.save();
    for(int i=0; i<14; i++) {
      _calValues.timer[i].save();
    }
    _calValues.userSettings.save();
    _calValues.MQTT.save();
    _calValues.Credentials.save();
    _calValues.hourMeter.save();
  }
}

void 
CESP32HeaterStorage::save()
{
  _bShouldSave = true;   // queue request to save to NV
}

void 
sHeaterTuning::load()
{
  // section for heater calibration params
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("Calibration", false);
  validatedLoad("minPump", Pmin, 14, u8inBounds, 4, 100);
  validatedLoad("maxPump", Pmax, 45, u8inBounds, 4, 150);
  validatedLoad("minFan", Fmin, 1500, u16inBounds, 100, 5000);
  validatedLoad("maxFan", Fmax, 4500, u16inBounds, 100, 6000);
  validatedLoad("systemVoltage", sysVoltage, 120, u8Match2, 120, 240);
  validatedLoad("fanSensor", fanSensor, 1, u8inBounds, 1, 2);
  validatedLoad("glowDrive", glowDrive, 5, u8inBounds, 1, 6);
  if(sysVoltage == 120)
    validatedLoad("lowVolts", lowVolts, 115, u8inBoundsOrZero, 100, 125);
  else
    validatedLoad("lowVolts", lowVolts, 230, u8inBoundsOrZero, 200, 250);
  validatedLoad("pumpCal", pumpCal, 0.02, 0.001, 1);
  validatedLoad("tempOffset0", DS18B20probe[0].offset, 0.0, -10.0, +10.0);
  validatedLoad("tempOffset1", DS18B20probe[1].offset, 0.0, -10.0, +10.0);
  validatedLoad("tempOffset2", DS18B20probe[2].offset, 0.0, -10.0, +10.0);
  preferences.getBytes("probeSerial0", DS18B20probe[0].romCode.bytes, 8);
  preferences.getBytes("probeSerial1", DS18B20probe[1].romCode.bytes, 8);
  preferences.getBytes("probeSerial2", DS18B20probe[2].romCode.bytes, 8);
  validatedLoad("tempOffsetBME", BME280probe.offset, 0.0, -10.0, +10.0);
  validatedLoad("probeBMEPrmy", BME280probe.bPrimary, 0, u8inBounds, 0, 1);
  preferences.end();    

  // for(int i=0; i<3; i++) {
  //   DebugPort.printf("Rd Probe[%d] %02X:%02X:%02X:%02X:%02X:%02X\r\n",
  //                    i,
  //                    DS18B20probe[i].romCode.fields.serial_number[5],
  //                    DS18B20probe[i].romCode.fields.serial_number[4],
  //                    DS18B20probe[i].romCode.fields.serial_number[3],
  //                    DS18B20probe[i].romCode.fields.serial_number[2],
  //                    DS18B20probe[i].romCode.fields.serial_number[1],
  //                    DS18B20probe[i].romCode.fields.serial_number[0]
  //                    );
  // }
}

void 
sHeaterTuning::save()
{
  // section for heater calibration params
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("Calibration", false);
  preferences.putUChar("minPump", Pmin);
  preferences.putUChar("maxPump", Pmax);
  preferences.putUShort("minFan", Fmin);
  preferences.putUShort("maxFan", Fmax);
  preferences.putUChar("systemVoltage", sysVoltage);
  preferences.putUChar("fanSensor", fanSensor);
  preferences.putUChar("glowDrive", glowDrive);
  preferences.putUChar("lowVolts", lowVolts);
  preferences.putFloat("pumpCal", pumpCal);
  preferences.putFloat("tempOffset0", DS18B20probe[0].offset);
  preferences.putFloat("tempOffset1", DS18B20probe[1].offset);
  preferences.putFloat("tempOffset2", DS18B20probe[2].offset);
  preferences.putBytes("probeSerial0", DS18B20probe[0].romCode.bytes, 8);
  preferences.putBytes("probeSerial1", DS18B20probe[1].romCode.bytes, 8);
  preferences.putBytes("probeSerial2", DS18B20probe[2].romCode.bytes, 8);
  preferences.putFloat("tempOffsetBME", BME280probe.offset);
  preferences.putUChar("probeBMEPrmy", BME280probe.bPrimary);
  preferences.end();    

  // for(int i=0; i<3; i++) {
  //   DebugPort.printf("Wr Probe[%d] %02X:%02X:%02X:%02X:%02X:%02X\r\n",
  //                    i,
  //                    DS18B20probe[i].romCode.fields.serial_number[5],
  //                    DS18B20probe[i].romCode.fields.serial_number[4],
  //                    DS18B20probe[i].romCode.fields.serial_number[3],
  //                    DS18B20probe[i].romCode.fields.serial_number[2],
  //                    DS18B20probe[i].romCode.fields.serial_number[1],
  //                    DS18B20probe[i].romCode.fields.serial_number[0]
  //                    );
  // }
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
sUserSettings::load()
{
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("user", false);
  validatedLoad("dimTime", dimTime, 60000, -600000, 600000);
  validatedLoad("menuTimeout", menuTimeout, 60000, 0, 300000);
  validatedLoad("thermoTimeout", ExtThermoTimeout, 0, 0, 3600000);
  validatedLoad("degF", degF, 0, u8inBounds, 0, 1);
  validatedLoad("thermostat", useThermostat, 1, u8inBounds, 0, 1);
  validatedLoad("thermoMethod", ThermostatMethod, 0, u8inBounds, 0, 255);
  // catch and migrate old combined method & window
  if(ThermostatMethod & 0xFC) {
    float defVal = float(ThermostatMethod>>2) * 0.1f;
    validatedLoad("thermoWindow", ThermostatWindow, defVal, 0.2f, 10.0f);
    preferences.putUChar("thermoMethod", ThermostatMethod & 0x03);  // strip old window
  }
  validatedLoad("thermoWindow", ThermostatWindow, 1.0f, 0.2f, 10.f);
  DebugPort.printf("2) Window = %f\r\n", ThermostatWindow);
  validatedLoad("enableWifi", enableWifi, 1, u8inBounds, 0, 1);
  validatedLoad("enableOTA", enableOTA, 0, u8inBounds, 0, 1);
  validatedLoad("cyclicStop", cyclic.Stop, 0, s8inBounds, 0, 10);
  validatedLoad("cyclicStart", cyclic.Start, -1, s8inBounds, -20, 0);
  uint8_t tVal;
  validatedLoad("GPIOinMode", tVal, 0, u8inBounds, 0, 255);
  if(tVal <= 4) {
    // migrate old GPIO input mode
    GPIO.in1Mode = CGPIOin1::Disabled;  
    GPIO.in2Mode = CGPIOin2::Disabled; 
    switch(tVal) {
      case 1: GPIO.in1Mode = CGPIOin1::Start;   GPIO.in2Mode = CGPIOin2::Stop; break;
      case 2: GPIO.in1Mode = CGPIOin1::Run;     GPIO.in2Mode = CGPIOin2::Thermostat; break;
      case 3: GPIO.in1Mode = CGPIOin1::StartStop; break;
      case 4: GPIO.in2Mode = CGPIOin2::Thermostat; break;
    }
    preferences.putUChar("GPIOinMode", 0xff);  // cancel old
    preferences.putUChar("GPIOin1Mode", GPIO.in1Mode);  // set new
    preferences.putUChar("GPIOin2Mode", GPIO.in2Mode);  // set new
  }  
  validatedLoad("GPIOin1Mode", tVal, 0, u8inBounds, 0, 3); GPIO.in1Mode = (CGPIOin1::Modes)tVal;
  validatedLoad("GPIOin2Mode", tVal, 0, u8inBounds, 0, 2); GPIO.in2Mode = (CGPIOin2::Modes)tVal;
  validatedLoad("GPIOoutMode", tVal, 0, u8inBounds, 0, 255); 
  if(tVal <= 2) {
    // migrate old GPIO output mode
    GPIO.out1Mode = CGPIOout1::Disabled;  
    GPIO.out2Mode = CGPIOout2::Disabled; 
    switch(tVal) {
      case 1: GPIO.out1Mode = CGPIOout1::Status;  break;
      case 2: GPIO.out1Mode = CGPIOout1::User;  GPIO.out2Mode = CGPIOout2::User; break;
    }
    preferences.putUChar("GPIOoutMode", 0xff);  // cancel old
    preferences.putUChar("GPIOout1Mode", GPIO.out1Mode);  // set new
    preferences.putUChar("GPIOout2Mode", GPIO.out2Mode);  // set new
  }
  validatedLoad("GPIOout1Mode", tVal, 0, u8inBounds, 0, 2); GPIO.out1Mode = (CGPIOout1::Modes)tVal;
  validatedLoad("GPIOout2Mode", tVal, 0, u8inBounds, 0, 1); GPIO.out2Mode = (CGPIOout2::Modes)tVal;
  validatedLoad("GPIOalgMode", tVal, 0, u8inBounds, 0, 2); GPIO.algMode = (CGPIOalg::Modes)tVal;
  validatedLoad("MenuOnTimeout", HomeMenu.onTimeout, 0, u8inBounds, 0, 3);
  validatedLoad("MenuonStart", HomeMenu.onStart, 0, u8inBounds, 0, 3);
  validatedLoad("MenuonStop", HomeMenu.onStop, 0, u8inBounds, 0, 3);
  validatedLoad("FrameRate", FrameRate, 1000, u16inBounds, 300, 1500);
  validatedLoad("JSONsingle", JSON.singleElement, 0, u8inBounds, 0, 1);
  validatedLoad("JSONLF", JSON.LF, 0, u8inBounds, 0, 1);
  validatedLoad("JSONpad", JSON.padding, 0, u8inBounds, 0, 1);
  validatedLoad("menuMode", menuMode, 0, u8inBounds, 0, 2);
  validatedLoad("Clock12hr", clock12hr, 0, u8inBounds, 0, 1);
  validatedLoad("holdPassword", holdPassword, 0, u8inBounds, 0, 1);
  preferences.end();    
}

void 
sUserSettings::save()
{
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("user", false);
  preferences.putLong("dimTime", dimTime);
  preferences.putLong("menuTimeout", menuTimeout);
  preferences.putLong("thermoTimeout", ExtThermoTimeout);
  preferences.putUChar("thermostat", useThermostat);
  preferences.putUChar("degF", degF);
  preferences.putUChar("thermoMethod", ThermostatMethod);
  preferences.putFloat("thermoWindow", ThermostatWindow);
  preferences.putUChar("enableWifi", enableWifi);
  preferences.putUChar("enableOTA", enableOTA);
  preferences.putChar("cyclicStop", cyclic.Stop);
  preferences.putChar("cyclicStart",cyclic.Start);  
  preferences.putUChar("GPIOin1Mode", GPIO.in1Mode);
  preferences.putUChar("GPIOin2Mode", GPIO.in2Mode);
  preferences.putUChar("GPIOout1Mode", GPIO.out1Mode);
  preferences.putUChar("GPIOout2Mode", GPIO.out2Mode);
  preferences.putUChar("GPIOalgMode", GPIO.algMode);
  preferences.putUChar("MenuOnTimeout", HomeMenu.onTimeout);
  preferences.putUChar("MenuonStart", HomeMenu.onStart);
  preferences.putUChar("MenuonStop", HomeMenu.onStop);
  preferences.putUShort("FrameRate", FrameRate);
  preferences.putUChar("JSONsingle", JSON.singleElement);
  preferences.putUChar("JSONLF", JSON.LF);
  preferences.putUChar("JSONpad", JSON.padding);
  preferences.putUChar("menuMode", menuMode);
  preferences.putUChar("Clock12hr", clock12hr);
  preferences.putUChar("holdPassword", holdPassword);
  preferences.end();    
}

void 
sMQTTparams::load()
{
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("mqtt", false);
  validatedLoad("enabled", enabled, 0, u8inBounds, 0, 1);
  validatedLoad("port", port, 1883, u16inBounds, 0, 0xffff);
  validatedLoad("qos", qos, 0, u8inBounds, 0, 2);
  validatedLoad("host", host, 127, "");
  validatedLoad("username", username, 31, "");
  validatedLoad("password", password, 31, "");
  validatedLoad("topic", topicPrefix, 31, "Afterburner");
  preferences.end();    
}

void 
sMQTTparams::save()
{
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("mqtt", false);
  preferences.putUChar("enabled", enabled);
  preferences.putUShort("port", port);
  preferences.putUChar("qos", qos);
  preferences.putString("host", host);
  preferences.putString("username", username);
  preferences.putString("password", password);
  preferences.putString("topic", topicPrefix);
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

void toggle(bool& ref)
{
  ref = !ref;
}

void toggle(uint8_t& ref)
{
  ref = ref ? 0 : 1;
}

void
sHourMeter::save()
{
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("hourmeter", false);
  preferences.putULong("RunTime", RunTime);
  preferences.putULong("GlowTime", GlowTime);
  preferences.end();    
  DebugPort.printf("Hourmeter NV save: Run=%d, Glow=%d\r\n", RunTime, GlowTime);
}

void 
sHourMeter::load()
{
  // **** MAX LENGTH is 15 for names ****
  preferences.begin("hourmeter", false);
  validatedLoad("RunTime", RunTime, 0, 0, 0xffffffffL);
  validatedLoad("GlowTime", GlowTime, 0, 0, 0xffffffffL);
  preferences.end();    
  DebugPort.printf("Hourmeter NV read: Run=%d, Glow=%d\r\n", RunTime, GlowTime);
}