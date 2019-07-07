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
#include "../Utility/helpers.h"
#include "../Utility/UtilClasses.h"
#include "../Utility/NVStorage.h"
#include "../WiFi/BTCWifi.h"
#include "../Utility/BoardDetect.h"
#include "fonts/Icons.h"

// nominally show the current version of firmware & hardware
// from here we can also update the firmware using web server update (requires internet STA connection)
// or factory default the stored non volatile memory contents
//
// progression is basically via the UP key:

//  _rowSel=0 - standard view, may animate upload arrow if update is available, help prompt shows 'Exit': 
//       CENTRE > exit menu
//
//  UP > _rowSel=1 - if update is available, help prompt shows 'Get Update', otherwise a silent step:
//       CENTRE > _rowSel=20 - present firmware update confirmation (UP to perform)
//           UP > update initated, reboot upon conclusion, % progress shown on display
//
//  UP > _rowSel=2 - Factory default cancel selection, help prompt shows 'Exit':
//       CENTRE > exit menu
//
//  UP > _rowSel=3 - Factory default perform selection, help prompt shows 'Apply':
//       CENTRE > _rowSel=10 - request factory default confirm 
//           UP > _rowSel=11 - defaults installed, present DONE screen, REBOOT after 5 seconds 


CVersionInfoScreen::CVersionInfoScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
}

void 
CVersionInfoScreen::onSelect()
{
  CScreenHeader::onSelect();
  _rowSel = 0;
  _animateCount = 0;
  checkFOTA();
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

    if(_rowSel < 2) {
      // standard version information screens,
      // animation of update available via animate() if firmware update is available on web server
      _printInverted(_display.xCentre(), 0, " Version Information ", true, eCentreJustify);
      
      _drawBitmap(13, 11, FirmwareIconInfo);
      _printMenuText(46, 14, getVersionStr());
      _printMenuText(46, 25, getVersionDate());

      _drawBitmap(23, 34, HardwareIconInfo);
      int PCB = getBoardRevision();
      sprintf(msg, "V%.1f", float(PCB)*0.1f);
      _printMenuText(46, 38, msg);
      if(PCB == 20) {
        _printMenuText(108, 38, "Analog", false, eCentreJustify);
        _display.drawLine(88, 42, 127, 42, WHITE);
      }

      if(_rowSel == 1 && isUpdateAvailable()) {
        // prompt 'Get Update' for new firmware available and first UP press from home
        _printMenuText(_display.xCentre(), 53, " \021  Get Update  \020 ", true, eCentreJustify);
      }
      else {
        _printMenuText(_display.xCentre(), 53, " \021     Exit     \020 ", true, eCentreJustify);
      }
    }
    else {
      if(_rowSel == 11) {  // after the saving popup has expired
        // factory default completed screen, progress to REBOOT
        const char* content[2];
        content[0] = "Factory reset";
        content[1] = "completed";
        _ScreenManager.showRebootMsg(content, 5000);
      }
      else if(_rowSel == 20) {
        // firmware update confirmation screen
        _printInverted(_display.xCentre(), 0, " Firmware update ", true, eCentreJustify);
        _printMenuText(_display.xCentre(), 35, "Press UP to", false, eCentreJustify);
        _printMenuText(_display.xCentre(), 43, "confirm download", false, eCentreJustify);
      }
      else {
        _printInverted(_display.xCentre(), 0, " Factory Default ", true, eCentreJustify);
        if(_rowSel == 10) {
          // factory default confirmation screen
          _printMenuText(_display.xCentre(), 35, "Press UP to", false, eCentreJustify);
          _printMenuText(_display.xCentre(), 43, "confirm save", false, eCentreJustify);
        }
        else {
          // factory default apply/abort screens
          _drawBitmap(10, 15, CautionIconInfo);

          _printMenuText(50, 30, "Abort", _rowSel == 2);
          _printMenuText(50, 16, "Apply", _rowSel == 3);
          if(_rowSel == 3) 
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
CVersionInfoScreen::animate()
{
  if(_rowSel <= 1 && isUpdateAvailable()) {
    // show ascending up arrow if firmware update is available on web server
    _animateCount++;
    WRAPUPPERLIMIT(_animateCount, 5, 0);
    int ypos = 11 + 15 - 7 - _animateCount;
    _display.fillRect(0, 11, 10, 21, BLACK);
    _display.drawBitmap(2, ypos, WifiOutIconInfo.pBitmap, WifiOutIconInfo.width, 7, WHITE);    // upload arrow - from web to afterburner
    _display.fillRect(1, 11, 7, 2, WHITE);   // top bar
    _drawBitmap(0, 11+16, WWWIconInfo);   // www icon
  }
  return true;
}

bool 
CVersionInfoScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // UP press
    if(event & key_Up) {
      if(_rowSel == 20) {
        isUpdateAvailable(false);   // make firmware update happen
        _rowSel = 0;
      }
      else if(_rowSel == 10) {
        wifiFactoryDefault();
        BoardRevisionReset();
        NVstore.init();
        NVstore.save();
        _showStoringMessage();
        _rowSel = 11;
      }
      else {
        _rowSel++;
        UPPERLIMIT(_rowSel, 3);
      }
    }
    // DOWN press
    if(event & key_Down) {
      if(_rowSel == 20) {      // firmware update cancel
        _rowSel = 0;
      }
      _rowSel--;
      LOWERLIMIT(_rowSel, 0);
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
      if(_rowSel == 20) {      // firmware update cancel
        _rowSel = 0;
      }
      else if(_rowSel == 3) {  // factory enable selection
        _rowSel = 10;
      }
      else if(_rowSel == 1) {  // firmware update selection
        _rowSel = 20;
      }
      else {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
      }
    }
  }

  _ScreenManager.reqUpdate();

  return true;
}

