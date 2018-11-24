// BTCWebServer.h

#ifndef _BTCWEBSERVER_h
#define _BTCWEBSERVER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "BTCWifi.h"
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "ESPAsyncWebServer.h"
#include "Index.h"
#include <WebSocketsServer.h>




#endif

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void initWebServer();
void doWebServer();
void handleRoot();
void handleNotFound();
void webturnOn();
void webturnOff();
