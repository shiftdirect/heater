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


#include "FuelGauge.h"
#include "NVStorage.h"
#include "DebugPort.h"
#include "../RTC/RTCStore.h"

CFuelGauge::CFuelGauge()
{
  _pumpStrokes = 0;  
  _pumpCal = 0.02;
  _lastStoredVal = _pumpStrokes;
  DebugPort.println("CFuelGauge::CFuelGauge");
}

void 
CFuelGauge::init(float fuelUsed)
{
  _pumpCal = NVstore.getHeaterTuning().pumpCal;

  _pumpStrokes = fuelUsed;
  DebugPort.printf("Initialising fuel gauge with %.2f strokes\r\n", _pumpStrokes);
  _lastStoredVal = _pumpStrokes;
}

void 
CFuelGauge::reset()
{
  _pumpStrokes = 0;
  _lastStoredVal = _pumpStrokes;
  RTC_Store.setFuelGauge(_pumpStrokes);      // uses RTC registers to store this
}

void 
CFuelGauge::Integrate(float Hz)
{
  unsigned long timenow = millis();
  long tSample = timenow - _lasttime;
  _lasttime = timenow;

  _pumpStrokes += Hz * tSample * 0.001;   // Hz * seconds 

  float fuelDelta = _pumpStrokes - _lastStoredVal;
  bool bStoppedSave = (Hz == 0) && (_pumpStrokes != _lastStoredVal);
  if(fuelDelta > 10 || bStoppedSave) {         // record fuel usage every 10 minutes, or every 10 strokes
//    DebugPort.printf("Storing fuel gauge: %.2f strokes\r\n", _pumpStrokes);
    RTC_Store.setFuelGauge(_pumpStrokes);            // uses RTC registers to store this
    _lastStoredVal = _pumpStrokes;
  }

}

float 
CFuelGauge::Used_mL()
{
  return _pumpStrokes * _pumpCal;   // strokes * millilitre / stroke
}

float 
CFuelGauge::Used_strokes()
{
  return _pumpStrokes;   
}
