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
#include <esp32fota.h>
#include "../Protocol/helpers.h"

extern void ShowOTAScreen(int percent=0, bool webpdate=false);

esp32FOTA FOTA("afterburner-fota-http", int(getVersion()*1000));
unsigned long FOTAtime = millis() + 60000;  // initially check in a minutes time 
int FOTAauth = 0;

#include <esp_int_wdt.h>
#include <esp_task_wdt.h>

void hard_restart() {
  esp_task_wdt_init(1,true);
  esp_task_wdt_add(NULL);
  while(true);
}

void initOTA(){
  FOTA.checkURL = "http://www.mrjones.id.au/afterburner/fota/fota.json";
	ArduinoOTA.setHostname("AfterburnerOTA");

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
#if USE_SW_WATCHDOG == 1
    feedWatchdog();   // we get stuck here for a while, don't let the watchdog bite!
#endif
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
    DebugPort.print("%%");
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

  // manage Firmware OTA
  // this is where the controller contacts a web server to discover if new firmware is available
  // if so, it can dowload and implant using OTA and become effective next reboot!
  long tDelta = millis() - FOTAtime;
  if(tDelta > 0) {  
    FOTAtime = millis() + 600000;  // 10 minutes
    if ((WiFi.status() == WL_CONNECTED)) {   // bug workaround in FOTA where execHTTPcheck does not return false in this condition
      DebugPort.println("Checking for new firmware...");
      if(FOTA.execHTTPcheck()) {
        DebugPort.println("New firmware available on web server!");
        if(FOTAauth == 2) {  // user has authorised update (was == 1 before auth.)
          FOTA.execOTA();    // go ahead and do the update, reading new file from web server
          FOTAauth = 0;      // and we're done.
        }
        else 
          FOTAauth = 1;   // flag that new firmware is available
      }
    }
  }
};

bool isUpdateAvailable(bool test)
{
  if(test) {
    return FOTAauth == 1;
  }
  else
  {
    FOTAauth = 2;
    FOTAtime = millis();  // force immediate update test
    return true;
  }
  
}

void checkFOTA()
{
  FOTAtime = millis();
}