// 
// 
// 

#include "BTCWebServer.h"
#include "DebugPort.h"
#include "TxManage.h"

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
unsigned char cVal;

void doWebServer(void) {
	webSocket.loop();
	server.handleClient();
	char c[] = { "23" };
	webSocket.broadcastTXT(c, sizeof(c));

}

extern void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
	if (type == WStype_TEXT) {
			for (int i = 0; i < length; i++)
				Serial.print((char)payload[i]);
			Serial.println();
		}
	}