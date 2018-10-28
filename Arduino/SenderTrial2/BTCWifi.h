#include <Arduino.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug
/*
  const char *failedssid;
  const char *failedpassword;
  */
  extern RemoteDebug Debug;
  void doWiFiManager();
  void initWifi(int initpin,const char *failedssid, const char *failedpassword);
  void DoDebug();
  void inittelnetdebug(String HOST_NAME);
  
