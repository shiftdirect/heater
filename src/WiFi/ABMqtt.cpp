// 
// 
// 

#include "../cfg/BTCConfig.h"

#if USE_MQTT == 1
#include <Arduino.h>
#include "ABMqtt.h"
#include "../../lib/aysync-mqtt-client/src/AsyncMqttClient.h"
#include "BTCWifi.h"
#include "BTCWebServer.h"
#include "../Utility/DebugPort.h"
#include "../Utility/NVStorage.h"

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(172, 16, 0, 100);
IPAddress MQTTserver(5, 196, 95, 208);

void MqttCallback(char* topic, byte* payload, unsigned int length) {
  DebugPort.printf("MQTT callback %s ", topic);
  for(int i=0; i<length; i++) {
    DebugPort.printf("0x%02X ", payload[i]);
  }
  DebugPort.println();
  char test[256];
  int len = length < 256 ? length : 255;
  strncpy(test, (char*)payload, len);
  test[len] = 0;
  DebugPort.println(test);
	// handle message arrived
}

//WiFiClient espClient;
AsyncMqttClient MQTTclient;

bool MqttSetup() {
  MQTTclient.setServer(MQTTserver, 1883);
  espClient.setTimeout(5);
  const sMQTTparams params = NVstore.getMQTTinfo();
  if(params.enabled) {
    if(strlen(params.host)) {
//      DebugPort.printf("MQTT: setting server to %s:%d\r\n", params.host, params.port);
//      MQTTclient.setServer(params.host, params.port);
      MQTTclient.setCallback(MqttCallback);
      return true;
    }
  }
  return false;
}

bool reconnect() {
  static unsigned long HoldOff = 0;

  if(HoldOff) {
    long tDelta = millis() - HoldOff;
    if(tDelta > 0)
      HoldOff = 0;
    return false;
  }

  if(MqttSetup()) {

    if(NVstore.getMQTTinfo().enabled && HoldOff == 0 && !MQTTclient.connected()) {
      DebugPort.println("Attempting MQTT connection");
      if (MQTTclient.connect("afterburnerClient")) {
        DebugPort.println("MQTT connected");
        // Once connected, publish an announcement...
  //      MQTTclient.publish("Afterburner/test", "hello world");
        // ... and resubscribe
        const sMQTTparams params = NVstore.getMQTTinfo();
        char topic[128];
        sprintf(topic, "%s/JSONin", params.topic);
        MQTTclient.subscribe(topic);
  //      MQTTclient.publish("Afterburner/test", "the end is nigh");
      }
      else {
        DebugPort.printf("MQTT connect failed, rc = %d, try again in 5 seconds\r\n", MQTTclient.state());
        HoldOff = millis() + 5000;
      }
    }
  }
  return MQTTclient.connected();
}

void doMQTT()
{
  if(reconnect())
    MQTTclient.loop();
}

#endif