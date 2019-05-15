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
#include "../cfg/BTCConfig.h"
#if USE_SPIFFS == 1  
#include <SPIFFS.h>
#endif
extern void ShowOTAScreen(int percent=0, bool webpdate=false);


#include <esp_int_wdt.h>
#include <esp_task_wdt.h>

void hard_restart() {
  esp_task_wdt_init(1,true);
  esp_task_wdt_add(NULL);
  while(true);
}

void initOTA(){
	// ArduinoOTA.setHostname("myesp32");
	ArduinoOTA.setHostname("AfterburnerOTA");
//  ArduinoOTA.setPassword("TESTO123");

	ArduinoOTA
		.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH)
			type = "sketch";
		else // U_SPIFFS
			type = "filesystem";

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
	    SPIFFS.end();
		DebugPort.println("Start updating " + type);
    DebugPort.handle();    // keep telnet spy alive
    ShowOTAScreen();

	})
		.onEnd([]() {
		DebugPort.println("\nEnd");
    DebugPort.handle();    // keep telnet spy alive
    delay(100);
//    DebugPort.end();       // force graceful close of telnetspy - ensures a client will reconnect cleanly
	})
		.onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress / (total / 100));
		DebugPort.printf("Progress: %u%%\r", percent);
    DebugPort.handle();    // keep telnet spy alive
    ShowOTAScreen(percent);

	})
		.onError([](ota_error_t error) {
		DebugPort.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) DebugPort.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) DebugPort.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) DebugPort.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) DebugPort.println("Receive Failed");
		else if (error == OTA_END_ERROR) DebugPort.println("End Failed");
    DebugPort.handle();    // keep telnet spy alive
	});

	ArduinoOTA.begin();
}

void DoOTA(){
  ArduinoOTA.handle();
};
