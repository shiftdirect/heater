/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2018  Ray Jones <ray@mrjones.id.au>
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


extern void  requestOn();
extern void  requestOff();
extern bool  reqTempDelta(int delta);
extern bool  reqTemp(unsigned char newTemp, bool save=true);
extern bool  reqThermoToggle();
extern bool  setThermostatMode(unsigned char);
extern bool  getThermostatModeActive();  // OEM: actual mode from blue wire, BTC: or our NV
extern void  reqPumpPrime(bool on);
float getTemperatureDesired();    // OEM: the advertised value, BTC our setpoint
extern float getTemperatureSensor();
extern void  setPumpMin(float);
extern void  setPumpMax(float);
extern void  setFanMin(short);
extern void  setFanMax(short);
extern void  setFanSensor(unsigned char cVal);
extern void  setDateTime(const char* newTime);
extern void  setDate(const char* newTime);
extern void  setTime(const char* newTime);
extern void  setGlowDrive(unsigned char val);
extern void  saveNV();
extern void  setSystemVoltage(float val);
extern void  interpretJsonCommand(char* pLine);
extern void  resetWebModerator();
extern void  resetJSONmoderator();
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
extern void  setGPIOout(int channel, bool state);
extern bool  getGPIOout(int channel);
extern bool  toggleGPIOout(int channel);
extern void  feedWatchdog();
extern bool  isUpdateAvailable(bool test=true);
extern void  checkFOTA();
extern void  setUploadSize(long val);
extern void getGPIOinfo(sGPIO& info);
extern void simulateGPIOin(uint8_t newKey);   
extern void setDegFMode(bool state);


extern void ShowOTAScreen(int percent=0, eOTAmodes updateType=eOTAnormal);


#endif