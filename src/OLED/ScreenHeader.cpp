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

#include <Arduino.h>
#include "ScreenHeader.h"
#include "../Protocol/Protocol.h"
#include "../Utility/helpers.h"
#include "../WiFi/BTCWifi.h"
#include "../Bluetooth/BluetoothAbstract.h" 
#include "../Utility/NVStorage.h"
#include "../RTC/Clock.h"
#include "fonts/Arial.h"
#include "fonts/Icons.h"
#include "fonts/MiniFont.h"
#include "../RTC/TimerManager.h"
#include "../Protocol/SmartError.h"
#include "../Utility/DataFilter.h"


#define MINIFONT miniFontInfo

#define X_BT_ICON      12
#define Y_BT_ICON       0
#define X_WIFI_ICON    22
#define Y_WIFI_ICON     0
#define X_CLOCK        50  
#define Y_CLOCK         0
#define X_TIMER_ICON   83
#define Y_TIMER_ICON    0
#define X_BATT_ICON   103
#define Y_BATT_ICON     0

// |0        |10       |20       |30       |40       |50       |60       |70       |80       |90       |100      |110      |120    
// xxxxxxxxxxxxBBBBBB    WWWWWWWWWWWWOOOOOOOOOOOO                                                                         xxxxxxxxx
// xxxxxxxxxxxxBBBBBB    WWWWWWWWWWWWOOOOOOOOOOOO                                                                         xxxxxxxxx
// xxxxxxxxxxxxBBBBBB    WWWWWWWWWWWWOOOOOOOOOOOO                                                                         xxxxxxxxx
// xxxxxxxxxxxxBBBBBB    WWWWWWWWWWWWOOOOOOOOOOOO                                                                         xxxxxxxxx
// xxxxxxxxxxxxBBBBBB    WWWWWWWWWWWWOOOOOOOOOOOO                                                                         xxxxxxxxx
// xxxxxxxxxxxxBBBBBB    WWWWWWWWWWWWW                                                                                    xxxxxxxxx
// xxxxxxxxxxxxBBBBBB    WWWWWWWWWWWWAAAAAAAAAAAA                                                                         xxxxxxxxx
// xxxxxxxxxxxxBBBBBB    WWWWWWWWWWWWAAAAAAAAAAAA                                                                         xxxxxxxxx
// xxxxxxxxxxxxBBBBBB    WWWWWWWWWWWWAAAAAAAAAAAA                                                                         xxxxxxxxx
// xxxxxxxxxxxxBBBBBB    WWWWWWWWWWWWAAAAAAAAAAAA                                                                         xxxxxxxxx
// xxxxxxxxxxxxBBBBBB                AAAAAAAAAAAA                                                                         xxxxxxxxx
// xxxxxxxxxxxx                                                                                                           xxxxxxxxx
// xxxxxxxxxxxx                                                                                                           xxxxxxxxx
// xxxxxxxxxxxx                                                                                                           xxxxxxxxx
// xxxxxxxxxxxx                                                                                                           xxxxxxxxx
// xxxxxxxxxxxx                                                                                                           xxxxxxxxx
// xxxxxxxxxxxx                                                                                                           xxxxxxxxx
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

CScreenHeader::CScreenHeader(C128x64_OLED& disp, CScreenManager& mgr) : CScreen(disp, mgr)
{
  _colon = false;
  _hdrDetail = false;
}

void 
CScreenHeader::onSelect()
{
  CScreen::onSelect();

  _batteryCount = 255;
  _animateCount = 255;
}


bool 
CScreenHeader::show(bool erase)
{
  if(erase)
    _display.clearDisplay();                  // erase everything
  else {
    _display.fillRect(0, 17, 128, 47, BLACK); // only erase below the header
    _display.fillRect(119, 0, 9, 17, BLACK); // erase top of body thermo
    _display.fillRect(0, 0, X_BT_ICON, 17, BLACK);   // erase top of ambient thermo
  }

  // standard header items
  // Bluetooth
  showBTicon();

  // WiFi icon is updated in animate()

  // Battery is updated in animate

  // clock
  showTime();

  return true;
}

void crackVer(char* msg, int newVer)
{
  int major = (int)(newVer * 0.01);
  int minor = newVer - major*100;
  float prtMajor = major * 0.1;
  sprintf(msg, "V%.1f.%d", prtMajor, minor);
}

// Animate IN/OUT arrows against the WiFi icon, according to actual web server traffic:
//   an IN (down) arrow is drawn if incoming data has been detected.
//   an OUT (up) arrow is drawn if outgoing data has been sent.
//
// Each arrow is drawn for one animation interval with a minimum of one clear interval 
// creating a clean flash on the display.
// Both arrows may appear in the same interval.
// The following is a typical sequence, relative to animation ticks, note the gap
// that always appears in the animation interval between either arrow shown:
//   
//    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
// _________^^^^^________________________________________^^^^^_________________________
// ______________vvvvv_____vvvvv_______________vvvvv_____vvvvv_____vvvvv_______________

