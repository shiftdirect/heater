#include <Arduino.h>
#include <WiFiManager.h>
#include "RemoteDebug.h"        //https://github.com/JoaoLopesF/RemoteDebug

extern RemoteDebug Debug;

  void doWiFiManager();
  void initWifi(int initpin);
  void DoDebug();
  void inittelnetdebug(String HOST_NAME);
  

