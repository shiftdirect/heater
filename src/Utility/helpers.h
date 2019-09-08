/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2019  Ray Jones <ray@mrjones.id.au>
 * Copyright (C) 2018  James Clark
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

#ifndef __BTC_HELPERS_H__
#define __BTC_HELPERS_H__

#include "UtilClasses.h"

struct sGPIO;

extern void forceBootInit();

extern void  requestOn();
extern void  requestOff();
extern bool  reqDemandDelta(int delta);
extern bool  reqDemand(uint8_t newTemp, bool save=true);
extern bool  reqThermoToggle();
extern bool  setThermostatMode(uint8_t);
extern bool  getThermostatModeActive();  // OEM: actual mode from blue wire, BTC: or our NV
extern bool  getExternalThermostatModeActive();  
extern bool getExternalThermostatOn();
extern const char* getExternalThermostatHoldTime();
extern void  reqPumpPrime(bool on);
extern float getTemperatureDesired();    // OEM: the advertised value, BTC our setpoint
extern uint8_t getDemandDegC();
extern void  setDemandDegC(uint8_t val);
extern uint8_t getDemandPump();

extern float getTemperatureSensor();
extern void  setPumpMin(float);
extern void  setPumpMax(float);
extern void  setFanMin(uint16_t);
extern void  setFanMax(uint16_t);
extern void  setFanSensor(uint8_t cVal);
extern void  setDateTime(const char* newTime);
extern void  setDate(const char* newTime);
extern void  setTime(const char* newTime);
extern void  setGlowDrive(uint8_t val);
extern void  saveNV();
extern void  setSystemVoltage(float val);
extern void  interpretJsonCommand(char* pLine);
extern void  resetWebModerator();
extern void  resetJSONmoderator();
extern void  resetFuelGauge();
extern const char* getBlueWireStatStr();
extern bool  hasOEMcontroller();
extern bool  hasOEMLCDcontroller();
extern int   getBlueWireStat();
extern int   getSmartError();
extern bool  isCyclicActive();
extern float getVersion();
const char* getVersionStr();
extern const char* getVersionDate();
extern int   getBoardRevision();
extern void  setupGPIO();
extern bool  setGPIOout(int channel, bool state);
extern bool  getGPIOout(int channel);
extern bool  toggleGPIOout(int channel);
extern void  feedWatchdog();
extern int   isUpdateAvailable(bool test=true);
extern void  checkFOTA();
extern void  setUploadSize(long val);
extern void getGPIOinfo(sGPIO& info);
extern void simulateGPIOin(uint8_t newKey);   
extern void setDegFMode(bool state);
extern float getBatteryVoltage(bool fast);
extern float getGlowVolts();
extern float getGlowCurrent();
extern float getFanSpeed();
extern int sysUptime();
extern void doJSONwatchdog(int topup);

void setSSID(const char* name);
void setAPpassword(const char* name);

extern void ShowOTAScreen(int percent=0, eOTAmodes updateType=eOTAnormal);


#endif