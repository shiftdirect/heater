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
#include "KeyPad.h"
#include "helpers.h"
#include "Screen3.h"

///////////////////////////////////////////////////////////////////////////
//
// CScreen3
//
// This screen allows the temeprature control mode to be selected and
// allows pump priming
//
///////////////////////////////////////////////////////////////////////////


CScreen3::CScreen3(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _PrimeStop = 0;
  _PrimeCheck = 0;
  _rowSel = 0;
  _colSel = 0;
}


void 
CScreen3::show()
{
  CScreenHeader::show();
  
  CRect extents;
  int yPos = 52;

  // show next/prev screen navigation line
  _drawMenuText(_display.xCentre(), yPos, "<-             ->", _rowSel == 0, eCentreJustify);

  yPos = 40;
  int col = getHeaterInfo().isThermostat() ? 0 : 1;              // follow actual heater settings
  if(_rowSel == 1) {
    _drawMenuText(border, yPos, "Thermostat", col == 0);
    _drawMenuText(_display.width()-border, yPos, "Fixed Hz", col == 1, eRightJustify);
  }
  else {
    _printInverted(border, yPos, "Thermostat", col == 0);
    _printInverted(_display.width()-border, yPos, "Fixed Hz", col == 1, eRightJustify);
  }

  // fuel pump priming menu
  yPos = 28;
  _drawMenuText(border, yPos, "Prime pump");
  if(_rowSel == 2) {
    _drawMenuText(70, yPos, "OFF", _colSel == 1);
    if(_colSel != 2) {
      if(!getHeaterInfo().getRunState()) {                    // prevent option if heater is running
        _drawMenuText(100, yPos, "ON");  // becomes Hz when actually priming 
      }
    }
    else {
      float pumpHz = getHeaterInfo().getPump_Actual();
      // recognise if heater has stopped pump, after an initial holdoff upon first starting
      long tDelta = millis() - _PrimeCheck;
      if(_PrimeCheck && tDelta > 0 && pumpHz < 0.1) {
        stopPump();
      }
      // test if time is up, stop priming if so
      tDelta = millis() - _PrimeStop;
      if(_PrimeStop && tDelta > 0) {
        stopPump();
      }

      if(_PrimeStop) {
        char msg[16];
        sprintf(msg, "%.1fHz", pumpHz);
        _drawMenuText(_display.width()-border, yPos, msg, true, eRightJustify);
      }
    }
  }

  _display.display();

}


void 
CScreen3::animate()
{
  // do nothing!!
};


void 
CScreen3::keyHandler(uint8_t event)
{
  
  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      return;
    }
    // press LEFT 
    if(event & key_Left) {
      switch(_rowSel) {
        case 0: 
          _ScreenManager.prevScreen(); 
          break;
        case 1: 
          _colSel = 0; 
          setThermostatMode(1);
          break;
        case 2: 
          _colSel = 1; 
          break;
        case 3: break;
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      switch(_rowSel) {
        case 0: 
          _ScreenManager.nextScreen(); 
          break;
        case 1: 
          _colSel = 1; 
          setThermostatMode(0);
          break;
        case 2: 
          if(!getHeaterInfo().getRunState()) 
            _colSel = 2; 
          break;
        case 3: break;
      }
    }
    // press UP
    if(event & key_Up) {
      _rowSel++;
      UPPERLIMIT(_rowSel, 2);
      if(_rowSel == 2)
        _colSel = 1;       // select OFF upon entry to priming menu
    }
    // press DOWN
    if(event & key_Down) {
      _rowSel--;
      LOWERLIMIT(_rowSel, 0);
      _colSel = 0;
    }

    // check if fuel priming was selected
    if(_rowSel == 2 && _colSel == 2) {
      reqPumpPrime(true);
      _PrimeStop = millis() + 150000;   // allow 2.5 minutes - much the same as the heater itself cuts out at
      _PrimeCheck = millis() + 3000;    // holdoff upon start before testing for heater shutting off pump
    }
    else {
      stopPump();
    }

    _ScreenManager.reqUpdate();
  }
}

void 
CScreen3::stopPump()
{
  reqPumpPrime(false);
  _PrimeCheck = 0;
  _PrimeStop = 0;
  if(_colSel == 2)
    _colSel = 1;
}
