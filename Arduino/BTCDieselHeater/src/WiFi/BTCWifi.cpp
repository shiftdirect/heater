/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2019  Ray Jones
 * Copyright (C) 2019  James Clark
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
#include "../OLED/ScreenManager.h"

#include "esp_system.h"
#include <Preferences.h>
#include "../Utility/NVStorage.h"

#define USE_AP  

// function to control the behaviour upon reboot if no wifi manager credentials exist
// or connection fails
void prepBootIntoConfigPortal(bool state);
bool shouldBootIntoConfigPortal();
void saveParamsCallback();
void APstartedCallback(WiFiManager*);

WiFiManager wm;

bool isPortalAP         = false;   // true if config portal is running
bool isSTA              = false;   // true if connected to an access point
int TRIG_PIN;                      // pin that triggers the configuration portal when set to LOW
unsigned restartServer = 0;        // set to time of portal reconfig - will cause reboot a while later
char MACstr[2][20];                // MACstr[0] STA, MACstr[1] = AP
int wifiButtonState = 0;

extern CScreenManager ScreenManager;


bool initWifi(int initpin,const char *failedssid, const char *failedpassword) 
{
  
  TRIG_PIN = initpin;
  pinMode(TRIG_PIN, INPUT_PULLUP);

  // report the MAC addresses - note individual values for STA and AP modes
  uint8_t MAC[6];
  esp_read_mac(MAC, ESP_MAC_WIFI_STA);
  sprintf(MACstr[0], "%02X:%02X:%02X:%02X:%02X:%02X", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
  DebugPort.printf("  STA MAC address: %s\r\n", MACstr[0]);
  esp_read_mac(MAC, ESP_MAC_WIFI_SOFTAP);
  sprintf(MACstr[1], "%02X:%02X:%02X:%02X:%02X:%02X", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
  DebugPort.printf("   AP MAC address: %s\r\n", MACstr[1]);

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
  DebugPort.printf("WifiMode after autoConnect = "); DebugPort.println(WiFi.getMode());

  int chnl = 1;
  bool retval = false;
  if(!res) {
    // failed STA mode
    DebugPort.println("WiFimanager failed STA connection. Setting up AP...");
    WiFi.disconnect();  // apparently needed for AP only OTA to reboot properly!!!
  }    
  else {
    // runs through here if STA connected OK
    // if you get here you have connected to the WiFi    
    isSTA = true;
    DebugPort.println("WiFiManager connected in STA mode OK");
    DebugPort.printf("  STA IP address: %s\r\n", getWifiSTAAddrStr());
    // must use same radio channel as STA to go to STA+AP, otherwise we drop the STA!
    chnl = WiFi.channel();  
    DebugPort.println("Now promoting to STA+AP mode..."); 
    retval = true;
  }
#ifdef USE_AP  
  // always setup an AP - for STA+AP mode we *must* use the same RF channel as STA
  DebugPort.println("Starting AP mode");
//REMOVED - UNSTABLE WHETHER WE GET 192.168.4.1 or 192.168.100.1 ????  
// REMOVED    WiFi.softAPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255,255,255,0));  
  WiFi.softAP(failedssid, failedpassword, chnl);
  WiFi.enableAP(true);
  DebugPort.printf("  AP SSID: %s\r\n", WiFi.softAPgetHostname());
  DebugPort.printf("  AP IP address: %s\r\n", getWifiAPAddrStr());
  DebugPort.printf("WifiMode after initWifi = %d\r\n", WiFi.getMode());
  #endif

  // even though we may have started in STA mode - start the config portal if demanded via the NV flag
  if(shouldBootIntoConfigPortal()) {
    DebugPort.println("Manually starting web portal");
    wm.startWebPortal();
    isPortalAP = true;                   // we started portal, we have to flag it!
  }

  return retval;
}

// call from main sketch loop()
void doWiFiManager()
{
  wm.process();

#if USE_PORTAL_TRIGGER_PIN == 1
  // manage handling of pin to enter WiFManager config portal
  // we typically use the BOOT pin for this (pins.h)
  //
  // Quick Press (< 1 sec)      - enable config portal
  // > 1 second (< 5 sec) press - disable config portal
  // > 5 second press           - erase credentials, enable config portal
  static bool pinDown = false;
  static long pinTime = 0;
  unsigned long tDelta;

  if(digitalRead(TRIG_PIN) == LOW) {
    if(!pinDown) {
      pinTime = millis();
      ScreenManager.reqUpdate();
    }
    pinDown = true;
    // track hold duration - change OLED Wifi annotation according to length of press
    tDelta = millis() - pinTime;
    if(tDelta > 5000)
      wifiButtonState = 3;        // we will show 'ERS' on OLED!
    else if(tDelta > 1000)
      wifiButtonState = 2;        // we will show 'HTR' on OLED!
    else
      wifiButtonState = 1;        // we will show 'CFG' on OLED!
  } 
  else {
    if(pinDown) {
      pinDown = false;
      tDelta = millis() - pinTime;
      DebugPort.printf("Wifi config button tDelta = %ld\r\n", tDelta);
      // > 5 second press?
      if(tDelta > 5000) {    
        wifiEnterConfigPortal(true, true);  // very long press - clear credentials, reboot into portal
      }
      // > 1 second press?
      else if(tDelta > 1000) {    
        wifiEnterConfigPortal(false);   // long press - reboot into web server
      }
      // > 50ms press?
      else if(tDelta > 50) {
        wifiEnterConfigPortal(true);    // quick press - reboot into portal
      }
      // consider as contact bounce if < 50ms!
    }
  }
#endif
}

void wifiDisable(long rebootDelay) 
{
  NVstore.setWifiEnabled(0); 
  NVstore.save(); 

  DebugPort.println("*** Disabling WiFi ***");

  restartServer = (millis() + rebootDelay) | 1;      // prepare to reboot in the future - ensure non zero!

  const char* content[2];
  content[0] = "WiFi Mode \032 DISABLED";
  content[1] = "";
  ScreenManager.showRebootMsg(content, rebootDelay);
}

void wifiEnterConfigPortal(bool state, bool erase, long rebootDelay) 
{
	wm.disconnect();

  NVstore.setWifiEnabled(1); 
  NVstore.save(); 

  prepBootIntoConfigPortal(state);  

  const char* content[2];

  if(isWifiSTA() && !erase)
    content[0] = "WiFi Mode \032 STA+AP";
  else 
    content[0] = "WiFi Mode \032 AP only";

  if(erase) {
    wm.resetSettings();
    DebugPort.println("*** Erased wifi credentials ***");
  } 

  if(state) {
    DebugPort.println("*** Rebooting into config portal ***");
    content[1] = "Web \032 Config Portal";
  }
  else {
    DebugPort.println("*** Rebooting into web server ***");
    content[1] = "Web \032 Heater control";
  }

  restartServer = (millis() + rebootDelay) | 1;      // prepare to reboot in the future - ensure non zero!

  ScreenManager.showRebootMsg(content, rebootDelay);
}

void wifiFactoryDefault()
{
  wm.resetSettings();
  prepBootIntoConfigPortal(false);
  NVstore.setWifiEnabled(1); 
}

// callback is invoked by WiFiManager after new credentials are saved and verified
void saveParamsCallback() 
{
  wifiEnterConfigPortal(false);  // stop config portal, reboot
}

// callback called if the WiFiManager started the config portal
void APstartedCallback(WiFiManager*)
{
  isPortalAP = true;                  // will add CFG adornment to OLED WiFi icon
}

const char* getWifiAPAddrStr()
{ 
  noInterrupts();
  static IPAddress IPaddr = WiFi.softAPIP();   // use stepping stone - function returns an automatic stack var - LAME!
  interrupts();
  return IPaddr.toString().c_str(); 
}
  
const char* getWifiSTAAddrStr()
{ 
  noInterrupts();
  static IPAddress IPaddr = WiFi.localIP();    // use stepping stone - function returns an automatic stack var - LAME!
  interrupts();
  return IPaddr.toString().c_str(); 
}
  
const char* getWifiAPMACStr()
{ 
  return MACstr[1]; 
}
  
const char* getWifiSTAMACStr()
{ 
  return MACstr[0]; 
}

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
  return isSTA;             // true: STAtion mode link is active
}

bool isWifiConfigPortal()
{
  return isPortalAP;        // true: config portal is running
}

// save an NV flag to determine whether config portal should run after reboot
void prepBootIntoConfigPortal(bool state)
{
  Preferences NV;
  NV.begin("user");
  NV.putBool("bootPortal", state);
  NV.end();
  DebugPort.printf("Setting boot config portal if WiFiManager fails = %d\r\n", state);
}

// test the NV flag whether the config portal should run after reboot
bool shouldBootIntoConfigPortal()
{
  Preferences NV;
  NV.begin("user");
  bool retval = NV.getBool("bootPortal", false);
  NV.end();
  DebugPort.printf("Boot config portal if WiFiManager fails = %d\r\n", retval);
  return retval;
}

int  isWifiButton()
{
  return wifiButtonState;
}
