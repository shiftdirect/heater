/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2018  James Clark
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

// Should be working - Jimmy C
#include "BTCWifi.h"
#include "../Utility/DebugPort.h"
#include <DNSServer.h>

#include "esp_system.h"
#include <Preferences.h>

// function to control the behaviour upon reboot if no wifi manager credentials exist
// or connection fails
void prepBootIntoConfigPortal(bool state);
bool shouldBootIntoConfigPortal();


#define FAILEDSSID "BTCESP32"
#define FAILEDPASSWORD "thereisnospoon"

WiFiManager wm;
extern void stopWebServer();
extern void initWebServer();
void saveParamsCallback();
void APstartedCallback(WiFiManager*);

unsigned int  timeout   = 120; // seconds to run for
unsigned int  startTime = millis();
bool isPortalAP         = false;
bool isAP               = false;
bool portalRunning      = false;
bool startCP            = true;//true; // start AP and webserver if true, else start only webserver
int TRIG_PIN;           //  which pin triggers the configuration portal when set to LOW

unsigned startServer = 0;


bool initWifi(int initpin,const char *failedssid, const char *failedpassword) 
{
  TRIG_PIN = initpin;
  pinMode(TRIG_PIN, INPUT_PULLUP);

  uint8_t MAC[6];
  esp_read_mac(MAC, ESP_MAC_WIFI_STA);
  char msg[64];
  sprintf(msg, "STA MAC address: %02X:%02X:%02X:%02X:%02X:%02X", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
  DebugPort.println(msg);
  esp_read_mac(MAC, ESP_MAC_WIFI_SOFTAP);
  sprintf(msg, " AP MAC address: %02X:%02X:%02X:%02X:%02X:%02X", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
  DebugPort.println(msg);

  //reset settings - wipe credentials for testing
//  wm.resetSettings();

  // Automatically connect using saved credentials:
  // WiFiManager will prepare a link connection, using stored credentials if available.
  //
  // NO CREDENTIALS: 
  //   Using a stored NV variable, we control the link creation via wm.setEnableConfigPortal():
  //     true -  SoftAP is created (SSID = failedssid), and linked to the config portal 
  //     false - we need to create a Soft AP, the portal does not start, we provide a web server
 //
  // WITH CREDENTIALS:
  //
  //   Connected to stored AP, AP provides an IP address to use, we are STA (station)
  //   failed to connect to stored AP, using a stored NV variable we control the behaviour via wm.setEnableConfigPortal():
  //     true -  SoftAP is created (SSID = failedssid), and linked to the config portal
  //     false - we need to create a Soft AP, the portal does not start, we provide a web server
 
  wm.setHostname(failedssid);
  wm.setConfigPortalTimeout(20);
  wm.setConfigPortalBlocking(false);
  wm.setSaveParamsCallback(saveParamsCallback);  // ensure our webserver gets awoken when IP config changes to STA
  wm.setAPCallback(APstartedCallback);
  wm.setEnableConfigPortal(shouldBootIntoConfigPortal());
  wm.setAPStaticIPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255,255,255,0));
 
  bool res = wm.autoConnect(failedssid, failedpassword); // auto generated AP name from chipid
//  bool res = false;

  if(!res) {
    // runs through here if we need to start our own soft AP to run THE web page
    DebugPort.println("Failed to connect");
    DebugPort.println("Setting up ESP as AP");
    // We need to start the soft AP 
    // - wifimanger has done most of the work, but has been left us high and dry :-)
    WiFi.softAPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255,255,255,0));
    delay(100);
    isAP = WiFi.softAP(failedssid, failedpassword);
    DebugPort.print("AP IP address: "); DebugPort.println(WiFi.softAPIP());
    return false;
  } 
  else {
    // runs through here is STA or portal config AP mode
    //if you get here you have connected to the WiFi    
    DebugPort.println("connected...yeey :)");
    DebugPort.println("Ready");
    if(isPortal())
      DebugPort.print("AP IP address: ");
    else
      DebugPort.print("STA IP address: ");
    DebugPort.println(WiFi.localIP());
  }
  return true;
}

void doWiFiManager()
{
  wm.process();

  if(startServer) {
    long tDelta = millis() - startServer;
    if(tDelta > 10000) {
      startServer = 0;
      initWebServer();
      ESP.restart();
    }
  }

  static bool pinDown = false;
  static long pinTime = 0;
  if(digitalRead(TRIG_PIN) == LOW) {
    if(!pinDown)
      pinTime = millis();
    pinDown = true;
  } 
  else {
    if(pinDown) {
      pinDown = false;
      unsigned long tDelta = millis() - pinTime;
      DebugPort.print("Wifi config button tDelta = "); DebugPort.println(tDelta);
      if(tDelta > 5000) {    // > 5 second press
        prepBootIntoConfigPortal(true);   // very long press - clear credentials, boot into portal
        wm.resetSettings();
        DebugPort.println("*** Clearing credentials and rebooting into portal ***");
        delay(1000);
        ESP.restart();
      }
      else if(tDelta > 1000) {    // > 1 second press
        prepBootIntoConfigPortal(false);   // long press - boot into SoftAP
        DebugPort.println("*** Rebooting into web server ***");
        delay(1000);
        ESP.restart();
      }
      else if(tDelta > 50) {
        prepBootIntoConfigPortal(true);    // short press - boot into Portal
        DebugPort.println("*** Rebooting into config portal ***");
        delay(1000);
        ESP.restart();
      }
      // contact bounce otherwise!
    }
  }
}

void saveParamsCallback() 
{
  startServer = millis();
  prepBootIntoConfigPortal(false);   // ensure we fall back to SoftAP with our web page in future
  isPortalAP = false;
  isAP = false;
}

void APstartedCallback(WiFiManager*)
{
  isPortalAP = true;
}

const char* getWifiAddrStr()
{ 
  if(isAP)
    return WiFi.softAPIP().toString().c_str(); 
  else
    return WiFi.localIP().toString().c_str(); 
};
  

bool isWifiConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

bool isWifiAP()
{
  return isAP;
}

bool isPortal()
{
  return isPortalAP;
}


void prepBootIntoConfigPortal(bool state)
{
  Preferences NV;
  NV.begin("user");
  NV.putBool("bootPortal", state);
  NV.end();
  DebugPort.print("Setting boot config portal if WiFiManager not configured = "); DebugPort.println(state);
}

bool shouldBootIntoConfigPortal()
{
  Preferences NV;
  NV.begin("user");
  bool retval = NV.getBool("bootPortal", false);
  NV.end();
  DebugPort.print("Boot config portal if WiFiManager not configured = "); DebugPort.println(retval);
  return retval;
}

