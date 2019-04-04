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

#include "TxManage.h"
#include "../Utility/NVStorage.h"
#include "../Protocol/helpers.h"

//#define DEBUG_THERMOSTAT


extern void DebugReportFrame(const char* hdr, const CProtocol&, const char* ftr);

// CTxManage is used to send a data frame to the blue wire
//
// As the blue wire is bidirectional, we need to only allow our transmit data
// to reach the blue wire when we actually want to send data.
// At all other times we are listening to the blue wire, receiving any async data
//
// This requires external circuitry to toggle the Tx/Rx modes.
// A "Tx Gating" signal is used.
//   when high, transmit data is sent to the blue wire
//   when low, transmit data is blocked (Hi-Z)
//
// Ideally the circuit also prevents feeding back our own Tx data into the Rx pin
// but the main software loop handles this situation by only accepting Rx data when expected.
//
//  Timing diagram
//                                 ____________________
//   Tx Gate  ____________________|                    |___________________________
//            _____________________________________________________________________
//   Tx Data                          |||||||||||||||

CTxManage::CTxManage(int TxGatePin, HardwareSerial& serial) : 
  m_BlueWireSerial(serial),
  m_TxFrame(CProtocol::CtrlMode)
{
  m_bOnReq = false;
  m_bOffReq = false;
  m_bTxPending = false;
  m_nStartTime = 0;
  m_nTxGatePin = TxGatePin;
  _rawCommand = 0;
}

void CTxManage::begin()
{
  pinMode(m_nTxGatePin, OUTPUT);
  digitalWrite(m_nTxGatePin, LOW);   // default to receive mode
}

void
CTxManage::queueOnRequest(bool set)
{
  m_bOnReq = set;  // allow cancellation via heater response frame decode
  m_bOffReq = false;
}

void 
CTxManage::queueOffRequest(bool set)
{
  m_bOffReq = set;  // allow cancellation via heater response frame decode 
  m_bOnReq = false;
}

void 
CTxManage::queueRawCommand(unsigned char val)
{
  _rawCommand = val;
}

