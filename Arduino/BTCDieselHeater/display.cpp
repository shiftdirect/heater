/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2018  Ray Jones <ray@mrjones.id.au>
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
#include "Screen3.h"
#include "Screen4.h"
#include "KeyPad.h"
#include "helpers.h"
#include "clock.h"
#include "BTCConfig.h"

#define MAXIFONT tahoma_16ptFontInfo
#define MINIFONT miniFontInfo

#define X_BATT_ICON    95
#define Y_BATT_ICON     0
#define X_WIFI_ICON    22
#define Y_WIFI_ICON     0
#define X_BT_ICON      12
#define Y_BT_ICON       0

#define MINI_BATTLABEL

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

CScreenManager::CScreenManager() 
{
  _pDisplay = NULL;
  _pActiveScreen = NULL;
  for(int i = 0; i < _maxScreens; i++)
    _pScreen[i] = NULL;
  _currentScreen = 1;
}

CScreenManager::~CScreenManager()
{
  _pActiveScreen = NULL;      
  for(int i=0; i<_maxScreens; i++) {
    if(_pScreen[i]) {
      delete _pScreen[i]; _pScreen[i] = NULL;
    }
  }
  if(_pDisplay) {
    delete _pDisplay; _pDisplay = NULL;
  }
}

void 
CScreenManager::init()
{

  // 128 x 64 OLED support (Hardware SPI)
  // SH1106_SWITCHCAPVCC = generate display voltage from 3.3V internally
#if OLED_HW_SPI == 1
  SPI.setFrequency(8000000);
  _pDisplay = new C128x64_OLED(OLED_DC_pin, -1, OLED_CS_pin);
  _pDisplay->begin(SH1106_SWITCHCAPVCC, 0, false);
#else
  _pDisplay = new C128x64_OLED(OLED_SDA_pin, OLED_SCL_pin);
  _pDisplay->begin(SH1106_SWITCHCAPVCC);
#endif


  // Show initial display buffer contents on the screen --
  _pDisplay->display();

  _pScreen[0] = new CScreen1(*_pDisplay, *this);
  _pScreen[1] = new CScreen2(*_pDisplay, *this);
  _pScreen[2] = new CScreen3(*_pDisplay, *this);
  _pScreen[3] = new CScreen4(*_pDisplay, *this);

  _switchScreen();
}

void 
CScreenManager::update(const CProtocol& CtlFrame, const CProtocol& HtrFrame)
{
  if(_pActiveScreen) _pActiveScreen->show(CtlFrame, HtrFrame);
}
  
void 
CScreenManager::animate()
{
  if(_pActiveScreen) _pActiveScreen->animate();
}

void 
CScreenManager::_switchScreen()
{
  if(_currentScreen < _maxScreens)
    _pActiveScreen = _pScreen[_currentScreen]; 
  
  reqDisplayUpdate();
}

void 
CScreenManager::nextScreen()
{
  _currentScreen++;
  if(_currentScreen >= _maxScreens) {
    _currentScreen = 0;
  }
  _switchScreen();
}

void 
CScreenManager::prevScreen()
{
  _currentScreen--;
  if(_currentScreen < 0) {
    _currentScreen = _maxScreens-1;
  }
  _switchScreen();
}

void 
CScreenManager::keyHandler(uint8_t event)
{
  if(_pActiveScreen) _pActiveScreen->keyHandler(event);
}



CScreen::CScreen(C128x64_OLED& disp, CScreenManager& mgr) : 
  _display(disp), 
  _Manager(mgr) 
{
}

CScreen::~CScreen()
{
}

void
CScreen::animate()
{
  _display.display();
}

void 
CScreen::show(const CProtocol& CtlFrame, const CProtocol& HtrFrame)
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
  float voltage = HtrFrame.getVoltage_Supply() * 0.1f;
  showBatteryIcon(voltage);

  showTime(_display);
}

void 
CScreen::showBTicon()
{
  _display.drawBitmap(X_BT_ICON, Y_BT_ICON, BTicon, W_BT_ICON, H_BT_ICON, WHITE);
}

void 
CScreen::showWifiIcon()
{
  _display.drawBitmap(X_WIFI_ICON, Y_WIFI_ICON, wifiIcon, W_WIFI_ICON, H_WIFI_ICON, WHITE);
#ifdef DEMO_AP_MODE
  _display.fillRect(X_WIFI_ICON + 8, Y_WIFI_ICON + 5, 10, 7, BLACK);
  _display.setFontInfo(&MINIFONT);  // select Mini Font
  _display.setCursor(X_WIFI_ICON+9, Y_WIFI_ICON+6);
  _display.print("AP");
  _display.setFontInfo(NULL);  
#endif
}

void
CScreen::showBatteryIcon(float voltage)
{
  _display.drawBitmap(X_BATT_ICON, Y_BATT_ICON, BatteryIcon, W_BATT_ICON, H_BATT_ICON, WHITE);
#ifdef MINI_BATTLABEL
  char msg[16];
  sprintf(msg, "%.1fV", voltage);
  int xPos = X_BATT_ICON + 7 - strlen(msg) * 2;
  _display.setCursor(xPos, Y_BATT_ICON+H_BATT_ICON+2);
  _display.setFontInfo(&MINIFONT);  // select Mini Font
  _display.print(msg);
  _display.setFontInfo(NULL);  
#else
  _display.setCursor(85, 12);
  _display.setTextColor(WHITE);
  _display.print(voltage, 1);
  _display.print("V");
#endif

  // nominal 10.5 -> 13.5V bargraph
  int Capacity = (voltage - 10.7) * 4;
  if(Capacity < 0)   Capacity = 0;
  if(Capacity > 11)  Capacity = 11;
  _display.fillRect(X_BATT_ICON+2 + Capacity, Y_BATT_ICON+2, W_BATT_ICON-4-Capacity, 6, BLACK);
}

