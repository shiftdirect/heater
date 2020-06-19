/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2020  Ray Jones <ray@mrjones.id.au>
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

#include <Arduino.h>
#include "AltControllerTask.h"
#include "AltController.h"
#include "../cfg/BTCConfig.h"
#include "../cfg/pins.h"
#include "Protocol.h"
#include "TxManage.h"
// #include "SmartError.h"
#include "../Utility/UtilClasses.h"
#include "../Utility/DataFilter.h"
#include "../Utility/FuelGauge.h"
#include "../Utility/HourMeter.h"
#include "../Utility/macros.h"
#include "../Utility/DemandManager.h"

#define MASK_FAN      0x0800
#define MASK_PUMP     0x0400
#define MASK_PLATEAU  0x0200
#define MASK_GLOW     0x0100
#define MASK_STOPPING 0x0080
#define MASK_ON       0x0040

void matchDemand(int desired);

/* known command values
  0xA0 - status/ Rx code 5
  0xA1 - switch to thermo mode 'h7' - get 0x5xxx messages in thermo... 
  0xA2 - power on/off
  0xA3 - increase demand (h1-h6) (also used to leave thermo mode ('h7' -> 'h1'))
  0xA4 - decrease demand (h1-h6)
  0xA5 - ??
  0xA6 - Primary get status
  0xA7 - status (alternates status and Rx code 9... when stopped)
  0xA8 - returns status
  0xA9 - returns status
  0xAA - enter pump edit mode ->0x3xxx returned / running minimum power (alternates status and Rx code 3...)
  0xAB - get voltage
  0xAC - prime
  0xAD - ?
  0xAE - toggle plateau
  0xAF - get body temp

  0x70 - maintain power (thermostat)
  0x71 - increase power (thermostat)
  0x72 - decrease power (thermostat)

  EDIT mode:
    when you send 0xAA and 0x3yxx is returned, you can step thru 'y' by using 0xAD
      ie 0x30xx, 0x31xx, 0x32xx, 0x33xx, 0x34xx, 0x35xx - Those seem to be the 6 set points.
    Using 0xA3 and 0xA4 you can increase or decrease 'xx' of the selected 'y' field
    0xA2 then saves the new setting (but stops the heater if running!)

    Have not found a fan speed anywhere...

  */

int 
sAltHeaterData::errState()
{
/* convert fault codes from the ECU to classic codes used by more capable unit
  A02 -> E-01 Power/voltage problem  Normal range: 24V (18-32V), 12V (9-16V)
  A03 -> E-04 Oil pump failure
  A04 -> E-03 Ignition plug failure
  A05 -> E-06 Fan failure
  A06 -> E-09 Body Sensor failure
  A07 -> E-10 Unsuccessful startup
  A08 -> E-05 High temperature alarm (inlet air > 50 °C; chassis > 200 ° C)
  A09 -> E-08 Flameout alarm

  Note that classic errors must +1 of that displayed
*/
  switch(Error) {
    case 2: return 2;   // low volts -> E=01
    case 3: return 5;   // pump -> E-04
    case 4: return 4;   // glow plug -> E-03
    case 5: return 7;   // fan -> E-06
    case 6: return 10;  // sensor -> E-09
    case 7: return 11;  // start fail -> E-10
    case 8: return 6;   // overtemp -> E-05
    case 9: return 9;   // flameout -> E-08
    default: return 0;
  }
}

int 
sAltHeaterData::runState()
{
  if(HeaterOn == 0) {
    return 0;
  }
  else {
    if(Stopping) {
      return 7;
    }
    else {
      if(GlowOn) {
        if(!PumpOn) 
          return 9;   // heating glow plug
        else      
          return 2;   // igniting
      }
      else {
        return 5;
      }
    }
  }
}

float
sAltHeaterData::getPumpRate()
{
  if(PumpOn) {
    if(INBOUNDS(Demand, 0, 5) && pumpRate[Demand] > 0) 
      return pumpRate[Demand] * 0.1;
    else 
      return -1;
  }
  else {
    return 0;
  }
}

float
sAltHeaterData::getDesiredPumpRate(int Idx)
{
  if(INBOUNDS(Idx, 0, 5) && pumpRate[Idx] > 0) 
    return pumpRate[Idx] * 0.1;
  else 
    return -1;
}

void 
sAltHeaterData::decodeThermoDemand(int rxData)
{
  Demand = rxData & 0x07;
  thermoMode = true;
}

void 
sAltHeaterData::decodeError(int rxData)
{
  Error = rxData & 0x0f;
}

void 
sAltHeaterData::decodePumpRate(int rxData)
{
  int index = (rxData & 0xF00) >> 8;
  int rate = rxData & 0xFF;

  if(INBOUNDS(index, 0, 5)) {
    pumpRate[index] = rate;
  }
}

void
sAltHeaterData::decodeVolts(int rxData)
{
  Volts = rxData & 0x1f;
  FilteredSamples.ipVolts.update(Volts);
  FilteredSamples.FastipVolts.update(Volts);
}

void 
sAltHeaterData::decodeBodyT(int rxData)
{
  BodyT = (rxData & 0xFFF);
}


