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

#include "128x64OLED.h"
#include "TempSensorScreen.h"
#include "KeyPad.h"
#include "fonts/Icons.h"
#include "fonts/MiniFont.h"
#include "../Utility/TempSense.h"


CTempSensorScreen::CTempSensorScreen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _bHasBME280 = false;
  _bHasDS18B20 = false;
  _bPrimary = false;
  _Offset = 0;
  _initUI();
}

void 
CTempSensorScreen::onSelect()
{
  CScreenHeader::onSelect();
  _initUI();
  _bHasBME280 = getTempSensor().getBME280().getCount() != 0;
  _bHasDS18B20 = getTempSensor().getNumSensors() != 0;
  _bPrimary = NVstore.getHeaterTuning().BME280probe.bPrimary;
  _readNV();
}

void
CTempSensorScreen::_initUI()
{
  _rowSel = 0;
  _colSel = 0;
  _keyHold = -1;
  _scrollChar = 0;
}

bool 
CTempSensorScreen::show()
{
  char msg[32];

  _display.clearDisplay();

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

    if(_rowSel == SaveConfirm) {
      _showConfirmMessage();
    }
    else {
      if(_colSel == 0)
        _showTitle("Temp Sensor Role");
      else
        _showTitle("Temp Sensor Offset");

      // force BME280 as primary if the only sensor
      if(!_bHasDS18B20 && _bHasBME280)
        _bPrimary = true;

      strcpy(msg, "Nul");

      if(_bHasBME280) {
        if(!_bPrimary && _bHasDS18B20) {
          strcpy(msg, "Lst");
        }
        else {
          strcpy(msg, "Pri");
        }
      }
      int baseLine = 36;
      _printMenuText(border, baseLine, msg, _rowSel == 1 && _colSel == 0);

      _printMenuText(27, baseLine, "BME280");
      if(_colSel == 0) {
        float temperature;
        getTempSensor().getBME280().getTemperature(temperature, false);
        sprintf(msg, "%.01fC", temperature + _Offset);
      }
      else {
        sprintf(msg, "%+.01f", _Offset);
      }
      _printMenuText(90, baseLine, msg, _rowSel == 1 && _colSel == 1);

      if(_bHasDS18B20) {
        if(_bPrimary) {
          strcpy(msg, "Nxt");   // BME280 is primary
        }
        else {
          strcpy(msg, "Pri");   // DS18B20(s) are primary
        }
        int baseLine = 24;
        _printMenuText(border, baseLine, msg, _rowSel == 2 && _colSel == 0);

        _printMenuText(27, baseLine, "DS18B20");
        _printMenuText(90, baseLine, "Edit", _rowSel == 2 && _colSel == 1);
      }

    }
  }
  return true;
}

bool 
CTempSensorScreen::animate()
{
  if(!CPasswordScreen::_busy() && !CPasswordScreen::isPasswordBusy()) {
    if(_rowSel != SaveConfirm) {
      const char* pMsg = NULL;
      switch(_rowSel) {
        case 0:
          _printMenuText(_display.xCentre(), 52, " \021  \030Edit  Exit   \020 ", true, eCentreJustify);
          break;
        case 1:
        case 2:
        case 3:
          if(_colSel == 0)
            pMsg = "                    Hold Right to adjust probe offset.                    "; 
          else
            pMsg = "                    Hold Left to select probe's role.                    "; 
          break;
      }
      if(pMsg != NULL) {
        _display.drawFastHLine(0, 52, 128, WHITE);
        _scrollMessage(56, pMsg, _scrollChar);
      }
      return true;
    }
  }
  return false;
}


bool 
CTempSensorScreen::keyHandler(uint8_t event)
{
  if(CPasswordScreen::keyHandler(event)) {
    if(_isPasswordOK()) {
      _rowSel = 1;
      _keyHold = -1;
    }
  }

  else {
    sUserSettings us;
    if(event & keyPressed) {
      _keyHold = 0;
      // DOWN press
      if(event & key_Down) {
        if(_rowSel == SaveConfirm)
          _rowSel = 0;
        _rowSel--;
        LOWERLIMIT(_rowSel, 0);
      }
    }


    if(event & keyRepeat) {
      if(_keyHold >= 0) {
        _keyHold++;
        if(_keyHold == 2) {
          if(event & key_Up) {
          }
          if(event & key_Left) {
            _colSel = 0;
            _scrollChar = 0;
          }
          if(event & key_Right) {
            _colSel = 1;
            _scrollChar = 0;
          }
          if(event & key_Centre) {
            if(_colSel == 1)
              _Offset = 0;
          }
          _keyHold = -1;
        }
      }
    }


    if(event & keyReleased) {
      if(_keyHold == 0) {
        // UP release
        if(event & key_Up) {
          if(_rowSel == SaveConfirm) {
            _enableStoringMessage();
            _saveNV();
            NVstore.save();
            _rowSel = 0;
          }
          else {
            if(_rowSel == 0) {
              _getPassword();
              if(_isPasswordOK()) {
                _rowSel = 1;
              }
            }
            else {
              _rowSel++;
              if(_rowSel == 3)
                _ScreenManager.selectMenu(CScreenManager::BranchMenu, CScreenManager::DS18B20UI);  // force return to main menu

              UPPERLIMIT(_rowSel, 2);
            }
          }
        }
        // LEFT press
        if(event & key_Left) {
          if(_rowSel == 0)
            _ScreenManager.prevMenu();
          else 
            adjust(-1);
        }
        // RIGHT press
        if(event & key_Right) {
          if(_rowSel == 0)
            _ScreenManager.nextMenu();
          else 
            adjust(+1);
        }
        // CENTRE press
        if(event & key_Centre) {
          if(_rowSel == 0) {
            _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
          }
          else  {
            _rowSel = SaveConfirm;
          }
        }
      }
      _keyHold = -1;
    }

    _ScreenManager.reqUpdate();
  }

  return true;
}

void
CTempSensorScreen::adjust(int dir)
{
  switch(_rowSel) {
    case 1:
      if(_colSel == 0) {
        _bPrimary = !_bPrimary;
      }
      else {
        _Offset += dir * 0.1;
        BOUNDSLIMIT(_Offset, -10, +10);
      }
      break;
    case 2:
      if(_colSel == 0) {
        _bPrimary = !_bPrimary;
      }
      else {
        _Offset += dir * 0.1;
        BOUNDSLIMIT(_Offset, -10, +10);
      }
      break;
  }
}



void
CTempSensorScreen::_readNV() 
{
  const sHeaterTuning& tuning = NVstore.getHeaterTuning();
  
  _bPrimary = tuning.BME280probe.bPrimary;
  _Offset = tuning.BME280probe.offset;
}

void
CTempSensorScreen::_saveNV() 
{
  sHeaterTuning tuning = NVstore.getHeaterTuning();

  tuning.BME280probe.bPrimary = _bPrimary;
  tuning.BME280probe.offset = _Offset;

  NVstore.setHeaterTuning(tuning);
}