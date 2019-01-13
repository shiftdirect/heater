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

// select which pin will trigger the configuration portal when set to LOW

#define FAILEDSSID "BTCESP32"
#define FAILEDPASSWORD "thereisnospoon"

WiFiManager wm;
extern void stopWebServer();
extern void initWebServer();
void saveParamsCallback();

unsigned int  timeout   = 120; // seconds to run for
unsigned int  startTime = millis();
bool isAP               = false;
bool portalRunning      = false;
bool startCP            = true;//true; // start AP and webserver if true, else start only webserver
int TRIG_PIN;
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
  sprintf(msg, "AP MAC address: %02X:%02X:%02X:%02X:%02X:%02X", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
  DebugPort.println(msg);

  //reset settings - wipe credentials for testing
//  wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result
 
  wm.setHostname(failedssid);
  wm.setConfigPortalTimeout(20);
  wm.setConfigPortalBlocking(false);
  wm.setSaveParamsCallback(saveParamsCallback);  // ensure our webserver gets awoken when IP config changes to STA
  wm.setEnableConfigPortal(false);
 
  bool res = wm.autoConnect(failedssid, failedpassword); // auto generated AP name from chipid
//  bool res = false;

  if(!res) {
    DebugPort.println("Failed to connect");
    DebugPort.println("Setting up ESP as AP");
//    WiFi.softAPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255,255,255,0));
    isAP = WiFi.softAP(failedssid, failedpassword);
//    WiFi.begin(failedssid);
//    if(isAP) {
//      WiFi.softAPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255,255,255,0));
//    }
    return false;
  } 
  else {
    //if you get here you have connected to the WiFi    
    DebugPort.println("connected...yeey :)");
    DebugPort.println("Ready");
    DebugPort.print("IP address: ");
    DebugPort.println(WiFi.localIP());
  }
  return true;
}

void doWiFiManager(){
    wm.process();

    if(startServer) {
      long tDelta = millis() - startServer;
      if(tDelta > 10000) {
        startServer = 0;
        initWebServer();
        ESP.restart();
      }
    }

  // is auto timeout portal running
/*  if(portalRunning){
    wm.process();
    long tDelta = millis() - startTime;
    if(tDelta > (timeout*1000)){
      DebugPort.println("portaltimeout");
      portalRunning = false;
      if(startCP){
        wm.stopConfigPortal();
      }  
      else{
        wm.stopWebPortal();
      } 
      wm.disconnect();
      WiFi.softAP("BTCDieselHeater");
      WiFi.softAPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255,255,255,0));
      initWebServer();
    }
  }

  // is configuration portal requested?
//  if(TRIG_PIN == 1 && (!portalRunning)) {
  if(digitalRead(TRIG_PIN) == LOW && !portalRunning) {
//    stopWebServer();
    if(startCP){
      DebugPort.println("Button Pressed, Starting Config Portal with new AP");
//      wm.setConfigPortalBlocking(false);
      wm.startConfigPortal();
//      TRIG_PIN = 0; // reset the flag
    }  
    else{
      DebugPort.println("Button Pressed, Starting Web Portal");
      wm.startWebPortal();
//      TRIG_PIN = 0; // reset the flag
    }  
    portalRunning = true;
    startTime = millis();
  }*/
}

void saveParamsCallback() 
{
  startServer = millis();
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