void 
sAltHeaterData::decodeStatus(int rxData)
{
  if(rxData & MASK_ON) {
    HeaterOn = true;
    FanOn = rxData & MASK_FAN ? true : false;
    GlowOn = rxData & MASK_GLOW ? true : false;
    PumpOn = rxData & MASK_PUMP ? true : false;
    Plateau = rxData & MASK_PLATEAU ? true : false;
    Stopping = rxData & MASK_STOPPING ? true : false;
    int dmd = rxData & 0x07;
    if(dmd != 6) {
      Demand = dmd;
      thermoMode = false;
    }
    else {
      thermoMode = true;    
    }
  }
  else {
    HeaterOn = false;
    FanOn = false;
    GlowOn = false;
    PumpOn = false;
    Plateau = false;
    Stopping = false;
    Error = 0;
    Volts = rxData & 0x1f;
    thermoMode = false;

    FilteredSamples.ipVolts.update(Volts);
    FilteredSamples.FastipVolts.update(Volts);
  }
}

sAltHeaterData::sAltHeaterData()
{
  init();
}

void
sAltHeaterData::init()
{
  thermoMode = false;
  Active = 0;
  HeaterOn = false;
  Stopping = false;
  GlowOn = false;
  FanOn = false;
  PumpOn = false;
  Plateau = false;
  Demand = 0;
  BodyT = -1;
  Volts = 0;
  Error = 0;
  for(int i=0; i<6; i++)  pumpRate[i] = -1;
}

float
sAltHeaterData::getBodyTemp()
{
  return BodyT;  // TODO: map to real world somehow
}

void 
sAltHeaterData::report()
{
  char msg[80];
  sprintf(msg, "On:%d Fan:%d Glow:%d Pump:%d Plateau:%d Demand:%d Battery:%d BodyT:%d\r\n",
                HeaterOn, 
                FanOn, 
                GlowOn, 
                PumpOn, 
                Plateau, 
                Demand, 
                Volts, 
                BodyT);
  Serial.print(msg);
}

void  
CAltCommsTask::_decode(int rxData)
{
  unsigned int ID = rxData >> 12;
  switch(ID) {
    case 0x0: break;
    case 0x1: break;
    case 0x2: break;
    case 0x3: 
      _htrData.decodePumpRate(rxData); 
      break;       // 0x3yxx   y = H1(0) - h6(5) xx = Hz x10
    case 0x4: 
      _htrData.decodeBodyT(rxData); 
      break;
    case 0x5: 
      _htrData.decodeThermoDemand(rxData); 
      break;   // heat bars on OEM LCD in thermo mode 0=1 bar
    case 0x6: 
      _htrData.decodeStatus(rxData); 
      break;
    case 0x7: break; 
    case 0x8: 
      _htrData.decodeError(rxData); 
      break;
    case 0x9: break; // 0x9009 in response to A7 when stopped...
    case 0xa: 
      _htrData.decodeVolts(rxData); 
      break;
    case 0xb: break;
    case 0xc: break;
    case 0xd: break;
    case 0xe: break;
    case 0xf: break;
  }
  _htrData.Active = millis() + 5000;
}

void 
CAltCommsTask::reportDecode()
{
  _htrData.report();
}

float
CAltCommsTask::getFanRPM()
{
  return _htrData.FanOn ? -1 : 0;
}

float 
CAltCommsTask::getActualPumpRate()
{
  return _htrData.getPumpRate();
}

float 
CAltCommsTask::getDesiredPumpRate(int idx)
{
  return _htrData.getDesiredPumpRate(idx);
}

float 
CAltCommsTask::getGlow()
{
  return _htrData.GlowOn ? -1 : 0;
}

float 
CAltCommsTask::getBodyTemp()
{
  return _htrData.getBodyTemp();
}

int
CAltCommsTask::getRunState()
{
  return _htrData.runState();
}

int
CAltCommsTask::getErrState()
{
  return _htrData.errState();
}

void 
CAltCommsTask::reqPower()
{
  putTxQueue(0xA2);
}

void 
CAltCommsTask::_reqStatus()
{
  putTxQueue(0xA6);
}

void 
CAltCommsTask::_reqVolts()
{
  putTxQueue(0xAB);
}


void 
CAltCommsTask::_reqBodyT()
{
  putTxQueue(0xAF);
}

void
CAltCommsTask::_reqDemand(int dir) 
{
  if(_htrData.thermoMode)
    putTxQueue(0xA1);   // switch from thermo mode

  if(dir == 0)
    putTxQueue(0xA6);
  else if(dir > 0)
    putTxQueue(0xA3);
  else 
    putTxQueue(0xA4);
}

void 
CAltCommsTask::_reqThermo(int delta)
{
  if(!_htrData.thermoMode)
    putTxQueue(0xA1);   // switch to thermo mode

  if(delta == 0)
    putTxQueue(0x70);
  else if(delta > 0)
    putTxQueue(0x71);
  else 
    putTxQueue(0x72);
}

void checkAltTxEvents()
{
}

