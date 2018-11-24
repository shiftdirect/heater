#include <SPI.h>
#include "128x64OLED.h"
#include "MiniFont.h"
#include "tahoma16.h"
#include "protocol.h" 
#include "display.h"
#include "pins.h"
#include "BluetoothAbstract.h" 
#include "OLEDconsts.h"
#include "BTCWifi.h"
#include "BluetoothAbstract.h" 
#include "Screen1.h"
#include "Screen2.h"
#include "KeyPad.h"
#include "helpers.h"

#define MAXIFONT tahoma_16ptFontInfo
#define MINIFONT miniFontInfo

#define X_BATT_ICON    95
#define Y_BATT_ICON     0
#define X_WIFI_ICON    22
#define Y_WIFI_ICON     0
#define X_BT_ICON      12
#define Y_BT_ICON       0

#define MINI_BATTLABEL

void showBTicon(C128x64_OLED& display);
void showWifiIcon(C128x64_OLED& display);
void showBatteryIcon(C128x64_OLED& display, float voltage);
void switchScreen();

static int currentScreen = 0;
const int maxScreens = 2;

//
// **** NOTE: There are two very lame libaries conspiring to make life difficult ****
// A/ The ESP32 SPI.cpp library instatiates an instance of SPI, using the VSPI port (and pins)
// B/ The Adfruit_SH1106 library has hard coded "SPI" as the SPI port instance
//
// As an ESP32 has a pin multiplexer, this is very bad form.
// The design uses the defacto HSPI pins (and port), 
// You **MUST comment out the SPIClass SPI(VSPI);**  at the end of the ESP32 SPI library
// then we declare "SPI" here, which will use HSPI!!!!

// 128 x 64 OLED support
SPIClass SPI;    // default constructor opens HSPI on standard pins : MOSI=13,CLK=14,MISO=12(unused)
C128x64_OLED display(OLED_DC_pin,  -1, OLED_CS_pin);

void initOLED()
{
  currentScreen = 0;

  SPI.setFrequency(8000000);
  // SH1106_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SH1106_SWITCHCAPVCC, 0, false);

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();

  KeyPad.setCallback(keyhandlerScreen1);

}

void animateOLED()
{
  switch(currentScreen) {
    case 0:
      animateScreen1(display);
      break;
    case 1:
      animateScreen2(display);
      break;
  }
}


void updateOLED(const CProtocol& CtlFrame, const CProtocol& HtrFrame)
{
  display.clearDisplay();

  // standard header items
  //Bluetooth
  if(getBluetoothClient().isConnected())
    showBTicon(display);
  // WiFi
  if(isWifiConnected()) {
    showWifiIcon(display);
#ifdef DEMO_AP_MODE
    display.fillRect(X_WIFI_ICON + 8, Y_WIFI_ICON + 5, 10, 7, BLACK);
    display.setFontInfo(&MINIFONT);  // select Mini Font
    display.setCursor(X_WIFI_ICON+9, Y_WIFI_ICON+6);
    display.print("AP");
    display.setFontInfo(NULL);  
#endif
  }
  // battery
  float voltage = HtrFrame.getVoltage_Supply() * 0.1f;
  showBatteryIcon(display, voltage);

  switch(currentScreen) {
    case 0:
      showScreen1(display, CtlFrame, HtrFrame);
      break;
    case 1:
      showScreen2(display, CtlFrame, HtrFrame);
      break;
  }
}

void showBTicon(C128x64_OLED& display)
{
  display.drawBitmap(X_BT_ICON, Y_BT_ICON, BTicon, W_BT_ICON, H_BT_ICON, WHITE);
}

void showWifiIcon(C128x64_OLED& display)
{
  display.drawBitmap(X_WIFI_ICON, Y_WIFI_ICON, wifiIcon, W_WIFI_ICON, H_WIFI_ICON, WHITE);
}

void showBatteryIcon(C128x64_OLED& display, float voltage)
{
  display.drawBitmap(X_BATT_ICON, Y_BATT_ICON, BatteryIcon, W_BATT_ICON, H_BATT_ICON, WHITE);
#ifdef MINI_BATTLABEL
  char msg[16];
  sprintf(msg, "%.1fV", voltage);
  int xPos = X_BATT_ICON + 7 - strlen(msg) * 2;
  display.setCursor(xPos, Y_BATT_ICON+H_BATT_ICON+2);
  display.setFontInfo(&MINIFONT);  // select Mini Font
  display.print(msg);
  display.setFontInfo(NULL);  
#else
  display.setCursor(85, 12);
  display.setTextColor(WHITE);
  display.print(voltage, 1);
  display.print("V");
#endif

  // nominal 10.5 -> 13.5V bargraph
  int Capacity = (voltage - 10.7) * 4;
  if(Capacity < 0)   Capacity = 0;
  if(Capacity > 11)  Capacity = 11;
  display.fillRect(X_BATT_ICON+2 + Capacity, Y_BATT_ICON+2, W_BATT_ICON-4-Capacity, 6, BLACK);
}

void nextScreen()
{
  currentScreen++;
  if(currentScreen >= maxScreens) {
    currentScreen = 0;
  }
  switchScreen();
}

void prevScreen()
{
  currentScreen--;
  if(currentScreen < 0) {
    currentScreen = maxScreens-1;
  }
  switchScreen();
}

void switchScreen()
{
  switch(currentScreen) {
    case 0:
      KeyPad.setCallback(keyhandlerScreen1);
      break;
    case 1:
      KeyPad.setCallback(keyhandlerScreen2);
      break;
  }
  reqDisplayUpdate();
}

