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
// select which pin will trigger the configuration portal when set to LOW

#define FAILEDSSID "BTCESP32"
#define FAILEDPASSWORD "thereisnospoon"

WiFiManager wm;

unsigned int  timeout   = 120; // seconds to run for
unsigned int  startTime = millis();
bool isAP               = false;
bool portalRunning      = false;
bool startCP            = true; // start AP and webserver if true, else start only webserver
int TRIG_PIN;


void configModeCallback (WiFiManager *myWiFiManager);
void saveConfigCallback ();


void initWifi(int initpin,const char *failedssid, const char *failedpassword) 
{
  TRIG_PIN = initpin;
  pinMode(TRIG_PIN, INPUT_PULLUP);

  //reset settings - wipe credentials for testing
//  wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result
 
//  wm.setHostname("BTCDieselHeater");
  wm.setConfigPortalTimeout(120);
  wm.setConfigPortalBlocking(false);
//  wm.setAPCallback(configModeCallback);
//  wm.setSaveConfigCallback(saveConfigCallback); 

  bool res = wm.autoConnect(); // auto generated AP name from chipid
//  bool res = false;

  if(!res) {
    DebugPort.println("Failed to connect");
    DebugPort.println("Setting up ESP as AP");
    isAP = WiFi.softAP(failedssid, failedpassword);
    if(isAP) {
      WiFi.softAPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255,255,255,0));
    }
  } 
  else {
    //if you get here you have connected to the WiFi    
    DebugPort.println("connected...yeey :)");
    DebugPort.println("Ready");
    DebugPort.print("IP address: ");
    DebugPort.println(WiFi.localIP());
  }
}

void doWiFiManager(){
  // is auto timeout portal running
  if(portalRunning){
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
   }
  }

  // is configuration portal requested?
//  if(TRIG_PIN == 1 && (!portalRunning)) {
  if(digitalRead(TRIG_PIN) == LOW && !portalRunning) {
    if(startCP){
      DebugPort.println("Button Pressed, Starting Config Portal");
      wm.setConfigPortalBlocking(false);
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
  }
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

//callback que indica que o ESP entrou no modo AP
void configModeCallback (WiFiManager *myWiFiManager) {  
  DebugPort.println("Entered config mode");
  DebugPort.println(WiFi.softAPIP()); //imprime o IP do AP
  DebugPort.println(myWiFiManager->getConfigPortalSSID()); //imprime o SSID criado da rede</p><p>}</p><p>//callback que indica que salvamos uma nova rede para se conectar (modo estação)
}
void saveConfigCallback () {
  DebugPort.println("Should save config");
  DebugPort.println(WiFi.softAPIP()); //imprime o IP do AP
}

