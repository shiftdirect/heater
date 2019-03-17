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
#include "../Protocol/helpers.h"
#include "../Utility/NVStorage.h"

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
  _rowSel = 0;
  _colSel = 0;
}

bool 
CPrimingScreen::show()
{
  CScreenHeader::show();
  
  CRect extents;

  int yPos = 53;
  // show next/prev menu navigation line
  switch(_rowSel) {
    case 0:
      _printMenuText(_display.xCentre(), yPos, " \021    \030Edit    \020 ", _rowSel == 0, eCentreJustify);
      break;
    case 1:
    case 2:
      _display.drawFastHLine(0, 53, 128, WHITE);
      _printMenuText(_display.xCentre(), 57, "\030\031 Sel       \033\032 Adj", false, eCentreJustify);
      break;
    case 3:
      _display.drawFastHLine(0, 53, 128, WHITE);
      if(_colSel == 2) {
        _printMenuText(_display.xCentre(), 57, "\033\030\031 Stop", false, eCentreJustify);
      }
      else {
        _printMenuText(_display.xCentre(), 57, "\032 Start     \031 Sel", false, eCentreJustify);
      }
      break;
  }

  yPos = 40;
  if(_rowSel == 1) {
    // follow user desired setting, heater info is laggy
    _printMenuText(border, yPos, "Thermostat", _colSel == 0);
    _printMenuText(_display.width()-border, yPos, "Fixed Hz", _colSel == 1, eRightJustify);
  }
  else {
    // follow actual heater settings
    int col = getHeaterInfo().isThermostat() ? 0 : 1;              
    _printInverted(border, yPos, "Thermostat", col == 0);
    _printInverted(_display.width()-border, yPos, "Fixed Hz", col == 1, eRightJustify);
  }
  yPos = 28;
  if(_rowSel == 2) {
    _printMenuText(border, yPos, "degC", _colSel == 0);
    _printMenuText(_display.width()-border, yPos, "degF", _colSel == 1, eRightJustify);
  }
  else {
    int col = NVstore.getDegFMode();              
    _printInverted(border, yPos, "degC", col == 0);
    _printInverted(_display.width()-border, yPos, "degF", col == 1, eRightJustify);
  }

  // fuel pump priming menu
  yPos = 16;
  _printMenuText(border, yPos, "Pump");
  if(_rowSel == 3) {
    _printMenuText(40, yPos, "OFF", _colSel == 1);
    if(_colSel != 2) {
      if(!getHeaterInfo().getRunState()) {                    // prevent option if heater is running
        _printMenuText(70, yPos, "ON");  // becomes Hz when actually priming 
      }
    }
    else {
      float pumpHz = getHeaterInfo().getPump_Actual();
      // recognise if heater has stopped pump, after an initial holdoff upon first starting
      long tDelta = millis() - _PrimeCheck;
      if(_PrimeCheck && tDelta > 0 && pumpHz < 0.1) {
        _stopPump();
      }
      // test if time is up, stop priming if so
      tDelta = millis() - _PrimeStop;
      if(_PrimeStop && tDelta > 0) {
        _stopPump();
      }

      if(_PrimeStop) {
        char msg[16];
        sprintf(msg, "%.1fHz", pumpHz);
        _printMenuText(70, yPos, msg, true);
      }
    }
  }

  return true;
}


bool 
CPrimingScreen::keyHandler(uint8_t event)
{
  
  if(event & keyPressed) {
    // press LEFT 
    if(event & key_Left) {
      switch(_rowSel) {
        case 0: 
          _ScreenManager.prevMenu(); 
          break;
        case 1: 
          _colSel = 0; 
          setThermostatMode(1);
          break;
        case 2: 
          _colSel = 0; 
          NVstore.setDegFMode(0);
          break;
        case 3: 
          _colSel = 1; 
          break;
        case 4: break;
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      switch(_rowSel) {
        case 0: 
          _ScreenManager.nextMenu(); 
          break;
        case 1: 
          _colSel = 1; 
          setThermostatMode(0);
          break;
        case 2: 
          _colSel = 1; 
          NVstore.setDegFMode(1);
          break;
        case 3: 
          if(!getHeaterInfo().getRunState()) 
            _colSel = 2; 
          break;
        case 4: break;
      }
    }
    // press UP
    if(event & key_Up) {
      if(hasOEMcontroller())
        _reqOEMWarning();
      else {
        _rowSel++;
        UPPERLIMIT(_rowSel, 3);
        if(_rowSel == 3)
          _colSel = 1;       // select OFF upon entry to priming menu
        if(_rowSel == 2)
          _colSel = NVstore.getDegFMode();
        if(_rowSel == 1)
          _colSel = getHeaterInfo().isThermostat() ? 0 : 1;              
      }
    }
    // press DOWN
    if(event & key_Down) {
      _rowSel--;
      LOWERLIMIT(_rowSel, 0);
      _colSel = 0;
      if(_rowSel == 1)
        _colSel = getHeaterInfo().isThermostat() ? 0 : 1;              
      if(_rowSel == 2)
        _colSel = NVstore.getDegFMode();
    }

    // check if fuel priming was selected
    if(_rowSel == 3 && _colSel == 2) {
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
  if(_colSel == 2)
    _colSel = 1;
}
