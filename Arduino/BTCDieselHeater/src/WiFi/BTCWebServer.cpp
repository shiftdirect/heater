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
#include "Index.h"
#include "../Utility/BTC_JSON.h"
#include "../Utility/Moderator.h"
#include <WiFiManager.h>
#if USE_SPIFFS == 1  
#include <SPIFFS.h>
#endif

extern WiFiManager wm;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

bool bRxWebData = false;   // flags for OLED animation
bool bTxWebData = false;

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
/*  if(SPIFFS.exists("/index.html")) {
    File html = SPIFFS.open("/index.html");
    server.streamFile(html, "text/html");
    html.close();
  }
  else {
    DebugPort.println("\"/index.html\" does not exist!!!");
  }*/
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
	//client.disconnect();
//	wifi_station_disconnect();
//	wm.disconnect();
//	wm.resetSettings();
  wifiEnterConfigPortal(true, true, 3000);
}

void handleFormat() {
	server.send(200, "text/plain", "Formatting SPIFFS partition!");
	DebugPort.println("Formatting SPIFFS partition");
  delay(500);
  SPIFFS.format();
	//client.disconnect();
//	wifi_station_disconnect();
//	wm.disconnect();
//	wm.resetSettings();
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

const char* serverIndex = "<form method='POST' action='/updatenow' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

void initWebServer(void) {

  
	if (MDNS.begin("BTCHeater")) {
		DebugPort.println("MDNS responder started");
	}
	
//	server.on("/", handleBTCRoot);

	server.on("/wmconfig", handleWMConfig);
	server.on("/resetwifi", handleReset);
	server.on("/formatspiffs", handleFormat);

  // magical code shaemlessly lifted from Arduino WebUpdate example, slightly modified in paths
  // this allows pushing new firmware to the ESP via OTA from a WEB BROWSER!
  //
  // Initial launch page
  server.on("/update", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  // actual guts that manages the new firmware upload
  server.on("/updatenow", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      DebugPort.setDebugOutput(true);
      DebugPort.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin()) { //start with max available size
        Update.printError(DebugPort);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(DebugPort);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        DebugPort.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(DebugPort);
      }
      DebugPort.setDebugOutput(false);
    } else {
      DebugPort.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
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

