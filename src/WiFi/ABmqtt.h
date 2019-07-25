// ABMqtt.h

#ifndef _ABMQTT_h
#define _ABMQTT_h


// #include "../../lib/PubSubClient/src/PubSubClient.h"
// #include "BTCWifi.h""
// #include "BTCWebServer.h"

        
void MqttCallback(char* topic, byte* payload, unsigned int length);
void MqttSetup();

#endif

