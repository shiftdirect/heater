#ifndef __BTCWIFI_H__
#define __BTCWIFI_H__

#include <Arduino.h>
#include <WiFiManager.h>
#include <WiFi.h>

/*
  const char *failedssid;
  const char *failedpassword;
  */
  void doWiFiManager();
  void initWifi(int initpin,const char *failedssid, const char *failedpassword);
  const char* getWifiAddrStr(); 
  bool isWifiConnected();

#endif __BTCWIFI_H__
