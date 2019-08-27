// 
// 
// 

#include "../cfg/BTCConfig.h"

#ifdef USE_MQTT
#include <Arduino.h>
#include "ABMqtt.h"
#include "../../lib/PubSubClient/src/PubSubClient.h"
#include "BTCWifi.h"
#include "BTCWebServer.h"
#include "../Utility/DebugPort.h"

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
  test[len];
  DebugPort.println(test);
	// handle message arrived
}

WiFiClient espClient;
PubSubClient MQTTclient(espClient);

void MqttSetup() {
  MQTTclient.setServer(MQTTserver, 1883);
  MQTTclient.setCallback(MqttCallback);
}

bool reconnect() {
  static unsigned long HoldOff = 0;

  if(HoldOff) {
    long tDelta = millis() - HoldOff;
    if(tDelta > 0)
      HoldOff = 0;
  }

  if(HoldOff == 0 && !MQTTclient.connected()) {
    DebugPort.println("Attempting MQTT connection");
    if (MQTTclient.connect("afterburnerClient")) {
      DebugPort.println("MQTT connected");
      // Once connected, publish an announcement...
      MQTTclient.publish("Afterburner/test", "hello world");
      // ... and resubscribe
//      MQTTclient.subscribe("inTopic");
//      MQTTclient.subscribe("Test/Test/Test/Test");
      MQTTclient.subscribe("Afterburner");
      MQTTclient.publish("Afterburner/test", "the end is nigh");
    }
    else {
      DebugPort.printf("MQTT connect failed, rc = %d, try again in 5 seconds\r\n", MQTTclient.state());
      HoldOff = millis() + 5000;
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