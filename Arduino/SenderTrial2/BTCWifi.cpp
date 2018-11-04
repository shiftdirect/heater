// Should be working - Jimmy C
#include "BTCWifi.h"
// select which pin will trigger the configuration portal when set to LOW

WiFiManager wm;

unsigned int  timeout   = 120; // seconds to run for
unsigned int  startTime = millis();
bool portalRunning      = false;
bool startCP            = false; // start AP and webserver if true, else start only webserver
int TRIG_PIN;
bool res;




void initWifi(int initpin,const char *failedssid, const char *failedpassword) 
{

  
  TRIG_PIN = initpin;

      //reset settings - wipe credentials for testing
    //wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result
    wm.setConfigPortalTimeout(20);
    wm.setConfigPortalBlocking(true);

    res = wm.autoConnect(); // auto generated AP name from chipid

    if(!res) {
        Serial.println("Failed to connect");
        Serial.println("Setting up ESP as AP");
        WiFi.softAP(failedssid, failedpassword);
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
		Serial.println("Ready");
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());
}
}

void doWiFiManager(){
  // is auto timeout portal running
  if(portalRunning){
    wm.process();
    if((millis()-startTime) > (timeout*1000)){
      Serial.println("portaltimeout");
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
  if(TRIG_PIN == 1 && (!portalRunning)) {
    if(startCP){
      Serial.println("Button Pressed, Starting Config Portal");
      wm.setConfigPortalBlocking(false);
      wm.startConfigPortal();
      TRIG_PIN = 0; // reset the flag
    }  
    else{
      Serial.println("Button Pressed, Starting Web Portal");
      wm.startWebPortal();
      TRIG_PIN = 0; // reset the flag
    }  
    portalRunning = true;
    startTime = millis();
  }
}
