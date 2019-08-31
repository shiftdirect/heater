// 
// 
// 

#include "../cfg/BTCConfig.h"

#if USE_MQTT == 1
#include <Arduino.h>
#include "ABMqtt.h"
#include "../../lib/async-mqtt-client/src/AsyncMqttClient.h"
#include "BTCWifi.h"
#include "BTCWebServer.h"
#include "../Utility/DebugPort.h"
#include "../Utility/NVStorage.h"

//IPAddress testMQTTserver(5, 196, 95, 208);  // test.mosquito.org
IPAddress testMQTTserver(18, 194, 98, 249);  // broker.hivemq.com

AsyncMqttClient MQTTclient;
TimerHandle_t mqttReconnectTimer = NULL;
char topicnameJSONin[128];

void connectToMqtt() {
  DebugPort.println("MQTT: Connecting...");
  MQTTclient.connect();
}

void onMqttConnect(bool sessionPresent) 
{
  DebugPort.println("MQTT: Connected to broker.");
//  DebugPort.printf("Session present: %d\r\n", sessionPresent);

  const sMQTTparams params = NVstore.getMQTTinfo();
  char topic[128];
  DebugPort.printf("MQTT: base topic name \"%s\"\r\n", params.topic);
  sprintf(topicnameJSONin, "%s/JSONin", params.topic);
  DebugPort.printf("MQTT: Subscribing to \"%s\"\r\n", topicnameJSONin);
  MQTTclient.subscribe(topicnameJSONin, params.qos);
  // spit out an "I'm here" message
  sprintf(topic, "%s/Status", params.topic);
  MQTTclient.publish(topic, params.qos, true, "onMqttConnect");

#ifdef MQTT_DBG_LOOPBACK
  // testo - loopback
  sprintf(topic, "%s/JSONout", params.topic);
  MQTTclient.subscribe(topic, params.qos);
#endif

  resetJSONmoderator();
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) 
{
	// handle message arrived
  DebugPort.printf("MQTT: onMessage %s ", topic);
#ifdef MQTT_DBG_RAWBYTES
  for(int i=0; i<len; i++) {
    DebugPort.printf("0x%02X ", payload[i]);
  }
  DebugPort.println();
#endif
  // string may not neccesarily be null terminated, make sure it is
  char tidyString[1024];
  int maxlen = sizeof(tidyString)-1;
  int lenLimit = len < maxlen ? len : maxlen;
  strncpy(tidyString, (char*)payload, lenLimit);
  tidyString[lenLimit] = 0;
  DebugPort.println(tidyString);

  if(strcmp(topic, topicnameJSONin) == 0) {  // check if incoming topic is our JSONin topic
    interpretJsonCommand(tidyString);
  }
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  DebugPort.println("MQTT: Disconnected!");

//  if (WiFi.isConnected()) {
//    xTimerStart(mqttReconnectTimer, 0);
//  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  DebugPort.println("MQTT: Subscribe acknowledged.");
//  DebugPort.printf("  packetId: %d\r\n", packetId);
//  DebugPort.printf("  qos: %d\r\n", qos);
}

bool mqttInit() 
{
//  if(mqttReconnectTimer==NULL)  
//    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));

  memset(topicnameJSONin, 0, sizeof(topicnameJSONin));
  const sMQTTparams params = NVstore.getMQTTinfo();
  if(params.enabled) {
    if(strlen(params.host)) {
      MQTTclient.disconnect();
      long escape = millis() + 10000;
      while(MQTTclient.connected()) {
        long tDelta = millis()-escape;
        if(tDelta > 0) {
          DebugPort.println("MQTT: TIMEOUT waiting for broker disconnect");
          break;
        }
      }
      DebugPort.printf("MQTT: setting broker to %s:%d\r\n", params.host, params.port);
      MQTTclient.setServer(params.host, params.port);
      MQTTclient.setCredentials(params.username, params.password);
      static bool setCallbacks = false;
      // callbacks should only be added once (vector of callbacks in client!)
      if(!setCallbacks) {
        MQTTclient.onConnect(onMqttConnect);
        MQTTclient.onMessage(onMqttMessage);
        MQTTclient.onDisconnect(onMqttDisconnect);
        MQTTclient.onSubscribe(onMqttSubscribe);
        setCallbacks = true;
      }
      MQTTclient.connect();
      return true;
    }
  }
  return false;
}

bool mqttPublishJSON(const char* str)
{
  if(MQTTclient.connected()) {
    const sMQTTparams params = NVstore.getMQTTinfo();
    char topic[128];
    sprintf(topic, "%s/JSONout", params.topic);
    MQTTclient.publish(topic, params.qos, false, str);
    return true;
  }
  return false;
}


#endif