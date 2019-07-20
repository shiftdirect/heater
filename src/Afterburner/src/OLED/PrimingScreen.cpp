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

#include "PrimingScreen.h"
#include "KeyPad.h"
#include "../Utility/NVStorage.h"
#include "../Protocol/Protocol.h"
#include "fonts/Icons.h"
#include "../RTC/Clock.h"
#include "../Utility/FuelGauge.h"


///////////////////////////////////////////////////////////////////////////
//
// CPrimingScreen
//
// This screen allows the temperature control mode to be selected and
// allows pump priming
//
///////////////////////////////////////////////////////////////////////////


CPrimingScreen::CPrimingScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _initUI();
}

void
CPrimingScreen::onSelect()
{
  _stopPump();
  _initUI();
}

void 
CPrimingScreen::onExit()
{
  _stopPump();
}


void
CPrimingScreen::_initUI()
{
  _PrimeStop = 0;
  _PrimeCheck = 0;
  _paramSel = 0;
  _colSel = 0;
}

bool 
CPrimingScreen::show()
{
  CScreenHeader::show();
  
  CRect extents;

  int yPos = 53;
  // show next/prev menu navigation line
  switch(_paramSel) {
    case 0:
      _printMenuText(_display.xCentre(), yPos, " \021    \030Edit    \020 ", _paramSel == 0, eCentreJustify);
      break;
    case 1:
    case 2:
      _display.drawFastHLine(0, 53, 128, WHITE);
      _printMenuText(_display.xCentre(), 57, "\030\031 Sel      \033\032 Param", false, eCentreJustify);
      break;
    case 3:
      _display.drawFastHLine(0, 53, 128, WHITE);
      switch(_colSel) {
        case 1: 
          _printMenuText(_display.xCentre(), 57, "\033\030\031\032 Stop", false, eCentreJustify);
          break;
        case 0:
          _printMenuText(_display.xCentre(), 57, "\030 Start     \031 Sel", false, eCentreJustify);
          break;
        case -1:
          _printMenuText(_display.xCentre(), 57, "\030 Sel       Zero", false, eCentreJustify);
          break;
      }
      break;
  }

  int topline = 19;
  int midline = 29;
  int botline = 35;
//  int xPos = border;
  CRect loc;
  loc.height = ThermostatDegCIconInfo.height;
  loc.width = ThermostatDegCIconInfo.width;
  loc.xPos = border;
  if(_paramSel == 1) {
    // follow user desired setting, heater info is laggy
    if(NVstore.getUserSettings().degF)
      _drawBitmap(loc.xPos, topline-1, ThermostatDegFIconInfo);
    else
      _drawBitmap(loc.xPos, topline-1, ThermostatDegCIconInfo);
    _drawBitmap(loc.xPos, botline+1, ThermostatHzIconInfo);
    loc.yPos = (_colSel == 0) ? topline-1 : botline+1;
    _drawMenuSelection(loc, border, radius);
  }
  else {
    // follow actual heater settings
    if(getThermostatModeActive()) {
      _drawBitmap(loc.xPos, midline, NVstore.getUserSettings().degF ? ThermostatDegFIconInfo : ThermostatDegCIconInfo);
    }
    else {
      _drawBitmap(loc.xPos, midline, ThermostatHzIconInfo);
    }
  }

  loc.height = DegCIconInfo.height;
  loc.width = DegCIconInfo.width;
  loc.xPos = 35;
  if(_paramSel == 2) {
    loc.yPos = (_colSel == 0) ? topline : botline;
    _drawMenuSelection(loc, border, radius);
    _drawBitmap(loc.xPos, topline, DegCIconInfo);
    _drawBitmap(loc.xPos, botline, DegFIconInfo);
  }
  else {
    _drawBitmap(loc.xPos, midline, NVstore.getUserSettings().degF ? DegFIconInfo : DegCIconInfo);
  }

  // fuel pump priming menu
  loc.xPos = 67;
//  yPos = 16;
//  _printMenuText(border, yPos, "Pump");
  loc.width = BowserIconInfo.width;
  loc.height = BowserIconInfo.height;
  _drawBitmap(loc.xPos, midline, BowserIconInfo);
  loc.xPos = 81;
  if(_paramSel == 3) {
    _drawBitmap(loc.xPos, topline, FuelIconInfo);
    _drawBitmap(loc.xPos, botline, resetIconInfo);

    if(_colSel == -1) {
      loc.yPos = botline;
      loc.width = resetIconInfo.width;
      loc.height = resetIconInfo.height;
      _drawMenuSelection(loc, border, radius);
    }

    loc.yPos = _colSel == -1 ? botline : topline;
    loc.width = FuelIconInfo.width + StartIconInfo.width + 2;
    loc.height = FuelIconInfo.height;
    if(_colSel == 0) {
      _drawMenuSelection(loc, border, radius);
    }
    loc.xPos += FuelIconInfo.width + 2;
    if(_colSel != 1) {                                    // only show start options if not priming already  
      if(getHeaterInfo().getRunState() == 0) {            // prevent priming option if heater is running
        _drawBitmap(loc.xPos, topline+2, StartIconInfo);  // becomes Hz when actually priming 
      }
      else {
        _drawBitmap(loc.xPos, topline, CrossIconInfo);
      }
    }
    if(_colSel == 1) {
      float pumpHz = getHeaterInfo().getPump_Actual();
      // recognise if heater has stopped pump, after an initial holdoff upon first starting
      long tDelta = millis() - _PrimeCheck;
      if(_PrimeCheck && tDelta > 0 && pumpHz < 0.1) {
        _stopPump();
        _paramSel = _colSel = 0;
      }
      // test if time is up, stop priming if so
      tDelta = millis() - _PrimeStop;
      if(_PrimeStop && tDelta > 0) {
        _stopPump();
        _paramSel = _colSel = 0;
      }

      if(_PrimeStop) {
        char msg[16];
        sprintf(msg, "%.1fHz", pumpHz);
        _printMenuText(loc.xPos+1+border, topline+3, msg, true);
        _ScreenManager.bumpTimeout();  // don't allow menu timeouts whilst priming is active
      }
    }
  }
  else {
    char msg[16];
    sprintf(msg, "%.02fL", FuelGauge.Used_ml() * 0.001);
    _printMenuText(loc.xPos+1, midline+3, msg);
  }

  return true;
}