void 
CTxManage::PrepareFrame(const CProtocol& basisFrame, bool isBTCmaster)
{
  // copy supplied frame, typically this will be the values an OEM controller delivered
  // which means we parrot that data by default.
  // When parroting, we must especially avoid ping ponging "set temperature"!
  // Otherwise we are supplied with the default params for standalone mode, which we 
  // then instil the NV parameters
  m_TxFrame = basisFrame;  

  // ALWAYS install on/off commands if required
#ifndef PROTOCOL_INVESTIGATION    
  m_TxFrame.resetCommand();   // no command upon blue wire initially, unless a request is pending
  if(_rawCommand) {
    m_TxFrame.setRawCommand(_rawCommand);
    _rawCommand = 0;
  }
  else {
    if(m_bOnReq) {
//      m_bOnReq = false;   // requires cancel via queueOnRequest(false)
      m_TxFrame.onCommand();
    }
    if(m_bOffReq) {
//      m_bOffReq = false;   // requires cancel via queueOffRequest(false)
      m_TxFrame.offCommand();
    }
  }
#endif

  // 0x78 prevents the controller showing bum information when we parrot the OEM controller
  // heater is happy either way, the OEM controller has set the max/min stuff already
  if(isBTCmaster) {
    m_TxFrame.setActiveMode();   // this allows heater to save the tuning params to EEPROM
    m_TxFrame.setFan_Min(NVstore.getFmin());
    m_TxFrame.setFan_Max(NVstore.getFmax());
    m_TxFrame.setPump_Min(NVstore.getPmin());
    m_TxFrame.setPump_Max(NVstore.getPmax());

    float tActual = getTemperatureSensor();
    uint8_t u8Temp = (uint8_t)(tActual);
    m_TxFrame.setTemperature_Actual(u8Temp);  // use current temp, for now
    m_TxFrame.setTemperature_Desired(NVstore.getDesiredTemperature());

    if(NVstore.getThermostatMode()) {
      uint8_t ThermoMode = NVstore.getThermostatMethodMode();  // get the METHOD of thermostat control
      float Window = NVstore.getThermostatMethodWindow();
      float tCurrent = getTemperatureSensor();
      float tDesired = float(NVstore.getDesiredTemperature());
      float tDelta = tCurrent - tDesired;
      float fTemp;
#ifdef DEBUG_THERMOSTAT
      DebugPort.print("Window = "); DebugPort.print(Window); DebugPort.print(" tCurrent = "); DebugPort.print(tCurrent); DebugPort.print(" tDesired = "); DebugPort.print(tDesired); DebugPort.print(" tDelta = "); DebugPort.println(tDelta); 
#endif
      Window /= 2;
      switch(ThermoMode) {

        case 0:  // conventional heater controlled thermostat mode
        case 3:  // conventional heater controlled thermostat mode
          m_TxFrame.setThermostatModeProtocol(1);  // using heater thermostat control
          u8Temp = (uint8_t)(tActual + 0.5);
          m_TxFrame.setTemperature_Actual(u8Temp);
#ifdef DEBUG_THERMOSTAT
          DebugPort.print("Conventional thermostat mode: tActual = "); DebugPort.println(u8Temp); 
#endif
          break;

        case 1:  // heater controlled thermostat mode - BUT actual temp is tweaked via a changed window
          m_TxFrame.setThermostatModeProtocol(1);  // using heater thermostat control
          u8Temp = (uint8_t)(tActual + 0.5);  // use rounded actual unless within window
          if(fabs(tDelta) < Window) {
            // hold at desired if inside window
            u8Temp = NVstore.getDesiredTemperature();   
          }
          else if(fabs(tDelta) <= 1.0) {
            // force outside if delta is <= 1 but greater than window
            u8Temp = NVstore.getDesiredTemperature() + ((tDelta > 0) ? 1 : -1); 
          }
          m_TxFrame.setTemperature_Actual(u8Temp);  
#ifdef DEBUG_THERMOSTAT
          DebugPort.print("Heater controlled windowed thermostat mode: tActual = "); DebugPort.println(u8Temp); 
#endif
          break;

        case 2:  // BTC controlled thermostat mode
          // map linear deviation within thermostat window to a Hz value,
          // Hz mode however uses the desired temperature field, somewhere between 8 - 35 for min/max
          // so create a desired "temp" according the the current hystersis
          tDelta /= Window;  // convert tDelta to fraction of window (CAUTION - may be > +-1 !)
#ifdef DEBUG_THERMOSTAT
          DebugPort.print("Linear window thermostat mode: Fraction = "); DebugPort.print(tDelta);
#endif
          fTemp = (m_TxFrame.getTemperature_Max() + m_TxFrame.getTemperature_Min()) * 0.5;  // midpoint - tDelta = 0 hinges here
          tDelta *= (m_TxFrame.getTemperature_Max() - fTemp);  // linear offset from setpoint
          fTemp -= tDelta;  // lower Hz when over temp, higher Hz when under!
          // bounds limit - recall original tDelta was NOT managed prior!
          LOWERLIMIT(fTemp, m_TxFrame.getTemperature_Min());
          UPPERLIMIT(fTemp, m_TxFrame.getTemperature_Max());
          // apply modifed desired temperature (works in conjunction with thermostatmode = 0!)
          u8Temp = (uint8_t)(fTemp + 0.5);
          m_TxFrame.setTemperature_Desired(u8Temp);
          m_TxFrame.setThermostatModeProtocol(0);  // direct heater to use Hz Mode
          m_TxFrame.setTemperature_Actual(0);      // must force actual to 0 for Hz mode
#ifdef DEBUG_THERMOSTAT
          DebugPort.print(" tDesired (pseudo Hz demand) = "); DebugPort.println(u8Temp); 
#endif
          break;
      }
    }
    else {
      m_TxFrame.setThermostatModeProtocol(0);  // not using any form of thermostat control
      m_TxFrame.setTemperature_Actual(0);      // must force actual to 0 for Hz mode
    }
//    m_TxFrame.setThermostatMode(NVstore.getThermostatMode());

    m_TxFrame.Controller.OperatingVoltage = NVstore.getSysVoltage();
    m_TxFrame.Controller.FanSensor = NVstore.getFanSensor();
    m_TxFrame.Controller.GlowDrive = NVstore.getGlowDrive();
  }
  else {
    m_TxFrame.setPassiveMode();    // this prevents the tuning parameters being saved by heater
  }

  // ensure CRC valid
  m_TxFrame.setCRC();
}

void
CTxManage::Start(unsigned long timenow)
{
  if(timenow == 0)  // avoid a black hole if millis() has wrapped to zero
    timenow++;

  m_nStartTime = timenow;
  m_bTxPending = true;
}

// generate a Tx Gate, then send the TxFrame to the Blue wire
// Note the serial data is ISR driven, we need to hold off
// for a while to let teh buffewred dat clear before closing the Tx Gate.
bool
CTxManage::CheckTx(unsigned long timenow)
{
  if(m_nStartTime) {

    long diff = timenow - m_nStartTime;

    if(diff > m_nStartDelay) {
      // begin front porch of Tx gating pulse
      digitalWrite(m_nTxGatePin, HIGH);
    }
    if(m_bTxPending && (diff > (m_nStartDelay + m_nFrontPorch))) {
      // front porch expired, perform serial transmission
      // Tx gate remains held high
      m_bTxPending = false;
      m_BlueWireSerial.write(m_TxFrame.Data, 24);  // write native binary values
    }
    if(diff > (m_nStartDelay + m_nFrameTime)) {
      // conclude Tx gating after (emperical) delay
      digitalWrite(m_nTxGatePin, LOW);
      m_nStartTime = 0;                          // cancel, we are DONE
    }
  }
  return m_nStartTime == 0;   // returns true when done
}

