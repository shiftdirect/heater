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
#include "fonts/Arial.h"


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

bool
CPrimingScreen::onSelect()
{
  CScreenHeader::onSelect();
  _stopPump();
  _initUI();
  return true;
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
  _resetConfirm = false;
}

bool 
CPrimingScreen::show()
{
  CScreenHeader::show(false);
  
  _display.fillRect(0, 15, 60, 2, BLACK);
  _display.fillRect(0, 17, 100, 1, BLACK);

  CRect extents;

  if(_resetConfirm) {
    _display.writeFillRect(12, 24, 104, 30, WHITE);
    int x = _display.xCentre();
    CTransientFont AF(_display, &arial_8ptBoldFontInfo);
    _printInverted(x, 27, "Press Up to", true, eCentreJustify);
    _printInverted(x, 39, "confirm reset ", true, eCentreJustify);
    return true;
  }

  int yPos = 53;
  // show next/prev menu navigation line
  switch(_paramSel) {
    case 0:
      _printMenuText(_display.xCentre(), 53, " \021                \020 ", _paramSel == 0, eCentreJustify);
      _printMenuText(_display.xCentre(), yPos, "\030Edit", false, eCentreJustify);
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
    if(CDemandManager::isThermostat()) {
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
  // int toplinePump = topline+1;
  // int botlinePump = botline+1;
  int toplinePump = topline+2;
  int botlinePump = botline+2;
  loc.xPos = 66;
  loc.width = BowserIconInfo.width;
  loc.height = BowserIconInfo.height;
  _drawBitmap(loc.xPos, midline, BowserIconInfo);
  loc.xPos = 81;
  if(_paramSel == 3) {
    _drawBitmap(loc.xPos, toplinePump, FuelIconInfo);
    _drawBitmap(loc.xPos, botlinePump, resetIconInfo);

    if(_colSel == -1) {
      loc.yPos = botlinePump;
      loc.width = resetIconInfo.width;
      loc.height = resetIconInfo.height;
      _drawMenuSelection(loc, border, radius);
    }

    loc.yPos = _colSel == -1 ? botlinePump : toplinePump;
    loc.width = FuelIconInfo.width + StartIconInfo.width + 2;
    loc.height = FuelIconInfo.height;
    if(_colSel == 0) {
      _drawMenuSelection(loc, border, radius);
    }
    loc.xPos += FuelIconInfo.width + 2;
    if(_colSel != 1) {                                    // only show start options if not priming already  
      if(getHeaterInfo().getRunState() == 0) {            // prevent priming option if heater is running
        _drawBitmap(loc.xPos, toplinePump+2, StartIconInfo);  // becomes Hz when actually priming 
      }
      else {
        _drawBitmap(loc.xPos, toplinePump, CrossIconInfo);
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
        _printMenuText(loc.xPos+1+border, toplinePump+3, msg, true);
        _ScreenManager.bumpTimeout();  // don't allow menu timeouts whilst priming is active
      }
    }
  }
  else {
    char msg[16];
    sprintf(msg, "%.02fL", FuelGauge.Used_mL() * 0.001);
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
              _colSel = CDemandManager::isThermostat() ? 0 : 1;              
              break;
            case 2:
              _colSel = NVstore.getUserSettings().degF ? 1 : 0;
              break;
          }
          break;
      }
      _resetConfirm = false;
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
              _colSel = -1;   // select fuel gauge reset    // select OFF upon entry to priming menu
              break;
            case 2:
              _colSel = NVstore.getUserSettings().degF ? 1 : 0;
              break;
            case 1:
              _colSel = CDemandManager::isThermostat() ? 0 : 1;              
              break;
          }
          break;
      }
      _resetConfirm = false;
    }
    // press UP
    if(event & key_Up) {
      if(hasOEMcontroller())
        _reqOEMWarning();
      else {
        switch(_paramSel) {
          case 0:
            _paramSel = 1;
            _colSel = CDemandManager::isThermostat() ? 0 : 1;              
            break;
          case 1: 
            _colSel++; 
            WRAPLIMITS(_colSel, 0, 1);
            CDemandManager::setThermostatMode(_colSel == 0);
            break;
          case 2: 
            _colSel++; 
            WRAPLIMITS(_colSel, 0, 1);
            CDemandManager::setDegFMode(_colSel != 0);
            break;
          case 3: 
            if(_resetConfirm) {
              FuelGauge.reset();
              _paramSel = _colSel = 0;
            }
            else if(_colSel == 1)
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
      _resetConfirm = false;
    }
    // press DOWN
    if(event & key_Down) {
      _resetConfirm = false;
      if(_paramSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::SystemSettingsLoop, CScreenManager::SysVerUI);  // force return to main menu
      }
      else {
        switch(_paramSel) {
          case 0: 
            break;
          case 1: 
            _colSel--;
            WRAPLIMITS(_colSel, 0, 1);
            CDemandManager::setThermostatMode(_colSel == 0);
            break;
          case 2: 
            _colSel--;
            WRAPLIMITS(_colSel, 0, 1);
            CDemandManager::setDegFMode(_colSel != 0);
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
    // press CENTRE
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
            _resetConfirm = !_resetConfirm;
            break;
        }
      }
      else {
        _paramSel = _colSel = 0;
        _resetConfirm = false;
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