bool 
CPrimingScreen::keyHandler(uint8_t event)
{
  
  if(event & keyPressed) {
    // press LEFT 
    if(event & key_Left) {
      switch(_paramSel) {
        case 0: 
          _ScreenManager.prevMenu(); 
          break;
        default:
          _paramSel--;
          LOWERLIMIT(_paramSel, 0);
          _colSel = 0;
          switch(_paramSel) {
            case 1:
              _colSel = getThermostatModeActive() ? 0 : 1;              
              break;
            case 2:
              _colSel = NVstore.getUserSettings().degF ? 1 : 0;
              break;
          }
          break;
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      switch(_paramSel) {
        case 0: 
          _ScreenManager.nextMenu(); 
          break;
        default:
          _paramSel++;
          UPPERLIMIT(_paramSel, 3);
          switch(_paramSel) {
            case 3:
              _colSel = 0;       // select OFF upon entry to priming menu
              break;
            case 2:
              _colSel = NVstore.getUserSettings().degF ? 1 : 0;
              break;
            case 1:
              _colSel = getThermostatModeActive() ? 0 : 1;              
              break;
          }
          break;
      }
    }
    // press UP
    if(event & key_Up) {
      if(hasOEMcontroller())
        _reqOEMWarning();
      else {
        switch(_paramSel) {
          case 0:
            _paramSel = 1;
            _colSel = getThermostatModeActive() ? 0 : 1;              
            break;
          case 1: 
            _colSel++; 
            WRAPLIMITS(_colSel, 0, 1);
            setThermostatMode(_colSel == 0);
            saveNV();
            break;
          case 2: 
            _colSel++; 
            WRAPLIMITS(_colSel, 0, 1);
            setDegFMode(_colSel != 0);
            saveNV();
            break;
          case 3: 
            if(_colSel == 1)
              _colSel = 0;
            else {
              _colSel++; 
              UPPERLIMIT(_colSel, (getHeaterInfo().getRunState() == 0) ? 1 : 0);  // prevent priming if heater is running
            }
            break;
          case 4: 
            break;
        }
      }
    }
    // press DOWN
    if(event & key_Down) {
      if(_paramSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::UserSettingsLoop, CScreenManager::VersionUI);  // force return to main menu
      }
      else {
        switch(_paramSel) {
          case 0: 
            break;
          case 1: 
            _colSel--;
            WRAPLIMITS(_colSel, 0, 1);
            setThermostatMode(_colSel == 0);
            saveNV();
            break;
          case 2: 
            _colSel--;
            WRAPLIMITS(_colSel, 0, 1);
            setDegFMode(_colSel != 0);
            saveNV();
            break;
          case 3: 
            _colSel--; 
            LOWERLIMIT(_colSel, -1);
            break;
          case 4: 
            break;
        }
      }
    }
    // press UP
    if(event & key_Centre) {
      if(_paramSel == 3) {
        switch(_colSel) {
          case 0:
            if(getHeaterInfo().getRunState() == 0)
              _colSel = 1;
            break;
          case 1:
            _colSel = 0;
            break;
          case -1:
            FuelGauge.reset();
            _paramSel = _colSel = 0;
            break;
        }
      }
      else {
        _paramSel = _colSel = 0;
      }
    }

    // check if fuel priming was selected
    if(_paramSel == 3 && _colSel == 1 ) {
      reqPumpPrime(true);
      _PrimeStop = millis() + 150000;   // allow 2.5 minutes - much the same as the heater itself cuts out at
      _PrimeCheck = millis() + 3000;    // holdoff upon start before testing for heater shutting off pump
    }
    else {
      _stopPump();
    }

    _ScreenManager.reqUpdate();
  }
  return true;
}

void 
CPrimingScreen::_stopPump()
{
  reqPumpPrime(false);
  _PrimeCheck = 0;
  _PrimeStop = 0;
  if(_paramSel == 3 && _colSel == 1) {
    _colSel = 0;
  }
}
