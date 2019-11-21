/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2019  Ray Jones <ray@mrjones.id.au>
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

#include "MQTTScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "../WiFi/ABMqtt.h"
#include "../WiFi/BTCWifi.h"
#include "../Utility/NVStorage.h"
#include "fonts/Arial.h"

///////////////////////////////////////////////////////////////////////////
//
// CMQTTScreen
//
// This screen presents sundry information
// eg WiFi status
//
///////////////////////////////////////////////////////////////////////////

#define STA_HOLD_TIME 10

static const int LIMIT_AWAY = 0;
static const int LIMIT_LEFT = 1;
static const int LIMIT_RIGHT = 2;

CMQTTScreen::CMQTTScreen(C128x64_OLED& display, CScreenManager& mgr) : CUIEditScreen(display, mgr) 
{
  _initUI();
}

void
CMQTTScreen::onSelect()
{
  CScreen::onSelect();
  _initUI();
}


bool 
CMQTTScreen::show()
{
  CScreen::show();
  
  _display.clearDisplay();
  _showTitle("MQTT status");

  int yPos = 18;

  if(NVstore.getMQTTinfo().enabled) {
    if(isWifiConnected()) {
      if(isMQTTconnected())
        _printMenuText(border, yPos, "CONNECTED");  
      else 
        _printInverted(border, yPos, " DISCONNECTED ", true);  
    }
    else {
      _printInverted(border, yPos, " NO STA NETWORK ", true);  
    }
  }
  else {
    _printMenuText(border, yPos, "DISABLED");  
  }
  char msg[40];
  sprintf(msg, "QoS:%d", NVstore.getMQTTinfo().qos);
  _printMenuText(_display.width(), yPos, msg, false, eRightJustify);  
  yPos += _display.textHeight() + 2;
  sprintf(msg, "%s:%d", NVstore.getMQTTinfo().host, NVstore.getMQTTinfo().port);
  _printMenuText(border, yPos, msg);  
  yPos += _display.textHeight() + 2;
  sprintf(msg, "%s/%s", NVstore.getMQTTinfo().username, NVstore.getMQTTinfo().password);
  _printMenuText(border, yPos, msg);  
  yPos += _display.textHeight() + 2;
  sprintf(msg, "%s", NVstore.getMQTTinfo().topicPrefix);
  _printMenuText(border, yPos, msg);  

  return true;
}

bool
CMQTTScreen::animate()
{
  return false;
}

bool 
CMQTTScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    _repeatCount = 0;
    // press LEFT 
    if(event & key_Left) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.prevMenu(); 
          break;
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      switch(_rowSel) {
        case 0:
          _ScreenManager.nextMenu(); 
          break;
      }
    }
    // press UP
    if(event & key_Up) {
//      _rowSel++;
      UPPERLIMIT(_rowSel, 2);
    }
    // press DOWN
    if(event & key_Down) {
//      _rowSel--;
      LOWERLIMIT(_rowSel, 0);
    }
    _ScreenManager.reqUpdate();
  }

  if(event & keyRepeat) {    // track key hold time
    if(event & key_Centre) {
      _repeatCount++;
    }
  }

  if(event & keyReleased) {
    if(event & key_Centre) {
      if(_rowSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
      }
      if(_rowSel == 1) {

      }
      if(_rowSel == 2) {
      }
    }
    _repeatCount = 0;
  }
  return true;
}

