#include <Arduino.h>
#include "BTCWifi.h"
#include <TelnetSpy.h>

void initTelnetSpy();
void DoTelnetSpy();
void waitForConnection();
void waitForDisconnection();
void telnetConnected();
void telnetDisconnected();
