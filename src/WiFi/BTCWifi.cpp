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
#include "../../lib/WiFiManager-dev/WiFiManager.h"

// function to control the behaviour upon reboot if no wifi manager credentials exist
// or connection fails
void prepBootIntoConfigPortal(bool state);
bool shouldBootIntoConfigPortal();
void saveParamsCallback();
void APstartedCallback(WiFiManager*);

WiFiManager wm;

bool isPortalAP         = false;   // true if config portal is running
bool isSTA              = false;   // true if connected to an access point
unsigned restartServer = 0;        // set to time of portal reconfig - will cause reboot a while later
char MACstr[2][20];                // MACstr[0] STA, MACstr[1] = AP
int wifiButtonState = 0;
unsigned long WifiReconnectHoldoff = 0;

extern CScreenManager ScreenManager;


bool initWifi() 
{
  sCredentials creds = NVstore.getCredentials();  // local Soft AP credentials
  
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

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

  wm.setHostname(creds.APSSID);  // define Soft AP name
  wm.setDebugOutput(true);
  wm.setConfigPortalTimeout(20);
  wm.setConfigPortalBlocking(false);
  wm.setWiFiAutoReconnect(true);
  wm.setSaveParamsCallback(saveParamsCallback);  // ensure our webserver gets awoken when IP config changes to STA
  wm.setAPCallback(APstartedCallback);
  wm.setEnableConfigPortal(shouldBootIntoConfigPortal());
//REMOVED - UNSTABLE WHETHER WE GET 192.168.4.1 or 192.168.100.1 ????  
// REMOVED    wm.setAPStaticIPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255,255,255,0)); 
 
  bool res = wm.autoConnect(creds.APSSID, creds.APpassword); // User definable AP name & password
  DebugPort.printf("WifiMode after autoConnect = "); DebugPort.println(WiFi.getMode());

  int chnl = 1;
  bool retval = false;
  bool startAP = false;
  if(!res) {
    // failed STA mode
    DebugPort.println("WiFimanager failed STA connection. Setting up AP...");
    WiFi.disconnect();  // apparently needed for AP only OTA to reboot properly!!!
    startAP = true;
  }    
  else {
    // runs through here if STA connected OK
    // if you get here you have connected to the WiFi    
    isSTA = true;
    DebugPort.println("WiFiManager connected in STA mode OK");
    DebugPort.printf("  STA IP address: %s\r\n", getWifiSTAAddrStr());
    // must use same radio channel as STA to go to STA+AP, otherwise we drop the STA!
    chnl = WiFi.channel();  
    retval = true;
  }
  if(isSTA) {
    if(NVstore.getUserSettings().wifiMode & 0x02) {  // Check for STA only mode
      DebugPort.println("  Using STA only mode."); 
    }
    else {
      DebugPort.println("Now promoting to STA+AP mode..."); 
      startAP = true;
    }
  }
#if USE_AP_ALWAYS == 1
  startAP = true;
  DebugPort.println("Forcing AP mode on!"); 
#endif

  // WiFi.softAP(failedssid, failedpassword, chnl);
  // WiFi.softAP(creds.SSID, creds.APpassword, chnl);
  if(startAP) {
    //  for STA+AP mode we *must* use the same RF channel as STA
    DebugPort.println("Starting AP mode");
    WiFi.softAP(creds.APSSID, creds.APpassword, chnl);
    // DebugPort.printf("  AP SSID: %s\r\n", WiFi.softAPgetHostname());
    // DebugPort.printf("  AP IP address: %s\r\n", getWifiAPAddrStr());
    DebugPort.printf("WifiMode after initWifi = %d\r\n", WiFi.getMode());
  }

  // even though we may have started in STA mode - start the config portal if demanded via the NV flag
  if(shouldBootIntoConfigPortal()) {
    DebugPort.println("Manually starting web portal");
    wm.startWebPortal();
    isPortalAP = true;                   // we started portal, we have to flag it!
  }

  // WiFi.setTxPower(WIFI_POWER_MINUS_1dBm);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  return retval;
}

// call from main sketch loop()
void doWiFiManager()
{
  if(NVstore.getUserSettings().wifiMode) {
    wm.process();

    if(WiFi.status() != WL_CONNECTED) {
      if(isSTA) {
        DebugPort.println("STA lost");
        // ensure inbuilt AP is started if STA is down
        sCredentials creds = NVstore.getCredentials();  // local AP credentials
        int chnl = 1;
        WiFi.softAP(creds.APSSID, creds.APpassword, chnl);  // this will intrinsically enable AP
      }
      isSTA = false;
      if(WifiReconnectHoldoff) {
        long tDelta = millis() - WifiReconnectHoldoff;
        if(tDelta >= 0) {
          WifiReconnectHoldoff = 0;  
          WiFi.disconnect();
          WiFi.mode(WIFI_AP_STA);
          wifi_config_t conf;
          esp_wifi_get_config(WIFI_IF_STA, &conf);
          if(strlen((char*)conf.sta.ssid))
            WiFi.begin((char*)conf.sta.ssid, (char*)conf.sta.password);
        }
      }
      else {
        WifiReconnectHoldoff = millis() + 15000;
      }
    }
    else {
      if(!isSTA) { 
        // initial regain of STA connection detected
        DebugPort.println("STA established");
        if(NVstore.getUserSettings().wifiMode & 0x02) {
          WiFi.enableAP(false);   // kill inbuilt AP if setup for STA only
        }
        else {
          // ensure inbuilt AP is set to same channel as STA
          int chnl = WiFi.channel();  
          sCredentials creds = NVstore.getCredentials();  // local AP credentials
          WiFi.softAP(creds.APSSID, creds.APpassword, chnl);  // intrinsically enables Soft AP
        }
      }
      isSTA = true;
      WifiReconnectHoldoff = 0;
    }

  }
}

