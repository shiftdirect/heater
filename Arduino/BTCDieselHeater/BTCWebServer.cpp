// 
// 
// 

#include "BTCWebServer.h"
#include "DebugPort.h"
#include "TxManage.h"
#include "helpers.h"
#include "pins.h"
#include "Index.h"

extern void Command_Interpret(const char* pLine);   // decodes received command lines, implemented in main .ino file!

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

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

void doWebServer(void) {
	static unsigned long lastTx = 0;
	webSocket.loop();
	server.handleClient();
	if(millis() > lastTx) {   // moderate the delivery of new messages - we simply cannot send every pass of the main loop!
		lastTx = millis() + 1000;
		char msg[16];
		sprintf(msg, "%.1f", getActualTemperature());
		webSocket.broadcastTXT(msg);
		// char c[] = { "23" };
		// webSocket.broadcastTXT(c, sizeof(c));
}

}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
	if (type == WStype_TEXT) {
		char cmd[16];
		memset(cmd, 0, 16);
		for (int i = 0; i < length && i < 15; i++) {
			cmd[i] = payload[i];
//				Serial.print((char)payload[i]);
		}
//			Serial.println();
    Serial.println(cmd);
		Command_Interpret(cmd);  // send to the main heater controller decode routine
}
}