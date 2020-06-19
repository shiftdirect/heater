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

#include <FreeRTOS.h>
#include "HeaterManager.h"
#include "../Utility/helpers.h"
#include "../Utility/macros.h"
#include "../Protocol/Protocol.h"
#include "Protocol/TxManage.h"
#include "Protocol/SmartError.h"
#include "Utility/NVStorage.h"

CHeaterManager HeaterManager;


const char* Runstates [] PROGMEM = {
  " Stopped/Ready ",      // 0
  "Starting...",          // 1
  "Igniting...",          // 2
  "Ignition retry pause", // 3
  "Ignited",              // 4
  "Running",              // 5
  "Stopping",             // 6 
  "Shutting down",        // 7
  "Cooling",              // 8
  "Heating glow plug",    // 9  - interpreted state - actually runstate 2 with no pump action!
  "Suspended",            // 10 - interpreted state - cyclic mode has suspended heater
  "Suspending...",        // 11 - interpreted state - cyclic mode is suspending heater
  "Suspend cooling",      // 12 - interpreted state - cyclic mode is suspending heater
  "Unknown run state"     
};

const char* Errstates [] PROGMEM = {
  "",                // [0] 
  "",                // [1] 
  "Low voltage",     // [2] E-01
  "High voltage",    // [3] E-02
  "Glow plug fault", // [4] E-03
  "Pump fault",      // [5] E-04
  "Overheat",        // [6] E-05
  "Motor fault",     // [7] E-06
  "Comms fault",     // [8] E-07
  "Flame out",       // [9] E-08
  "Temp sense",      // [10] E-09
  "Ignition fail",   // [11] E-10          SmartError manufactured state - sensing runstate 2 -> >5
  "Failed 1st ignite", // [12] E-11  SmartError manufactured state - sensing runstate 2 -> 3
  "Excess fuel usage", // [13] E-12  SmartError manufactured state - excess fuel consumed
  "Unknown error?"   // mystery code!
};

const char* ErrstatesEx [] PROGMEM = {
  "E-00: OK",              // [0] 
  "E-00: OK",              // [1]
  "E-01: Low voltage",     // [2] E-01
  "E-02: High voltage",    // [3] E-02
  "E-03: Glow plug fault", // [4] E-03
  "E-04: Pump fault",      // [5] E-04
  "E-05: Overheat",        // [6] E-05
  "E-06: Motor fault",     // [7] E-06
  "E-07: No heater comms", // [8] E-07
  "E-08: Flame out",       // [9] E-08
  "E-09: Temp sense",      // [10] E-09
  "E-10: Ignition fail",   // [11] E-10  SmartError manufactured state - sensing runstate 2 -> >5
  "E-11: Failed 1st ignite",  // [12] E-11  SmartError manufactured state - sensing runstate 2 -> 3
  "E-12: Excess fuel shutdown", // [13] E-12  SmartError manufactured state - excess fuel consumed
  "Unknown error?"   // mystery code!
};


CHeaterManager::CHeaterManager() {
  _heaterStyle = -1;
  _taskHandle = NULL;
  _pCommsTask = NULL;
}

bool 
CHeaterManager::detect()
{
  int style = NVstore.getHeaterTuning().heaterStyle;
  DebugPort.printf("Initial heater style %d\r\n", style);

  bool tested[2] = { false, false};
  bool found = false;

  for(;;) {
    if(tested[style]) {
      setHeaterStyle(NVstore.getHeaterTuning().heaterStyle);  // revert to saved heater type for now, but we did not find it
      return false;
    }

    tested[style] = true;
    setHeaterStyle(style);
    unsigned long timeout = millis() + 1500;

    long tDelta;
    do {
      delay(10);
      if(_pCommsTask && _pCommsTask->isOnline()) {
        found = true;
        sHeaterTuning tuning = NVstore.getHeaterTuning();
        DebugPort.printf("Found heater style %d\r\n", style);
        if(tuning.heaterStyle != style) {
          DebugPort.printf("saving heater style %d\r\n", style);
          tuning.heaterStyle = style;
          NVstore.setHeaterTuning(tuning);
          NVstore.save();
          // NVstore.doSave();
        }
        return true;  
      }
      tDelta = millis() - timeout;
    } while(tDelta < 0);

    style++;
    WRAPLIMITS(style, 0, 1);
  }
}

void 
CHeaterManager::setHeaterStyle(int mode)
{
  if(INBOUNDS(mode, 0, 1)) {
    if(mode != _heaterStyle) {


      // stop existing comms connection if running
      if(_pCommsTask) {
        _pCommsTask->taskStop();
        _taskHandle = NULL;
        _pCommsTask = NULL;
      }

      // start comms connection
      _heaterStyle = mode;
      switch(_heaterStyle) {
        case 0:
          _pCommsTask = &BlueWireCommsTask;
          break;

        case 1:
          _pCommsTask = &AltCommsTask;
          break;
      }

      if(_pCommsTask) {
        _pCommsTask->taskStart();
        delay(10);
      }
    }
  }
}

TaskHandle_t 
CHeaterManager::getTaskHandle() const
{
  if(_pCommsTask)
    return _pCommsTask->getTaskHandle();
  return NULL;
}

