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

//
// We need to identify the PCB the firmware is running upon for 2 reasons related to GPIO functions
//
// 1: Digital Inputs
//    To the outside world, the digital inputs are always treated as contact closures to ground.
//    V1.0 PCBs expose the bare ESP inputs for GPIO, they are normally pulled HIGH.
//    V2.0+ PCBs use an input conditioning transistor that inverts the sense state.
//      Inactive state for V1.0 is HIGH
//      Inactive state for V2.0+ is LOW
//
// 2: Analogue input
//    Unfortunately the pin originally chosen for the analogue input on the V2.0 PCB goes to 
//    an ADC2 channel of the ESP32.
//    It turns out NONE of the 10 ADC2 channels can be used if Wifi is enabled!
//    The remedy on V2.0 PCBs is to cut the traces leading from Digital input 1 and the Analogue input.
//    The signals are then tranposed.
//    This then presents Digital Input #1 to GPIO26, and analogue to GPIO33.
//    As GPIO33 uses an ADC1 channel no issue is present reading analogue values with wifi enabled.
//
//  Board Detection
//    Fortunately due to the use of the digital input transistors on V2.0+ PCBs, a logical
//    determination of the board configuration can be made.
//    By setting the pins as digital inputs with pull ups enabled, the logic level presented
//    can be read and thus the input signal paths can be determined.
//    Due to the input conditioning transistors, V2.0 PCBs will hold the inputs to the ESP32 
//    LOW when inactive, V1.0 PCBs will pull HIGH.
//    Likewise, the analogue input is left to float, so it will always be pulled HIGH.
//    NOTE: a 100nF capacitor exists on the analogue input so a delay is required to ensure
//    a reliable read.
//
//  Input state truth table
//                        GPIO26    GPIO33
//                        ------    ------
//                 V1.0    HIGH      HIGH
//      unmodified V2.0    HIGH      LOW
//      modified   V2.0    LOW       HIGH    
//                 V2.1    LOW       HIGH
//
//
//  ****************************************************************************************
//  This test only needs to be performed upon the very first firmware execution.
//  Once the board has been identified, the result is saved to non volatile memory 
//  If a valid value is detected, the test is bypassed.
//  This avoids future issues should the GPIO inputs be legitimately connected to 
//  extension hardware that may distort the test results when the system is repowered.
//  ****************************************************************************************
// 

#include "FuelGauge.h"
#include "NVStorage.h"
#include "DebugPort.h"

CFuelGauge::CFuelGauge()
{
  _tank_mL = 0;  
  _pumpCal = 0.02;
  record.lastsave = millis();
  record.storedval = _tank_mL;
  DebugPort.println("CFuelGauge::CFuelGauge");
}

void 
CFuelGauge::init()
{
  _pumpCal = NVstore.getHeaterTuning().pumpCal;
  float testVal;
  getStoredFuelGauge(testVal);    // RTC registers used to store this
  if(INBOUNDS(testVal, 0, 200000)) {
    DebugPort.printf("Initialising fuel gauge with %.2fmL\r\n", testVal);
    _tank_mL = testVal;
    record.storedval = _tank_mL;
  }
}


void 
CFuelGauge::Integrate(float Hz)
{
  unsigned long timenow = millis();
  long tSample = timenow - _lasttime;
  _lasttime = timenow;

  _tank_mL += Hz * tSample * 0.001 * _pumpCal;   // Hz * seconds * mL / stroke

  long tDiff = millis() - record.lastsave;
  float fuelDelta = _tank_mL - record.storedval;
  bool bStoppedSave = (Hz == 0) && (_tank_mL != record.storedval);
  if(tDiff > 600000 || fuelDelta > 1 || bStoppedSave) {         // record fuel usage every 10 minutes, or every 5mL used
    DebugPort.printf("Storing fuel gauge: %.2fmL\r\n", _tank_mL);
    storeFuelGauge(_tank_mL);            // uses RTC registers to store this
    record.lastsave = millis();
    record.storedval = _tank_mL;
  }

}

float 
CFuelGauge::Used_mL()
{
  return _tank_mL;
}
