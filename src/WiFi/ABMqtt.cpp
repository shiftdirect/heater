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

#define USE_RTOS_MQTTTIMER
//#define USE_LOCAL_MQTTSTRINGS

//IPAddress testMQTTserver(5, 196, 95, 208);   // test.mosquito.org
//IPAddress testMQTTserver(18, 194, 98, 249);  // broker.hivemq.com

AsyncMqttClient MQTTclient;
char topicnameJSONin[128];

#ifdef USE_LOCAL_MQTTSTRINGS
char mqttHost[128];
char mqttUser[32];
char mqttPass[32];
#endif

#ifdef USE_RTOS_MQTTTIMER
TimerHandle_t mqttReconnectTimer = NULL;
#else
unsigned long mqttReconnect = 0;
#endif

void connectToMqtt() {
#ifdef USE_RTOS_MQTTTIMER
  xTimerStop(mqttReconnectTimer, 0);
#else
  mqttReconnect = 0;
#endif
  if(!MQTTclient.connected()) {
    DebugPort.println("MQTT: Connecting...");
    if(NVstore.getMQTTinfo().enabled) {
      MQTTclient.connect();
    }
  }
}

void onMqttConnect(bool sessionPresent) 
{
#ifdef USE_RTOS_MQTTTIMER
  xTimerStop(mqttReconnectTimer, 0);
#else
  mqttReconnect = 0;
#endif

  DebugPort.println("MQTT: Connected to broker.");
//  DebugPort.printf("Session present: %d\r\n", sessionPresent);

  // create the topicname we use to accept incoming JSON
  DebugPort.printf("MQTT: base topic name \"%s\"\r\n", NVstore.getMQTTinfo().topic);
  sprintf(topicnameJSONin, "%s/JSONin", NVstore.getMQTTinfo().topic);
  // subscribe to that topic
  DebugPort.printf("MQTT: Subscribing to \"%s\"\r\n", topicnameJSONin);
  MQTTclient.subscribe(topicnameJSONin, NVstore.getMQTTinfo().qos);

  // spit out an "I'm here" message
  char lcltopic[128];
  sprintf(lcltopic, "%s/Status", NVstore.getMQTTinfo().topic);
  MQTTclient.publish(lcltopic, NVstore.getMQTTinfo().qos, true, "onMqttConnect");

#ifdef MQTT_DBG_LOOPBACK
  // testo - loopback
  sprintflcl(topic, "%s/JSONout", NVstore.getMQTTinfo().topic);
  MQTTclient.subscribe(lcltopic, NVstore.getMQTTinfo().qos);
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
  DebugPort.print("MQTT: Disconnected, reason: ");
  // ref: DisconnectReasons.hpp
  switch(reason) {
    case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED:             DebugPort.println("TCP disconnected"); break;
    case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:  DebugPort.println("protocol version"); break;
    case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED:     DebugPort.println("Identifier rejected"); break;
    case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE:      DebugPort.println("Server unavailable"); break;
    case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS:   DebugPort.println("Malformed credentials"); break;
    case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED:          DebugPort.println("No authorised"); break;
    case AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE:     DebugPort.println("Not enough space"); break;
    case AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT:          DebugPort.println("Bad TLS fingerprint"); break;
  }

  if (WiFi.isConnected()) {
    if(NVstore.getMQTTinfo().enabled) {
#ifdef USE_RTOS_MQTTTIMER      
      xTimerStart(mqttReconnectTimer, 0);
#else
      mqttReconnect = millis() + 5000;
#endif
    }
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  DebugPort.println("MQTT: Subscribe acknowledged.");
//  DebugPort.printf("  packetId: %d\r\n", packetId);
//  DebugPort.printf("  qos: %d\r\n", qos);
}

bool mqttInit() 
{
#ifdef USE_RTOS_MQTTTIMER      
  if(mqttReconnectTimer==NULL)  
    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
#else
  mqttReconnect = 0;
#endif

  memset(topicnameJSONin, 0, sizeof(topicnameJSONin));

  MQTTclient.disconnect(true);
  long escape = millis() + 10000;
  while(MQTTclient.connected()) {
    long tDelta = millis()-escape;
    if(tDelta > 0) {
      DebugPort.println("MQTT: TIMEOUT waiting for broker disconnect");
      break;
    }
  }

  const sMQTTparams params = NVstore.getMQTTinfo();
  if(params.enabled) {
#ifdef USE_LOCAL_MQTTSTRINGS
    strncpy(mqttHost, params.host, 127);
    strncpy(mqttUser, params.username, 31);
    strncpy(mqttPass, params.password, 31);
    mqttHost[127] = 0;
    mqttUser[31] = 0;
    mqttPass[31] = 0;
    DebugPort.printf("MQTT: setting broker to %s:%d\r\n", mqttHost, params.port);
    DebugPort.printf("MQTT: %s/%s\r\n", mqttUser, mqttPass);
    MQTTclient.setServer(mqttHost, params.port);
    MQTTclient.setCredentials(mqttUser, mqttPass);
#else
    // the client only stores a pointer - this must not be a volatile memory location! 
    // - NO STACK vars!!!
    DebugPort.printf("MQTT: setting broker to %s:%d\r\n", NVstore.getMQTTinfo().host, NVstore.getMQTTinfo().port);
    MQTTclient.setServer(NVstore.getMQTTinfo().host, NVstore.getMQTTinfo().port);
    DebugPort.printf("MQTT: %s/%s\r\n", NVstore.getMQTTinfo().username, NVstore.getMQTTinfo().password);
    MQTTclient.setCredentials(NVstore.getMQTTinfo().username, NVstore.getMQTTinfo().password);
#endif
    static bool setCallbacks = false;
    // callbacks should only be added once (vector of callbacks in client!)
    if(!setCallbacks) {
      MQTTclient.onConnect(onMqttConnect);
      MQTTclient.onMessage(onMqttMessage);
      MQTTclient.onDisconnect(onMqttDisconnect);
      MQTTclient.onSubscribe(onMqttSubscribe);
      setCallbacks = true;
    }
    // connection takes pplace via delayed start method
    return true;
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

void kickMQTT() {
  if (WiFi.isConnected()) {
    if(NVstore.getMQTTinfo().enabled) {
#ifdef USE_RTOS_MQTTTIMER      
      xTimerStart(mqttReconnectTimer, 0);
#else
      mqttReconnect = millis() + 5000;
#endif
    }
  }
}

void doMQTT()
{
  // most MQTT is managed via callbacks!!!
  if(NVstore.getMQTTinfo().enabled) {
#ifndef USE_RTOS_MQTTTIMER
    if(mqttReconnect) {
      long tDelta = millis() - mqttReconnect;
      if(tDelta > 0) {
        mqttReconnect = 0;
        connectToMqtt();
      }
    }
#endif

#ifdef USE_RTOS_MQTTTIMER
    if (!MQTTclient.connected() && WiFi.isConnected() && !xTimerIsTimerActive(mqttReconnectTimer)) {
      xTimerStart(mqttReconnectTimer, 0);
    }
#else
    if (!MQTTclient.connected() && WiFi.isConnected() && mqttReconnect==0) {
      mqttReconnect = millis() + 5000;
    }
#endif

  }

}

#endif