bool 
CScreenHeader::animate()
{
  // animate timer icon, 
  // inserting an update icon if new firmware available from internet web server
  _animateCount++;
  WRAPUPPERLIMIT(_animateCount, 11, 0);
  int newVer = isUpdateAvailable(true);
  if(newVer) {
    int xPos = X_TIMER_ICON - 3;   
    int yPos = Y_TIMER_ICON;
    char msg[16];
    CTransientFont AF(_display, &miniFontInfo);  // temporarily use a mini font
    switch(_animateCount) {
      case 0:
        _display.fillRect(xPos, yPos, TimerIconInfo.width+3, TimerIconInfo.height, BLACK);
        _display.fillRect(xPos-2, yPos+12, 21, 5, BLACK);  // erase annotation
        break;        
      case 1:
        _drawBitmap(xPos+6, yPos, UpdateIconInfo);
        crackVer(msg, newVer);
        _printMenuText(xPos+19, yPos+12, msg, false, eRightJustify);
        break;
      case 2:
        _display.fillRect(xPos, yPos, TimerIconInfo.width+3, TimerIconInfo.height, BLACK);
        break;        
      case 3:
        _display.fillRect(xPos-8, yPos+12, 30, 5, BLACK);  // erase version annotation
      default:
        showTimers();
        break;
    }
  }
  else {
    showTimers();
  }

  _batteryCount++;
  WRAPUPPERLIMIT(_batteryCount, 5, 0);
  int xPos = X_BATT_ICON;   
  int yPos = Y_BATT_ICON;
  switch(_batteryCount) {
    case 0:
      // establish  battery icon flash pattern
      // > 0.5 over LVC - solid
      // < 0.5 over LVC - slow flash
      // < LVC - fast flash
      _batteryWarn = SmartError.checkVolts(FilteredSamples.FastipVolts.getValue(), FilteredSamples.FastGlowAmps.getValue(), false);
      
      showBatteryIcon(getBatteryVoltage(true));
      break;
    case 1:
      if(_batteryWarn == 2)
        _display.fillRect(xPos, yPos, BatteryIconInfo.width, BatteryIconInfo.height, BLACK);
      break;
    case 2:
      if(_batteryWarn == 2)
        showBatteryIcon(getBatteryVoltage(true));
      break;
    case 3:
      if(_batteryWarn)   // works for either < LVC, or < LVC+0.5
        _display.fillRect(xPos, yPos, BatteryIconInfo.width, BatteryIconInfo.height, BLACK);
      break;
    case 4:
      if(_batteryWarn == 2)
        showBatteryIcon(getBatteryVoltage(true));
      break;
    case 5:
      if(_batteryWarn == 2)
        _display.fillRect(xPos, yPos, BatteryIconInfo.width, BatteryIconInfo.height, BLACK);
      break;

  }

  showWifiIcon();

  return true;                 // true if we need to update the physical display
}

void 
CScreenHeader::showBTicon()
{
  if(getBluetoothClient().isConnected()) {
    _drawBitmap(X_BT_ICON, Y_BT_ICON, BluetoothIconInfo, WHITE);
  }
}

void 
CScreenHeader::showWifiIcon()
{
  if(isWifiConnected() || isWifiAP()) {   // STA or AP mode active
    _drawBitmap(X_WIFI_ICON, Y_WIFI_ICON, WifiWideIconInfo, WHITE, BLACK);  // wide icon erases annotations!

    int xPos = X_WIFI_ICON + WifiIconInfo.width + 1;  // x loaction of upload/download arrows

    // UP arrow animation
    //
    int yPos = 0;

    if(hasWebServerSpoken(true)) {
      // we have emitted data to the web client, show an UP arrow
      _UpAnnotation.holdon = 2;   // hold up arrow on for 2 cycles
      _UpAnnotation.holdoff = 8;  // hold blank for 8 cycles
    };

    if(_UpAnnotation.holdon) {
      _UpAnnotation.holdon--;
      _drawBitmap(xPos, yPos, WifiOutIconInfo);     // add upload arrow
    }
    else if(_UpAnnotation.holdoff > 0) {
      _UpAnnotation.holdoff--;     // animation of arrow is now cleared
    }
    else {
      if(NVstore.getUserSettings().enableOTA) {
        // OTA is enabled, show OTA
        // erase top right portion of wifi icon
        _display.fillRect(X_WIFI_ICON+11, Y_WIFI_ICON, 2, 6, BLACK);
        CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
        _display.setCursor(X_WIFI_ICON+12, Y_WIFI_ICON);
        _display.print("OTA");
      }
    }
    
    // low side wifi icon annotation
    if(isWifiButton()) {
      CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
      _display.setCursor(X_WIFI_ICON+12, Y_WIFI_ICON+6);
      switch(isWifiButton()) {
        case 1:  _display.print("CFG"); break;
        case 2:  _display.print("HTR"); break;
        case 3:  _display.print("ERS"); break;
      }
    }
    else {
      // DOWN arrow animation
      //
      yPos = WifiIconInfo.height - WifiInIconInfo.height + 1;
      
      if(hasWebClientSpoken(true)) {
        // we have received data from the web client, show a DOWN arrow
        _DnAnnotation.holdon = 2;  // hold down arrow on for 2 cycles
        _DnAnnotation.holdoff = 8; // hold blank for 8 cycles
      } 

      if(_DnAnnotation.holdon) {
        _DnAnnotation.holdon--;
        _drawBitmap(xPos, yPos, WifiInIconInfo, WHITE);    // add down arrow
      }
      else if(_DnAnnotation.holdoff > 0) {
        _DnAnnotation.holdoff--;    // nothing drawn after arrow, side annotation stays clear for a while
      }
      else {
        // no activity for a while now
        if(isWifiConfigPortal()) {
          // if config portal, show CFG
          CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
          _display.setCursor(X_WIFI_ICON+12, Y_WIFI_ICON+6);
          _display.print("CFG");
        }
        else if(isWifiAP()) {
          // if AP only, show AP
          CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
          _display.setCursor(X_WIFI_ICON+12, Y_WIFI_ICON+6);
          _display.print("AP");
        }
      }
    }      
  }
}

