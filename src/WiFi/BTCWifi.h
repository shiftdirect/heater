/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
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

#ifndef __BTCWIFI_H__
#define __BTCWIFI_H__

#include <WiFi.h>

struct wmReboot {
  bool startPortal;
  bool eraseCreds;
  unsigned long delay;
  unsigned long started;

  wmReboot(bool timenow = false) {
    startPortal = false;
    eraseCreds = false;
    delay = 0;
    started = timenow ? millis() : 0;
  };
  wmReboot& operator=(wmReboot& rhs) {
    startPortal = rhs.startPortal;
    eraseCreds = rhs.eraseCreds;
    delay = rhs.delay;
    started = rhs.started;
    return *this;
  };
};

void doWiFiManager();
bool initWifi();
const char* getWifiAPAddrStr(); 
const char* getWifiSTAAddrStr(); 
const char* getWifiGatewayAddrStr();
const char* getWifiAPMACStr();
const char* getWifiSTAMACStr();
int8_t getWifiRSSI();
String getSTASSID();

bool isWifiSTAConnected();
bool isWifiAPonly();
bool isWifiSTA();
bool isWifiConfigPortal();
bool isWebClientConnected();
bool hasWebClientSpoken(bool reset = false);
bool hasWebServerSpoken(bool reset = false);
void wifiEnterConfigPortal(bool state, bool erase = false, long timeout = 7000, bool STAonly = false);
void wifiDisable(long rebootDelay = 7000);
void wifiFactoryDefault();
int  isWifiButton();

void scheduleWMreboot(wmReboot& newMode);
void manageWMreboot();

#endif // __BTCWIFI_H__
