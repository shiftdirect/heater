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
#include "DebugPort.h"
#include "TxManage.h"
#include "helpers.h"
#include "pins.h"
#include "Index.h"
#include <ArduinoJson.h>
#include "Moderator.h"

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

bool bRxWebData = false;   // flags for OLED animation
bool bTxWebData = false;

CModerator Moderator;         // check for settings that are not actually changing, avoid sending these

const int led = 13;

void handleRoot() {
	String s = MAIN_PAGE; //Read HTML contents
	server.send(200, "text/html", s); //Send web page
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
	server.onNotFound(handleNotFound);

	server.begin();
	webSocket.begin();
	webSocket.onEvent(webSocketEvent);
	DebugPort.println("HTTP server started");

}
unsigned char cVal;

bool doWebServer(void) {
	static unsigned long lastTx = 0;
	static int prevNumClients;
	webSocket.loop();
	server.handleClient();

	int numClients = webSocket.connectedClients();
	if(numClients != prevNumClients) {
		prevNumClients = numClients;
		Moderator.reset();   // force full update of params if number of clients change
		DebugPort.println("Changed number of web clients, resetting history");
	}

	if(numClients) {
		if(millis() > lastTx) {   // moderate the delivery of new messages - we simply cannot send every pass of the main loop!
			lastTx = millis() + 100;

      StaticJsonBuffer<512> jsonBuffer;   // create a JSON buffer on the heap
			JsonObject& root = jsonBuffer.createObject();  // create object to add JSON commands to

			bool bSend = false;  // reset should send flag

			float tidyTemp = int(getActualTemperature() * 10) * 0.1f;  // round to 0.1 resolution 
			bSend |= Moderator.send("CurrentTemp", tidyTemp, root); 
			bSend |= Moderator.send("DesiredTemp", getHeaterInfo().getTemperature_Desired(), root); 
			bSend |= Moderator.send("RunState", getHeaterInfo().getRunState(), root); 
			bSend |= Moderator.send("ErrorState", getHeaterInfo().getErrState(), root );


      if(bSend) {
				bTxWebData = true;              // OLED tx data animation flag
      	char jsonToSend[512];
				root.printTo(jsonToSend);
      	webSocket.broadcastTXT(jsonToSend);
			}
		}
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
//    DebugPort.println(cmd);
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


