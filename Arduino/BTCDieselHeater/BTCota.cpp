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

