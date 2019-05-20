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


#include "BTCWebServer.h"
#include "../Utility/DebugPort.h"
#include "../Protocol/TxManage.h"
#include "../Protocol/helpers.h"
#include "../cfg/pins.h"
#include "../cfg/BTCConfig.h"
#include "Index.h"
#include "../Utility/BTC_JSON.h"
#include "../Utility/Moderator.h"
#include <WiFiManager.h>
#if USE_SPIFFS == 1  
#include <SPIFFS.h>
#endif
#include "../Utility/NVStorage.h"

extern void ShowOTAScreen(int percent=0, bool webpdate=false);

extern WiFiManager wm;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

bool bRxWebData = false;   // flags for OLED animation
bool bTxWebData = false;
bool bUpdateAccessed = false;  // flag used to ensure web update always starts via /update. direct accesses to /updatenow will FAIL

const int led = 13;

#if USE_SPIFFS == 1  

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  DebugPort.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  DebugPort.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}

/*void handleFavIcon() {
  handleFileRead("/favicon.ico");
}*/

void handleBTCRoot() {
  handleFileRead("/index.html");
}
#else
void handleBTCRoot() {
	String s = MAIN_PAGE; //Read HTML contents
	server.send(200, "text/html", s); //Send web page
}
#endif

void handleWMConfig() {
	server.send(200, "text/plain", "Start Config Portal - Retaining credential");
	DebugPort.println("Starting web portal for wifi config");
  delay(500);
//	wm.startWebPortal();
  wifiEnterConfigPortal(true, false, 3000);
}

void handleReset() {
	server.send(200, "text/plain", "Start Config Portal - Resetting Wifi credentials!");
	DebugPort.println("diconnecting client and wifi, then rebooting");
  delay(500);
  wifiEnterConfigPortal(true, true, 3000);
}

void handleFormat() {
	server.send(200, "text/plain", "Formatting SPIFFS partition!");
	DebugPort.println("Formatting SPIFFS partition");
  delay(500);
  SPIFFS.format();
}

void handleBTCNotFound() {
	digitalWrite(led, 1);
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
	digitalWrite(led, 0);
}

const char* serverIndex = R"=====(
)=====";

const char* rootIndex = R"=====(
<!DOCTYPE html>
<script>
function init() {
  window.location.assign("/"); 
}
</script
<html>
  <head>
    <title>HTML Meta Tag</title>
  </head>
  <body onload="javascript:init()">
    <p>Root redirect</p>
  </body>
</html>
)=====";

void rootRedirect()
{
// server.sendHeader("Connection", "close");
  server.send(200, "text/html", rootIndex);
}


void initWebServer(void) {

  Update
  .onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress / (total / 100));
		DebugPort.printf("Progress: %u%%\r", percent);
    DebugPort.handle();    // keep telnet spy alive
    ShowOTAScreen(percent, true);
    DebugPort.print("^");
	});
  
	if (MDNS.begin("Afterburner")) {
		DebugPort.println("MDNS responder started");
	}
	
