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

extern WiFiManager wm;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

bool bRxWebData = false;   // flags for OLED animation
bool bTxWebData = false;

const int led = 13;

void handleRoot() {
	String s = MAIN_PAGE; //Read HTML contents
	server.send(200, "text/html", s); //Send web page
}

void handleWMConfig() {
	DebugPort.println("Starting web portal for wifi config");
	wm.startWebPortal();
}
void handleReset() {
	server.send(200, "text/plain", "Resetting Wifi Settings!");
	DebugPort.println("diconnecting client and wifi, then rebooting");
	//client.disconnect();
//	wifi_station_disconnect();
	wm.disconnect();
	wm.resetSettings();

  delay(1000);

	ESP.restart();



}

void handleNotFound() {
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

void initWebServer(void) {

	
	if (MDNS.begin("BTCHeater")) {
		DebugPort.println("MDNS responder started");
	}
	
	server.on("/", handleRoot);
	server.on("/wmconfig", handleWMConfig);
	server.on("/resetwifi",handleReset);
	server.onNotFound(handleNotFound);

	server.begin();
	webSocket.begin();
	webSocket.onEvent(webSocketEvent);
	DebugPort.println("HTTP server started");

}

void stopWebServer()
{
	server.close();
  webSocket.close();
}

unsigned char cVal;

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
	if(webSocket.connectedClients()) {
    bTxWebData = true;              // OLED tx data animation flag
    webSocket.broadcastTXT(Str);
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

