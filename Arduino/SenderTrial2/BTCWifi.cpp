#include "BTCWifi.h"



//this is for telnet only


// select which pin will trigger the configuration portal when set to LOW

WiFiManager wm;
// Time
RemoteDebug Debug;

uint32_t mLastTime = 0;
uint32_t mTimeSeconds = 0;

// Buildin Led ON ?

boolean mLedON = false;


unsigned int  timeout   = 120; // seconds to run for
unsigned int  startTime = millis();
bool portalRunning      = false;
bool startCP            = false; // start AP and webserver if true, else start only webserver
int TRIG_PIN;
bool res;


void inittelnetdebug(String HOST_NAME)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  String hostNameWifi = HOST_NAME;
  hostNameWifi.concat(".local");

  Debug.begin(HOST_NAME);
  
  Debug.setResetCmdEnabled(true); // Enable the reset command

    Serial.println("* Arduino RemoteDebug Library");
    Serial.println("*");
    Serial.print("* WiFI connected. IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("*");
    Serial.println("* Please use the telnet client (telnet for Mac/Unix or putty and others for Windows)");
    Serial.println("*");
    Serial.println("* This sample will send messages of debug in all levels.");
    Serial.println("*");
    Serial.println("* Please try change debug level in telnet, to see how it works");
    Serial.println("*");

}

void DoDebug()
{
   // Each second

    if ((millis() - mLastTime) >= 1000) {

        // Time

        mLastTime = millis();

        mTimeSeconds++;

        // Blink the led

        mLedON = !mLedON;
        digitalWrite(LED_BUILTIN, (mLedON)?LOW:HIGH);

        // Debug the time (verbose level)

        rdebugVln("* Time: %u seconds (VERBOSE)", mTimeSeconds);

        if (mTimeSeconds % 5 == 0) { // Each 5 seconds

          Debug.handle();
        }
    }
}

void initWifi(int initpin) 
{

  
  TRIG_PIN = initpin;

      //reset settings - wipe credentials for testing
    //wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    res = wm.autoConnect(); // auto generated AP name from chipid
    
    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
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