void
CScreenHeader::showBatteryIcon(float voltage)
{
  _drawBitmap(X_BATT_ICON, Y_BATT_ICON, BatteryIconInfo);
  char msg[16];
  sprintf(msg, "%.1fV", voltage);
  CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
  _display.setCursor(X_BATT_ICON + BatteryIconInfo.width/2, 
                     Y_BATT_ICON + BatteryIconInfo.height + 2);
  _display.printCentreJustified(msg);

  // nominal 10.5 -> 13.5V bargraph
  int Capacity = (voltage - 10.7) * 4;
  if(Capacity < 0)   Capacity = 0;
  if(Capacity > 11)  Capacity = 11;
  _display.fillRect(X_BATT_ICON+2 + Capacity, Y_BATT_ICON+2, BatteryIconInfo.width-4-Capacity, 6, BLACK);
}

int
CScreenHeader::showTimers()
{
  int nextTimer = CTimerManager::getNextTimer();   // timer ID and repeat flag info of next scheduled timer
  if(nextTimer) {
    int xPos = X_TIMER_ICON;   
    _drawBitmap(xPos, Y_TIMER_ICON, TimerIconInfo);
    if(nextTimer & 0x80)
      _drawBitmap(xPos-3, Y_TIMER_ICON, verticalRepeatIconInfo);
    if(_hdrDetail) {
      sTimer timerInfo;
      char msg[8];
      int activeTimer = CTimerManager::getActiveTimer();
      if(activeTimer) {
        CTimerManager::getTimer((activeTimer - 1) & 0xf, timerInfo);
        sprintf(msg, "%02d:%02d", timerInfo.stop.hour, timerInfo.stop.min);
        _drawBitmap(xPos-5, Y_TIMER_ICON+12, miniStopIconInfo, WHITE, BLACK);
      }
      else {
        CTimerManager::getTimer((nextTimer - 1) & 0xf, timerInfo);
        sprintf(msg, "%02d:%02d", timerInfo.start.hour, timerInfo.start.min);
        _drawBitmap(xPos-5, Y_TIMER_ICON+12, miniStartIconInfo, WHITE, BLACK);
      }
      CTransientFont AF(_display, &miniFontInfo);  // temporarily use a mini font
      _printMenuText(xPos-1, Y_TIMER_ICON+12, msg);
    }
    else {
      _display.fillRect(X_TIMER_ICON-5, Y_TIMER_ICON+12, 21, 5, BLACK);  // erase annotation
    }
    return 1;
  }
  _display.fillRect(X_TIMER_ICON-3, Y_TIMER_ICON, TimerIconInfo.width+3, TimerIconInfo.height, BLACK); // erase icon
  _display.fillRect(X_TIMER_ICON-5, Y_TIMER_ICON+12, 21, 5, BLACK);  // erase annotation
  return 0;
}



void 
CScreenHeader::showTime()
{
  const BTCDateTime& now = Clock.get();

  char msg[16];
  if(now.day() == 0xA5) {
    sprintf(msg, "No RTC");    
  }
  else {
    if(_colon)
      sprintf(msg, "%02d:%02d", now.hour(), now.minute());
    else
      sprintf(msg, "%02d %02d", now.hour(), now.minute());
    _colon = !_colon;
  }

  {
    CTransientFont AF(_display, &arial_8ptFontInfo);
    // determine centre position of remaining real estate
    int xPos = X_WIFI_ICON + WifiIconInfo.width + WifiInIconInfo.width;  // rhs of wifi conglomeration
    if(isWifiAP())  xPos += 4;                             // add more if an Access Point
    
    _display.fillRect(xPos - 15, Y_CLOCK, 30, arial_8ptFontInfo.nBitsPerLine, BLACK);
    _printMenuText(X_CLOCK, Y_CLOCK, msg);
  }
}