void wifiDisable(long rebootDelay) 
{
  sUserSettings settings = NVstore.getUserSettings();
  settings.wifiMode = 0;
  NVstore.setUserSettings(settings);
  NVstore.save(); 
  NVstore.doSave();   // ensure NV storage

  DebugPort.println("*** Disabling WiFi ***");

  restartServer = (millis() + rebootDelay) | 1;      // prepare to reboot in the future - ensure non zero!

  const char* content[2];
  content[0] = "WiFi Mode \032 DISABLED";
  content[1] = "";
  ScreenManager.showRebootMsg(content, rebootDelay);
}

void wifiEnterConfigPortal(bool state, bool erase, long rebootDelay, bool STAonly) 
{
	wm.disconnect();

  sUserSettings settings = NVstore.getUserSettings();
  settings.wifiMode = STAonly ? 0x02 : 0x01;
  NVstore.setUserSettings(settings);
  NVstore.save(); 
  NVstore.doSave();   // ensure NV storage

  prepBootIntoConfigPortal(state);  

  const char* content[2];

  if(isWifiSTA() && !erase) {
    if(STAonly)
      content[0] = "WiFi Mode \032 STA only";
    else
      content[0] = "WiFi Mode \032 STA+AP";
  }
  else {
    content[0] = "WiFi Mode \032 AP only";
  }

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
  sUserSettings settings = NVstore.getUserSettings();
  settings.wifiMode = 1;
  NVstore.setUserSettings(settings);
  NVstore.save(); 
  NVstore.doSave();   // ensure NV storage
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
  if(NVstore.getUserSettings().wifiMode) { // check that wifi should be active
    IPAddress IPaddr = WiFi.softAPIP();   // use stepping stone - function returns an automatic stack var - LAME!
    static char APIPaddr[16];
    sprintf(APIPaddr, "%d.%d.%d.%d", IPaddr[0], IPaddr[1], IPaddr[2], IPaddr[3]);
    return APIPaddr;
  }
  else 
    return "";
}
  
const char* getWifiSTAAddrStr()
{ 
  if(NVstore.getUserSettings().wifiMode) { // check that wifi should be active
    IPAddress IPaddr = WiFi.localIP();    // use stepping stone - function returns an automatic stack var - LAME!
    static char STAIPaddr[16];
    sprintf(STAIPaddr, "%d.%d.%d.%d", IPaddr[0], IPaddr[1], IPaddr[2], IPaddr[3]);
    return STAIPaddr;
  }
  else 
    return "";
}

const char* getWifiGatewayAddrStr()
{ 
  if(NVstore.getUserSettings().wifiMode) {  // check that wifi should be active
    IPAddress IPaddr = WiFi.gatewayIP();   // use stepping stone - function returns an automatic stack var - LAME!
    static char GWIPaddr[16];
    sprintf(GWIPaddr, "%d.%d.%d.%d", IPaddr[0], IPaddr[1], IPaddr[2], IPaddr[3]);
    return GWIPaddr;
  }
  else 
    return "";
}

int8_t getWifiRSSI()
{
  if(NVstore.getUserSettings().wifiMode) {  // check that wifi should be active
    static unsigned long updateRSSI = millis() + 2500;
    static int8_t RSSI = 0;
    long tDelta = millis() - updateRSSI;
    if(tDelta > 0) {
      updateRSSI = millis() + 2500;
      RSSI = WiFi.RSSI();
    }
    return RSSI;
  }
  else {
    return 0;
  }
}

  

  
const char* getWifiAPMACStr()
{ 
  return MACstr[1]; 
}
  
const char* getWifiSTAMACStr()
{ 
  return MACstr[0]; 
}

String getSTASSID()
{
  if(NVstore.getUserSettings().wifiMode) {  // check that wifi should be active
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    return String(reinterpret_cast<const char*>(conf.sta.ssid));
  }
  else 
    return "";
}

bool isWifiSTAConnected()
{
  if(NVstore.getUserSettings().wifiMode) {  // non zero => enabled wifi, maybe AP only or STA+AP or STA only
    return WiFi.status() == WL_CONNECTED;
  }

  return false;
}

bool isWifiAPonly()
{
  if(NVstore.getUserSettings().wifiMode) { // non zero => enabled wifi, maybe AP only or STA+AP or STA only
    int mode = WiFi.getMode();
    return !isSTA && ((mode & WIFI_MODE_AP) != 0);   
  }

  return false;
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
