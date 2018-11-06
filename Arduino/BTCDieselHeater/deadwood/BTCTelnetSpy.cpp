//#define SERIAL  DebugPort
#define SERIAL  SerialAndTelnet   
#include "BTCTelnetSpy.h"

TelnetSpy SerialAndTelnet;

void waitForConnection() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    SerialAndTelnet.print(".");
  }
  SerialAndTelnet.println(" Connected!");
}

void waitForDisconnection() {
  while (WiFi.status() == WL_CONNECTED) {
    delay(500);
    SerialAndTelnet.print(".");
  }
  SerialAndTelnet.println(" Disconnected!");
}

void telnetConnected() {
  SerialAndTelnet.println("Telnet connection established.");
}

void telnetDisconnected() {
  SerialAndTelnet.println("Telnet connection closed.");
}

void initTelnetSpy() {
  SerialAndTelnet.setWelcomeMsg("Welcome to the TelnetSpy example\n\n");
  SerialAndTelnet.setCallbackOnConnect(telnetConnected);
  SerialAndTelnet.setCallbackOnDisconnect(telnetDisconnected);


  waitForConnection();
/*
  SerialAndTelnet.println("Ready");
  SerialAndTelnet.print("IP address: ");
  SerialAndTelnet.println(WiFi.localIP());
  
  SerialAndTelnet.println("\nType 'C' for WiFi connect.\nType 'D' for WiFi disconnect.\nType 'R' for WiFi reconnect.");
  SerialAndTelnet.println("All other chars will be echoed. Play around...\n");
*/
}

void DoTelnetSpy() {
  SerialAndTelnet.handle();
}
