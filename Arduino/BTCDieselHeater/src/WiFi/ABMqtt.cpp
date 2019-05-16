// 
// 
// 

#ifdef USEMQTT
#include "ABMqtt.h"


void MqttCallback(char* topic, byte* payload, unsigned int length) {
	// handle message arrived
}

void MqttSetup() {
	WiFiClient espClient;
	PubSubClient client(espClient);

	long lastReconnectAttempt = 0;

	boolean reconnect() {
		if (client.connect("arduinoClient")) {
			// Once connected, publish an announcement...
			client.publish("outTopic", "hello world");
			// ... and resubscribe
			client.subscribe("inTopic");
		}
		return client.connected();
	}
}
#endif