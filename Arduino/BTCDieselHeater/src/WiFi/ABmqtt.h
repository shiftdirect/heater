// ABMqtt.h

#ifndef _ABMQTT_h
#define _ABMQTT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#endif


#include <PubSubClient.h>
#include "BTCWifi.h""
#include "BTCWebServer.h"

        
void MqttCallback(char* topic, byte* payload, unsigned int length);
void MqttSetup();