SemaphoreHandle_t 
CHeaterManager::getSemaphore() const
{
  if(_pCommsTask)
    return _pCommsTask->getSemaphore();
  return NULL;
}

// char dbgMsg[COMMS_MSGQUEUESIZE];

/*QueueHandle_t 
CHeaterManager::getMsgQueue() const
{
  switch(_heaterStyle) {
    case 0: return BlueWireMsgQueue;
    case 1: 
      if(AltCommsTask.getMsgQueue(dbgMsg))
        DebugPort.print(dbgMsg);
      return NULL;
    default: return NULL;
  }
}*/

  // QueueHandle_t MsgQueue = HeaterManager.getMsgQueue();


int   
CHeaterManager::getHeaterStyle() const
{
  return _heaterStyle;
}


float CHeaterManager::getFanRPM() const
{
  switch(_heaterStyle) {
    case 0: return getHeaterInfo().getFan_Actual();
    case 1: return AltCommsTask.getFanRPM();
    default: return 0;
  }
}

float 
CHeaterManager::getFanVoltage() const
{
  switch(_heaterStyle) {
    case 0: return getHeaterInfo().getFan_Voltage();
    case 1: return -1;
    default: return 0;
  }
}

float 
CHeaterManager::getPumpDemand() const
{
  switch(_heaterStyle) {
    case 0: return getHeaterInfo().getPump_Fixed();
    case 1: return AltCommsTask.getDesiredPumpRate(CDemandManager::getPumpHz()); //AltHeaterData.Demand;
    default: return 0;
  }
}

float 
CHeaterManager::getPumpRate() const
{
  switch(_heaterStyle) {
    case 0: 
      return getHeaterInfo().getPump_Actual();
    case 1: 
      return AltCommsTask.getActualPumpRate();
    default: 
      return 0;
  }
}

float 
CHeaterManager::getBodyTemp() const
{
  switch(_heaterStyle) {
    case 0: return getHeaterInfo().getTemperature_HeatExchg();
    case 1: return AltCommsTask.getBodyTemp(); 
    default: return 0;
  }
}

float 
CHeaterManager::getGlowPlugPower() const
{
  switch(_heaterStyle) {
    case 0: return getHeaterInfo().getGlowPlug_Power();
    case 1: return AltCommsTask.getGlow();
    default: return 0;
  }
}

int   
CHeaterManager::getRunStateEx() const
{
  switch(_heaterStyle) {
    case 0: return getHeaterInfo().getRunStateEx();
    case 1: return AltCommsTask.getRunState();
    default: return 0;
  }
}
int   
CHeaterManager::getRunState() const
{
  switch(_heaterStyle) {
    case 0: return getHeaterInfo().getRunState();
    case 1: return AltCommsTask.getRunState();
    default: return 0;
  }
}

int   
CHeaterManager::getErrState() const
{
  switch(_heaterStyle) {
    case 0: return getHeaterInfo().getErrState();
    case 1: return AltCommsTask.getErrState();
    default: return 8;
  }
}

const char* 
CHeaterManager::getRunStateStr() const 
{ 
  uint8_t runstate = getRunStateEx();
  UPPERLIMIT(runstate, 13);
  if(runstate == 2 && getPumpRate() == 0) {  // split runstate 2 - glow, then fuel
    runstate = 9;
  }
  return Runstates[runstate]; 
}

const char* 
CHeaterManager::getErrStateStr() const 
{ 
  uint8_t errstate = getErrState();
  UPPERLIMIT(errstate, 13);
  return Errstates[errstate]; 
}

const char* 
CHeaterManager::getErrStateStrEx() const 
{ 
  uint8_t errstate = getErrState();
  UPPERLIMIT(errstate, 13);
  return ErrstatesEx[errstate]; 
}



void 
CHeaterManager::reqOnOff(bool state)
{
  switch(_heaterStyle) {
    case 0:
      if(state) {
        TxManage.queueOnRequest();
        SmartError.reset();
      }
      else {
        TxManage.queueOffRequest();
        SmartError.inhibit();
      }
      break;
    case 1:
      DebugPort.println("Alt heater start request queued");
      AltCommsTask.reqPower();
      break;
  }
}

void 
CHeaterManager::checkRxEvents()
{
  if(_heaterStyle == 0) {
    checkBlueWireRxEvents();
  }
  else if(_heaterStyle == 1) {
    AltCommsTask.checkEvents();
  }
}

void 
CHeaterManager::checkTxEvents()
{
  if(_heaterStyle == 0) {
    checkBlueWireTxEvents();
  }
  else if(_heaterStyle == 1) {
    AltCommsTask.manage();//checkAltTxEvents();
  }
}

char taskMsg[COMMS_MSGQUEUESIZE];

void 
CHeaterManager::checkMsgEvents()
{
  if(_pCommsTask && _pCommsTask->getMsgQueue(taskMsg))
    DebugPort.print(taskMsg);
}

bool 
CHeaterManager::isOnline()
{
  switch(_heaterStyle) {
    case 0:
      return hasHtrData();
      break;
    case 1:
      return AltCommsTask.isActive();
      break;
  }
  return false;
}
