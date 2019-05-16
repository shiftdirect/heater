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

#include "128x64OLED.h"
#include "VersionInfoScreen.h"
#include "KeyPad.h"
#include "../Protocol/helpers.h"
#include "../Utility/UtilClasses.h"
#include "../Utility/NVStorage.h"
#include "../Utility/GPIO.h"
#include "../WiFi/BTCWifi.h"
#include "../Utility/BoardDetect.h"
#include "fonts/Icons.h"



CVersionInfoScreen::CVersionInfoScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
}

void 
CVersionInfoScreen::onSelect()
{
  CScreenHeader::onSelect();
  _factoryDefaultEn = 0;
}

void
CVersionInfoScreen::_initUI()
{
}

bool 
CVersionInfoScreen::show()
{
  char msg[16];

  _display.clearDisplay();

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

    if(_factoryDefaultEn < 2) {
      _printInverted(_display.xCentre(), 0, " Version Information ", true, eCentreJustify);
      
      _display.drawBitmap(10, 11, firmwareIcon, firmwareWidth, firmwareHeight, WHITE);
      _printMenuText(43, 14, getVersionStr());
      _printMenuText(43, 25, getVersionDate());

      _display.drawBitmap(20, 34, hardwareIcon, hardwareWidth, hardwareHeight, WHITE);
      int PCB = getBoardRevision();
      sprintf(msg, "V%.1f", float(PCB)*0.1f);
      _printMenuText(43, 38, msg);
      if(PCB == 20) {
        _printMenuText(108, 38, "Analog", false, eCentreJustify);
        _display.drawLine(88, 42, 127, 42, WHITE);
      }
      _printMenuText(_display.xCentre(), 53, " \021     Exit     \020 ", true, eCentreJustify);
    }
    else {
      if(_factoryDefaultEn == 11) {  // after the saving popup has expired
        const char* content[2];
        content[0] = "Factory reset";
        content[1] = "completed";
        _ScreenManager.showRebootMsg(content, 5000);
      }
      else {
        _printInverted(_display.xCentre(), 0, " Factory Default ", true, eCentreJustify);
        if(_factoryDefaultEn == 10) {
          _printMenuText(_display.xCentre(), 35, "Press UP to", false, eCentreJustify);
          _printMenuText(_display.xCentre(), 43, "confirm save", false, eCentreJustify);
        }
        else {
        
          _display.drawBitmap(10, 15, cautionIcon, cautionWidth, cautionHeight, WHITE);

          _printMenuText(50, 30, "Abort", _factoryDefaultEn == 2);
          _printMenuText(50, 16, "Apply", _factoryDefaultEn == 3);
          if(_factoryDefaultEn == 3) 
            _printMenuText(_display.xCentre(), 53, " \021    Apply     \020 ", true, eCentreJustify);
          else
            _printMenuText(_display.xCentre(), 53, " \021     Exit     \020 ", true, eCentreJustify);
        }
      }
    }
  }

  return true;
}


bool 
CVersionInfoScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // UP press
    if(event & key_Up) {
      if(_factoryDefaultEn == 10) {
        wifiFactoryDefault();
        BoardRevisionReset();
        NVstore.init();
        NVstore.save();
        _showStoringMessage();
        _factoryDefaultEn = 11;
      }
      else {
        _factoryDefaultEn++;
        UPPERLIMIT(_factoryDefaultEn, 3);
      }
    }
    // DOWN press
    if(event & key_Down) {
      _factoryDefaultEn--;
      LOWERLIMIT(_factoryDefaultEn, 0);
    }
    // LEFT press
    if(event & key_Left) {
      _ScreenManager.prevMenu();
    }
    // RIGHT press
    if(event & key_Right) {
      _ScreenManager.nextMenu();
    }
    // CENTRE press
    if(event & key_Centre) {
      if(_factoryDefaultEn == 3) {
        _factoryDefaultEn = 10;
      }
      else {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
      }
    }
  }

  _ScreenManager.reqUpdate();

  return true;
}

