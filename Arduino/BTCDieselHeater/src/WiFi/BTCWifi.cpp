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
void saveParamsCallback();
void APstartedCallback(WiFiManager*);

#define FAILEDSSID "BTCESP32"
#define FAILEDPASSWORD "thereisnospoon"

WiFiManager wm;

bool isPortalAP         = false;
bool isSTA              = false;
int TRIG_PIN;           //  which pin triggers the configuration portal when set to LOW

unsigned restartServer = 0;


bool initWifi(int initpin,const char *failedssid, const char *failedpassword) 
{
  TRIG_PIN = initpin;
  pinMode(TRIG_PIN, INPUT_PULLUP);

  uint8_t MAC[6];
  esp_read_mac(MAC, ESP_MAC_WIFI_STA);
  char msg[64];
  sprintf(msg, "  STA MAC address: %02X:%02X:%02X:%02X:%02X:%02X", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
  DebugPort.println(msg);
  esp_read_mac(MAC, ESP_MAC_WIFI_SOFTAP);
  sprintf(msg, "   AP MAC address: %02X:%02X:%02X:%02X:%02X:%02X", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
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
 
  DebugPort.println("Attempting to start STA mode (or config portal) via WifiManager...");

  wm.setHostname(failedssid);
  wm.setConfigPortalTimeout(20);
  wm.setConfigPortalBlocking(false);
  wm.setSaveParamsCallback(saveParamsCallback);  // ensure our webserver gets awoken when IP config changes to STA
  wm.setAPCallback(APstartedCallback);
  wm.setEnableConfigPortal(shouldBootIntoConfigPortal());
//REMOVED - UNSTABLE WHETHER WE GET 192.168.4.1 or 192.168.100.1 ????  
// REMOVED    wm.setAPStaticIPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255,255,255,0)); 
 
  bool res = wm.autoConnect(failedssid, failedpassword); // auto generated AP name from chipid
//  bool res = false;
  DebugPort.print("WifiMode after autoConnect = "); DebugPort.println(WiFi.getMode());

  int chnl = 1;
  bool retval = false;
  if(!res) {
    // failed STA mode
    DebugPort.println("WiFimanager failed STA connection. Setting up AP...");
  }    
  else {
    // runs through here if STA connected OK
    // if you get here you have connected to the WiFi    
    isSTA = true;
    DebugPort.println("WiFiManager connected in STA mode OK");
    DebugPort.print("  STA IP address: "); DebugPort.println(WiFi.localIP());
    // must use same radio channel as STA to go to STA+AP, otherwise we drop the STA!
    chnl = WiFi.channel();  
    DebugPort.print("Now promoting to STA+AP mode"); 
    retval = true;
  }
  // always setup an AP - for STA+AP mode we *must* use the same RF channel as STA
  DebugPort.println("Starting AP mode");
//REMOVED - UNSTABLE WHETHER WE GET 192.168.4.1 or 192.168.100.1 ????  
// REMOVED    WiFi.softAPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255,255,255,0));  
  WiFi.softAP(failedssid, failedpassword, chnl);
  WiFi.enableAP(true);
  DebugPort.print("  AP IP address: "); DebugPort.println(WiFi.softAPIP());
  DebugPort.print("WifiMode after initWifi = "); DebugPort.println(WiFi.getMode());

  // even though we may have started in STA mode - start the config portal if demanded via the NV flag
  if(shouldBootIntoConfigPortal()) {
    DebugPort.println("Manually starting web portal");
    wm.startWebPortal();
  }
  
  return retval;
}

void doWiFiManager()
{
  wm.process();

  if(restartServer) {
    long tDelta = millis() - restartServer;
    if(tDelta > 7500) {
      restartServer = 0;
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
  restartServer = millis() | 1;      // prepare to reboot in the near future - ensure non zero!
  prepBootIntoConfigPortal(false);   // ensure we fall back to SoftAP with our web page in future
  isPortalAP = false;
}

void APstartedCallback(WiFiManager*)
{
  isPortalAP = true;
}

const char* getWifiAPAddrStr()
{ 
  if(WiFi.getMode() & WIFI_MODE_AP)
    return WiFi.softAPIP().toString().c_str(); 
  else
    return NULL; 
};
  
const char* getWifiSTAAddrStr()
{ 
  if(WiFi.getMode() & WIFI_MODE_STA)
    return WiFi.localIP().toString().c_str(); 
  else
    return NULL; 
};
  

bool isWifiConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

bool isWifiAP()
{
  int mode = WiFi.getMode();
  return !isSTA && ((mode & WIFI_MODE_AP) != 0);   
}

bool isWifiSTA()
{
  return isSTA;  
}

bool isConfigPortal()
{
  return isPortalAP;
}


void prepBootIntoConfigPortal(bool state)
{
  Preferences NV;
  NV.begin("user");
  NV.putBool("bootPortal", state);
  NV.end();
  DebugPort.print("Setting boot config portal if WiFiManager fails = "); DebugPort.println(state);
}

bool shouldBootIntoConfigPortal()
{
  Preferences NV;
  NV.begin("user");
  bool retval = NV.getBool("bootPortal", false);
  NV.end();
  DebugPort.print("Boot config portal if WiFiManager fails = "); DebugPort.println(retval);
  return retval;
}

