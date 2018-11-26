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

#include "BTCota.h"
#include "debugport.h"

void initOTA(){
	// ArduinoOTA.setHostname("myesp32");
	ArduinoOTA.setHostname("BTCOTA");

	ArduinoOTA
		.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH)
			type = "sketch";
		else // U_SPIFFS
			type = "filesystem";

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		DebugPort.println("Start updating " + type);
	})
		.onEnd([]() {
		DebugPort.println("\nEnd");
	})
		.onProgress([](unsigned int progress, unsigned int total) {
		DebugPort.printf("Progress: %u%%\r", (progress / (total / 100)));
	})
		.onError([](ota_error_t error) {
		DebugPort.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) DebugPort.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) DebugPort.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) DebugPort.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) DebugPort.println("Receive Failed");
		else if (error == OTA_END_ERROR) DebugPort.println("End Failed");
	});

	ArduinoOTA.begin();
}

void DoOTA(){
  ArduinoOTA.handle();
};

