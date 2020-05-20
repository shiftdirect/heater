/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2019  Ray Jones <ray@mrjones.id.au>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */


#include "../cfg/BTCConfig.h"

#if USE_MQTT == 1

//#define BLOCK_MQTT_RECON


#include <Arduino.h>
#include "ABMQTT.h"
#include "../../lib/async-mqtt-client/src/AsyncMqttClient.h"
#include "BTCWifi.h"
#include "BTCWebServer.h"
#include "../Utility/DebugPort.h"
#include "../Utility/NVStorage.h"
#include "../Utility/Moderator.h"
#include "../Protocol/Protocol.h"
#include "../Utility/BTC_JSON.h"
#include "../Utility/TempSense.h"
#include "../Utility/DemandManager.h"
#include "../Utility/FuelGauge.h"
#include "../Utility/BoardDetect.h"
#include "../Utility/NVStorage.h"
#include <FreeRTOS.h>

extern void DecodeCmd(const char* cmd, String& payload);

//IPAddress testMQTTserver(5, 196, 95, 208);   // test.mosquito.org
//IPAddress testMQTTserver(18, 194, 98, 249);  // broker.hivemq.com

AsyncMqttClient MQTTclient;
char topicnameJSONin[128];
char topicnameCmd[128];
CModerator BasicMQTTmoderator;  // for basic MQTT interface
unsigned long MQTTrestart = 0;
char statusTopic[128];
TimerHandle_t mqttReconnectTimer = NULL;
SemaphoreHandle_t mqttSemaphore = NULL;

void subscribe(const char* topic);



void connectToMqtt() {
  xTimerStop(mqttReconnectTimer, 0);
  if(!MQTTclient.connected()) {
    DebugPort.println("MQTT: Connecting...");
    if(NVstore.getMQTTinfo().enabled) {
      MQTTclient.connect();
    }
  }
}

// timer callbacks are called from ISRL
// ALL callback code MUST run from IRAM, safest to pass the event down thru a queue to user code
void IRAM_ATTR callbackMQTTreconnect() {
  BaseType_t awoken;
  xSemaphoreGiveFromISR(mqttSemaphore, &awoken);
}

void onMqttConnect(bool sessionPresent) 
{
  xTimerStop(mqttReconnectTimer, 0);

  DebugPort.println("MQTT: Connected to broker.");
//  DebugPort.printf("Session present: %d\r\n", sessionPresent);

  // apply the topicname we need to subscribe to incoming JSON
  
  DebugPort.printf("MQTT: topic prefix name \"%s\"\r\n", getTopicPrefix());
  sprintf(statusTopic, "%s/status", getTopicPrefix());
  sprintf(topicnameJSONin, "%s/JSONin", getTopicPrefix());
  sprintf(topicnameCmd, "%s/cmd/#", getTopicPrefix());
  
  subscribe(topicnameJSONin);     // subscribe to the JSONin topic
  subscribe(topicnameCmd);        // subscribe to the basic command topic
  subscribe(statusTopic);         // subscribe to the status topic

  // spit out an "I'm here" message
  MQTTclient.publish(statusTopic, NVstore.getMQTTinfo().qos, true, "online");
  // and a will if we die unexpectedly
  MQTTclient.setWill(statusTopic, NVstore.getMQTTinfo().qos, true, "offline");

#ifdef MQTT_DBG_LOOPBACK
  // testo - loopback
  sprintflcl(topic, "%s/JSONout", NVstore.getMQTTinfo().topic);
  MQTTclient.subscribe(lcltopic, NVstore.getMQTTinfo().qos);
#endif

  resetAllJSONmoderators();
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) 
{
	// handle message arrived
  DebugPort.printf("MQTT: onMessage %s ", topic);
  // string may not neccesarily be null terminated, make sure it is
  char szPayload[1024];
  int maxlen = sizeof(szPayload)-1;
  int lenLimit = len < maxlen ? len : maxlen;
  strncpy(szPayload, (char*)payload, lenLimit);
  szPayload[lenLimit] = 0;
  DebugPort.println(szPayload);

  if(strcmp(topic, topicnameJSONin) == 0) {  // check if incoming topic is our JSONin topic
    interpretJsonCommand(szPayload);
  }
  else if(strncmp(topic, topicnameCmd, strlen(topicnameCmd)-1) == 0) {  // check if incoming topic is our cmd topic
    const char* cmdTopic = &topic[strlen(topicnameCmd)-1];
    DebugPort.printf("%s %s %s\r\n", topicnameCmd, cmdTopic, szPayload);
    String cmdPayload(szPayload);
    DecodeCmd(cmdTopic, cmdPayload);
  }
  else if(strcmp(topic, statusTopic) == 0) {  // check if incoming topic is our general status
    if(strcmp(szPayload, "1") == 0) {
      BasicMQTTmoderator.reset();
      MQTTclient.publish(statusTopic, NVstore.getMQTTinfo().qos, true, "online");
    }
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
      xTimerStart(mqttReconnectTimer, 0);
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
#ifndef BLOCK_MQTT_RECON
  if(mqttReconnectTimer==NULL)  
    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(20000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(callbackMQTTreconnect));
  if(mqttSemaphore == NULL) {
    mqttSemaphore = xSemaphoreCreateBinary();
  }
#endif
  MQTTrestart = 0;

  memset(topicnameJSONin, 0, sizeof(topicnameJSONin));

  DebugPort.println("MQTT: Initialising...");
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
    // the client only stores a pointer - this must not be a volatile memory location! 
    // - NO STACK vars!!!
    DebugPort.printf("MQTT: setting broker to %s:%d\r\n", NVstore.getMQTTinfo().host, NVstore.getMQTTinfo().port);
    MQTTclient.setServer(NVstore.getMQTTinfo().host, NVstore.getMQTTinfo().port);
    DebugPort.printf("MQTT: %s/%s\r\n", NVstore.getMQTTinfo().username, NVstore.getMQTTinfo().password);
    MQTTclient.setCredentials(NVstore.getMQTTinfo().username, NVstore.getMQTTinfo().password);

    static bool setCallbacks = false;
    // callbacks should only be added once (vector of callbacks in client!)
    if(!setCallbacks) {
      MQTTclient.onConnect(onMqttConnect);
      MQTTclient.onMessage(onMqttMessage);
      MQTTclient.onDisconnect(onMqttDisconnect);
      MQTTclient.onSubscribe(onMqttSubscribe);
      setCallbacks = true;
    }
    // connection takes place via delayed start method
    return true;
  }
  return false;
}