//	server.on("/", handleBTCRoot);

	server.on("/wmconfig", handleWMConfig);
	server.on("/resetwifi", handleReset);
	server.on("/formatspiffs", handleFormat);

  server.on("/tst", HTTP_GET, []() {
    rootRedirect();
  });

  // Magical code originally shamelessly lifted from Arduino WebUpdate example, then modified
  // This allows pushing new firmware to the ESP from a WEB BROWSER!
  // Added authentication and a sequencing flag to ensure this is not bypassed
  //
  // Initial launch page
  server.on("/update", HTTP_GET, []() {
    sCredentials creds = NVstore.getCredentials();
    if (!server.authenticate(creds.webUpdateUsername, creds.webUpdatePassword)) {
      return server.requestAuthentication();
    }
    bUpdateAccessed = true;
    server.sendHeader("Connection", "close");
    server.sendHeader("Cache-Control", "no-cache");
    handleFileRead("/uploadfirmware.html");
  });
  server.on("/updatenow", HTTP_GET, []() {  // handle attempts to just browse the /updatenow path - force redirect to root
    rootRedirect();
  });
  // actual guts that manages the new firmware upload
  server.on("/updatenow", HTTP_POST, []() {
    // completion functionality
    if(Update.hasError())
      server.send(200, "text/plain", "FAIL - Afterburner will reboot shortly");
    else 
      server.send(200, "OK - Afterburner will reboot shortly");
    delay(1000);
    ESP.restart();                             // reboot
  }, []() {
    if(bUpdateAccessed) {  // only allow progression via /update, attempts to directly access /updatenow will fail
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        DebugPort.setDebugOutput(true);
        DebugPort.printf("Update: %s\r\n", upload.filename.c_str());
        if (!Update.begin()) { //start with max available size
          Update.printError(DebugPort);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
#if USE_SW_WATCHDOG == 1
        feedWatchdog();   // we get stuck here for a while, don't let the watchdog bite!
#endif
        if(upload.totalSize) {
          char JSON[64];
          sprintf(JSON, "{\"progress\":%d}", upload.totalSize);
          sendWebServerString(JSON);  // feedback proper byte count of update
//          DebugPort.print(JSON);
        }
        DebugPort.print(".");
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(DebugPort);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          DebugPort.printf("Update Success: %u\r\nRebooting...\r\n", upload.totalSize);
        } else {
          Update.printError(DebugPort);
        }
        DebugPort.setDebugOutput(false);
        bUpdateAccessed = false;
      } else {
        DebugPort.printf("Update Failed Unexpectedly (likely broken connection): status=%d\r\n", upload.status);
        bUpdateAccessed = false;
      }
    }
    else {
      // attempt to POST without using /update - forced redirect to root
      rootRedirect();
    }
  });

#if USE_SPIFFS == 1  
  // NOTE: this serves the default home page, and favicon.ico
  server.onNotFound([]() 
    {                                                      // If the client requests any URI
      if (!handleFileRead(server.uri()))                   // send it if it exists
         server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    }
  );
#else
	server.onNotFound(handleBTCNotFound);
#endif

	server.begin();
	webSocket.begin();
	webSocket.onEvent(webSocketEvent);
	DebugPort.println("HTTP server started");

}

unsigned char cVal;

// called my main sketch loop()
bool doWebServer(void) {
	webSocket.loop();
	server.handleClient();
}

bool isWebServerClientChange() 
{
	static int prevNumClients = -1;

	int numClients = webSocket.connectedClients();
	if(numClients != prevNumClients) {
		prevNumClients = numClients;
		DebugPort.println("Changed number of web clients, should reset JSON moderator");
    return true;
	}
  return false;
}

bool sendWebServerString(const char* Str)
{
  unsigned long tStart = millis();
	if(webSocket.connectedClients()) {
    unsigned long tCon = millis() - tStart;
    tStart = millis();
    bTxWebData = true;              // OLED tx data animation flag
    webSocket.broadcastTXT(Str);
    unsigned long tWeb = millis() - tStart;
//    DebugPort.printf("Websend times : %ld,%ld\r\n", tCon, tWeb); 
		return true;
	}
  return false;
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
	if (type == WStype_TEXT) {
		bRxWebData = true;
		char cmd[256];
		memset(cmd, 0, 256);
		for (int i = 0; i < length && i < 256; i++) {
			cmd[i] = payload[i];
		}
		interpretJsonCommand(cmd);  // send to the main heater controller decode routine
  }
}

bool hasWebClientSpoken(bool reset)
{
	bool retval = bRxWebData;
	if(reset)
  	bRxWebData = false;
	return retval;
}

bool hasWebServerSpoken(bool reset)
{
	bool retval = bTxWebData;
	if(reset)
		bTxWebData = false;
	return retval;
}