void 
CAltCommsTask::manage()
{
  static bool flipflop = false;
  if(_connState == 0) {
    _doStartupProbe();
  }

  else if(_connState == 1) {
    long tDelta = xTaskGetTickCount() - _tPause;
    if(tDelta >= 0) {
      _tPause += 1000;

      if(CDemandManager::isThermostat()) {
        _doThermo();
      }
      else {
        // _reqStatus();
//        _reqDemand(0);
        _matchDemand(CDemandManager::getPumpHz());
      }

      if(_htrData.HeaterOn) {
        if((flipflop = !flipflop))
          _reqVolts();
        else
          _reqBodyT();
      }
    }
  }
}

void 
CAltCommsTask::_doStartupProbe()
{
  // interrogate the ehater and extract the fixed pump rates for each heat demand setting H1->H6
  if(_startup.state == 0) {
    // erase pump rate record
    _htrData.init();
    _startup.pumpIdx = 0;
    putTxQueue(0xAA);   // request initial pump rate
    _tPause = xTaskGetTickCount() + 600;
    _startup.state++;
  }

  else if(_startup.state == 1) {
    if(_htrData.pumpRate[_startup.pumpIdx] > 0) {
      _startup.state++;  
    }
    else {
      long tDelta = xTaskGetTickCount() - _tPause;
      if(tDelta >= 0) {
        _startup.state = 0;
      }
    }
  }

  else if(_startup.state == 2) {
    if(_htrData.pumpRate[5] < 0) {
      _startup.pumpIdx++;
      if(_startup.pumpIdx == 6) {
        _startup.state = 0;
      }
      else {
        putTxQueue(0xAD);  // req next pump rate
        _tPause = xTaskGetTickCount() + 600;
        _startup.state++;
      }
    }
    else {
      putTxQueue(0xAA);   // finished - leave
      _tPause = xTaskGetTickCount() + 600;
      _connState = 1;
    }
  }

  else if(_startup.state == 3) {
    if(_htrData.pumpRate[_startup.pumpIdx] > 0) {
      _startup.state = 2;  
    }
    else {
      long tDelta = xTaskGetTickCount() - _tPause;
      if(tDelta >= 0) {
        _startup.state = 0;
      }
    }
  }
}


void 
CAltCommsTask::checkEvents()
{
  int rxHeater;
  if(AltCommsTask.getRxQueue(rxHeater)) {
    _decode(rxHeater);
  }
  isActive();

  manage();
}


bool 
CAltCommsTask::isActive() {
  if(_htrData.Active) {
    long tDelta = millis() - _htrData.Active;
    if(tDelta > 0) {
      _htrData.Active = 0;
    }
  }
  return _htrData.Active != 0;
}


void 
CAltCommsTask::_doThermo()
{
  static int demandMemory = 5;
  uint8_t ThermostatMode = NVstore.getUserSettings().ThermostatMethod;  // get the METHOD of thermostat control
  float Window = NVstore.getUserSettings().ThermostatWindow;
  float tCurrent = getTemperatureSensor();
  float tDesired = float(CDemandManager::getDegC());
  float tDelta = tCurrent - tDesired;
  Window /= 2;
  switch(ThermostatMode) {

    case 3:  // GPIO controlled thermostat mode
      if(CDemandManager::isExtThermostatMode()) {
        if(CDemandManager::isExtThermostatOn()) { 
          _matchDemand(5);
        }
        else {
          _matchDemand(0);
        }
        break;
      }
      // deliberately fall through if not enabled for GPIO control to standard thermostat
      // |
      // V
    case 0:  // conventional heater controlled thermostat mode
      Window = 1.0;
      // deliberately fall through with +-1C window
      // |
      // V
    case 1:  // heater controlled thermostat mode - BUT actual temp is tweaked via a changed window
      // if(fabs(tDelta) < Window) {
      //   _reqThermo(0); // hold at desired if inside window
      // }
      // else if(tDelta < -Window) {
      //   _reqThermo(+1); 
      // }
      // else  {
      //   _reqThermo(-1); 
      // }
      if(tDelta >= Window) {
        demandMemory = 0;            // flip flop to minimum power when over +ve threshold
      }
      else if(tDelta <= -Window){
        demandMemory = 5;            // flip flop to maximum power when under -ve threshold
      }
      _matchDemand(demandMemory);
      break;

    case 2:  // BTC controlled thermostat mode
          // map linear deviation within thermostat window to a Hz value,
          // Hz mode however uses the desired temperature field, somewhere between 8 - 35 for min/max
          // so create a desired "temp" according the the current hystersis
      if(fabs(tDelta) < Window/2) {
        _matchDemand(3);
      }
      else if(fabs(tDelta) < Window) {
        _matchDemand((tDelta > 0) ? 2 : 4);
      }
      else {
        _matchDemand((tDelta > 0) ? 0 : 5);
      }
      break;

    case 4:
      _matchDemand(5);
      break;
  }
}

void 
CAltCommsTask::_matchDemand(int desired)
{
  if(_htrData.Demand == desired)
    _reqDemand(0);
  else if(_htrData.Demand > desired)
    _reqDemand(-1);
  else 
    _reqDemand(+1);
}