bool mqttPublishJSON(const char* str)
{
  if(MQTTclient.connected()) {
    const sMQTTparams params = NVstore.getMQTTinfo();
    char topic[128];
    sprintf(topic, "%s/JSONout", getTopicPrefix());
    MQTTclient.publish(topic, params.qos, false, str);
    return true;
  }
  return false;
}

void doMQTT()
{
  // manage restart of MQTT
  if(MQTTrestart) {
    long tDelta = millis() - MQTTrestart;
    if(tDelta > 0) {
      MQTTrestart = 0;
      mqttInit();
    }
  }

  // most MQTT is managed via callbacks!!!
  if(NVstore.getMQTTinfo().enabled) {
    
    // test if MQTT start timeout has expired
    if(xSemaphoreTake(mqttSemaphore, 0)) {
      DebugPort.println("MQTT connect request via semaphore");
      connectToMqtt();
    }

#ifndef BLOCK_MQTT_RECON
    if (!MQTTclient.connected() && WiFi.isConnected() && !xTimerIsTimerActive(mqttReconnectTimer)) {
      DebugPort.println("Starting MQTT timer");
      xTimerStart(mqttReconnectTimer, 0);
    }
#endif

  }

}

bool isMQTTconnected() {
  return MQTTclient.connected();
}


void pubTopic(const char* name, int value) 
{
  if(MQTTclient.connected()) {
    if(BasicMQTTmoderator.shouldSend(name, value)) {
      const sMQTTparams params = NVstore.getMQTTinfo();
      char topic[128];
      sprintf(topic, "%s/sts/%s", getTopicPrefix(), name);
      char payload[128];
      sprintf(payload, "%d", value);
      MQTTclient.publish(topic, params.qos, false, payload);
    }
  }
}

void pubTopic(const char* name, float value) 
{
  if(MQTTclient.connected()) {
    if(BasicMQTTmoderator.shouldSend(name, value)) {
      const sMQTTparams params = NVstore.getMQTTinfo();
      char topic[128];
      sprintf(topic, "%s/sts/%s", getTopicPrefix(), name);
      char payload[128];
      sprintf(payload, "%.1f", value);
      MQTTclient.publish(topic, params.qos, false, payload);
    }
  }
}

