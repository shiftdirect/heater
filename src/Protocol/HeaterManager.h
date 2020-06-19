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
#ifndef __HEATERMANAGER_H__
#define __HEATERMANAGER_H__

#include <FreeRTOS.h>
#include "../Utility/UtilClasses.h"
#include "AltController.h"
#include "AltControllerTask.h"
#include "BlueWireTask.h"


class CHeaterManager {
  int  _heaterStyle;
  TaskHandle_t _taskHandle;
  CCommsTask* _pCommsTask;
public:
  CHeaterManager();
  bool detect();
  void setHeaterStyle(int mode);
  
  float getFanRPM() const;
  float getFanVoltage() const;
  float getPumpRate() const;
  float getPumpDemand() const;
  float getBodyTemp() const;
  float getGlowPlugPower() const;
  int   getErrState() const;
  int   getRunState() const;
  int   getRunStateEx() const;
  int   getHeaterStyle() const;
  const char* getErrStateStr() const;
  const char* getErrStateStrEx() const;
  const char* getRunStateStr() const;
  TaskHandle_t getTaskHandle() const;
  SemaphoreHandle_t getSemaphore() const;
  // QueueHandle_t getMsgQueue() const;

  void checkMsgEvents();
  void checkRxEvents();
  void checkTxEvents();
  bool isOnline();


  void reqOnOff(bool state);
  void reqVolts();
  void reqBodyT();
};

extern CHeaterManager HeaterManager;

#endif