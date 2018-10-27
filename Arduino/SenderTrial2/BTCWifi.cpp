#include "BTCWifi.h"

// select which pin will trigger the configuration portal when set to LOW

WiFiManager wm;

unsigned int  timeout   = 120; // seconds to run for
unsigned int  startTime = millis();
bool portalRunning      = false;
bool startCP            = false; // start AP and webserver if true, else start only webserver
int TRIG_PIN;

void initWifi(int initpin) 
{
  TRIG_PIN = initpin;
  pinMode(initpin, INPUT_PULLUP);
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
  if(digitalRead(TRIG_PIN) == LOW && (!portalRunning)) {
    if(startCP){
      Serial.println("Button Pressed, Starting Config Portal");
      wm.setConfigPortalBlocking(false);
      wm.startConfigPortal();
    }  
    else{
      Serial.println("Button Pressed, Starting Web Portal");
      wm.startWebPortal();
    }  
    portalRunning = true;
    startTime = millis();
  }
}


