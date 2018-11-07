// 
// 
// 

#include "BTCWebServer.h"
#include "DebugPort.h"
#include "TxManage.h"

WebServer server(80);

const int led = 13;

void handleRoot() {
	digitalWrite(led, 1);
	server.send(200, "text/plain", "Chnage URL to /on to poweron heater... /off to poweroff");
	digitalWrite(led, 0);
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
	server.on("/on", webturnOn);
	server.on("/off", webturnOff);
	server.on("/", handleRoot);

	server.on("/inline", []() {
		server.send(200, "text/plain", "this works as well");
	});

	server.onNotFound(handleNotFound);

	server.begin();
	DebugPort.println("HTTP server started");
}

void doWebServer(void) {
	server.handleClient();
}

void webturnOn() {

	TxManage.queueOnRequest();
	server.send(200, "text/plain", "Heater Turning on");

}

void webturnOff() {

	TxManage.queueOffRequest();
	server.send(200, "text/plan", "Turning off heater");

}