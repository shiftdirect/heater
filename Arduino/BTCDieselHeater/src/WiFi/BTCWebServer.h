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

// BTCWebServer.h

#ifndef _BTCWEBSERVER_h
#define _BTCWEBSERVER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "BTCWifi.h"
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <WebSocketsServer.h>




#endif

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void initWebServer();
bool doWebServer();
void handleWMConfig();
void handleRoot();
void handleReset();
void handleNotFound();
void webturnOn();
void webturnOff();

bool sendWebServerString(const char* Str);
bool isWebServerClientChange(); 
void listDir(fs::FS &fs, const char * dirname, uint8_t levels, String& HTMLreport, bool withHTMLanchors=false);