void pubTopic(const char* name, const char* payload) 
{
  if(MQTTclient.connected()) {
    if(BasicMQTTmoderator.shouldSend(name, payload)) {
      const sMQTTparams params = NVstore.getMQTTinfo();
      char topic[128];
      sprintf(topic, "%s/sts/%s", getTopicPrefix(), name);
      MQTTclient.publish(topic, params.qos, false, payload);
    }
  }
}

void updateMQTT()
{
  pubTopic("RunState", getHeaterInfo().getRunStateEx());
  pubTopic("Run", getHeaterInfo().getRunStateEx() ? "1" : "0");
  pubTopic("RunString", getHeaterInfo().getRunStateStr());

  float tidyTemp;
  if(getTempSensor().getTemperature(0, tidyTemp)) {
    tidyTemp = int(tidyTemp * 10 + 0.5) * 0.1f;  // round to 0.1 resolution 
    pubTopic("TempCurrent", tidyTemp); 
  }
  else
    pubTopic("TempCurrent", "n/a"); 
  if(getTempSensor().getNumSensors() > 1) {
    if(getTempSensor().getTemperature(1, tidyTemp)) {
      tidyTemp = int(tidyTemp * 10 + 0.5) * 0.1f;  // round to 0.1 resolution 
      pubTopic("Temp2Current", tidyTemp); 
    }
    else
      pubTopic("Temp2Current", "n/a"); 
    if(getTempSensor().getNumSensors() > 2) {
      if(getTempSensor().getTemperature(2, tidyTemp)) {
        tidyTemp = int(tidyTemp * 10 + 0.5) * 0.1f;  // round to 0.1 resolution 
        pubTopic("Temp3Current", tidyTemp); 
      }
      else
        pubTopic("Temp3Current", "n/a"); 
    }
    if(getTempSensor().getNumSensors() > 3) {
      if(getTempSensor().getTemperature(3, tidyTemp)) {
        tidyTemp = int(tidyTemp * 10 + 0.5) * 0.1f;  // round to 0.1 resolution 
        pubTopic("Temp4Current", tidyTemp); 
      }
      else
        pubTopic("Temp4Current", "n/a"); 
    }
  }
  pubTopic("TempDesired", CDemandManager::getDemand()); 
  pubTopic("TempBody", getHeaterInfo().getTemperature_HeatExchg()); 
  pubTopic("ErrorState", getHeaterInfo().getErrState());
  pubTopic("ErrorString", getHeaterInfo().getErrStateStrEx()); // verbose it up!
  pubTopic("Thermostat", CDemandManager::isThermostat());
  pubTopic("PumpFixed", getHeaterInfo().getPump_Fixed() );
  pubTopic("PumpActual", getHeaterInfo().getPump_Actual());
  pubTopic("FanRPM", getFanSpeed());
  pubTopic("InputVoltage", getBatteryVoltage(false));
  pubTopic("GlowVoltage", getGlowVolts());
  pubTopic("GlowCurrent", getGlowCurrent());
  if(getBoardRevision() != BRD_V2_GPIO_NOALG && getBoardRevision() != BRD_V3_GPIO_NOALG) {          // has GPIO support
    sGPIO info;
    getGPIOinfo(info);
    pubTopic("GPanlg", info.algVal * 100 / 4096); 
  }
  pubTopic("FuelUsage", FuelGauge.Used_mL());
  float fuelRate = getHeaterInfo().getPump_Actual() * NVstore.getHeaterTuning().pumpCal * 60 * 60;
  pubTopic("FuelRate", fuelRate);

}

void refreshMQTT()
{
  BasicMQTTmoderator.reset();
}

void subscribe(const char* topic)
{
  DebugPort.printf("MQTT: Subscribing to \"%s\"\r\n", topic);
  MQTTclient.subscribe(topic, NVstore.getMQTTinfo().qos);
}

void requestMQTTrestart()
{
  MQTTrestart = (millis() + 1000) | 1;
}

const char* getTopicPrefix() 
{
#ifdef ALLOW_USER_TOPIC

  return NVstore.getMQTTinfo().topicPrefix;

#else

  static char prefix[32] = "";

  if(strlen(prefix) == 0) {
    uint8_t MAC[6];
    esp_read_mac(MAC, ESP_MAC_WIFI_STA);
    sprintf(prefix, "Afterburner%02X%02X%02X", MAC[3], MAC[4], MAC[5]);
  }
  return prefix;

#endif
}

#endif