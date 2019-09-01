// ABMqtt.h

#ifndef _ABMQTT_h
#define _ABMQTT_h


bool mqttInit();
void doMQTT();
bool mqttPublishJSON(const char* str);
void connectToMqtt();
void kickMQTT();


#endif

