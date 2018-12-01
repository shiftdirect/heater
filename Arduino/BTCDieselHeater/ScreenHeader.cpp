#include <Arduino.h>
#include "ScreenHeader.h"
#include "Protocol.h"
#include "128x64OLED.h"
#include "BTCWifi.h"
#include "BluetoothAbstract.h" 
#include "clock.h"
#include "OLEDconsts.h"
#include "MiniFont.h"
#include "helpers.h"

#define MINIFONT miniFontInfo

#define X_BATT_ICON    95
#define Y_BATT_ICON     0
#define X_WIFI_ICON    22
#define Y_WIFI_ICON     0
#define X_BT_ICON      12
#define Y_BT_ICON       0

#define MINI_BATTLABEL


CScreenHeader::CScreenHeader(C128x64_OLED& disp, CScreenManager& mgr) : CScreen(disp, mgr)
{
}

void 
CScreenHeader::show()
{
  _display.clearDisplay();

  // standard header items
  //Bluetooth
  if(getBluetoothClient().isConnected())
    showBTicon();
  // WiFi
  if(isWifiConnected()) {
    showWifiIcon();
  }
  // battery
  showBatteryIcon(getHeaterInfo().getBattVoltage());

  showTime(_display);
}

void 
CScreenHeader::showBTicon()
{
  _display.drawBitmap(X_BT_ICON, Y_BT_ICON, BTicon, W_BT_ICON, H_BT_ICON, WHITE);
}

void 
CScreenHeader::showWifiIcon()
{
  _display.drawBitmap(X_WIFI_ICON, Y_WIFI_ICON, wifiIcon, W_WIFI_ICON, H_WIFI_ICON, WHITE);
#ifdef DEMO_AP_MODE
  _display.fillRect(X_WIFI_ICON + 8, Y_WIFI_ICON + 5, 10, 7, BLACK);
  CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
  _display.setCursor(X_WIFI_ICON+9, Y_WIFI_ICON+6);
  _display.print("AP");
#endif
}

void
CScreenHeader::showBatteryIcon(float voltage)
{
  _display.drawBitmap(X_BATT_ICON, Y_BATT_ICON, BatteryIcon, W_BATT_ICON, H_BATT_ICON, WHITE);
  char msg[16];
  sprintf(msg, "%.1fV", voltage);
#ifdef MINI_BATTLABEL
  CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
#endif
  _display.setCursor(X_BATT_ICON + W_BATT_ICON/2, 
                     Y_BATT_ICON + H_BATT_ICON + 2);
  _display.printCentreJustified(msg);

  // nominal 10.5 -> 13.5V bargraph
  int Capacity = (voltage - 10.7) * 4;
  if(Capacity < 0)   Capacity = 0;
  if(Capacity > 11)  Capacity = 11;
  _display.fillRect(X_BATT_ICON+2 + Capacity, Y_BATT_ICON+2, W_BATT_ICON-4-Capacity, 6, BLACK);
}


