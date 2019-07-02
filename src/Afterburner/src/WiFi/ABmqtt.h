// ABMqtt.h

#ifndef _ABMQTT_h
#define _ABMQTT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include <Arduino.h>
#else
	#include <WProgram.h>
#endif


#endif


#include "../Libraries/PubSubClient/src/PubSubClient.h"
#include "BTCWifi.h""
#include "BTCWebServer.h"

        
void MqttCallback(char* topic, byte* payload, unsigned int length);
void MqttSetup();
