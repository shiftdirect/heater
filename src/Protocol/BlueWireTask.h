/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2018  Ray Jones <ray@mrjones.id.au>
 * Copyright (C) 2018  James Clark
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
#ifndef __BLUEWIRETASK_H__
#define __BLUEWIRETASK_H__

#include <FreeRTOS.h>
#include "../Utility/UtilClasses.h"
#include "CommsTask.h"

extern QueueHandle_t BlueWireMsgQueue;    // cannot use general Serial.print etc from this task without causing conflicts
extern QueueHandle_t BlueWireRxQueue;   // queue to pass down heater receive data
extern QueueHandle_t BlueWireTxQueue;   // queue to pass down heater transmit data
extern SemaphoreHandle_t BlueWireSemaphore;  // flag to indicate completion of heater data exchange

const int BLUEWIRE_MSGQUEUESIZE = 192;
const int BLUEWIRE_DATAQUEUESIZE = 24;

// extern void BlueWireTask(void*);
extern CommStates CommState;

class CBlueWireCommsTask : public CCommsTask {
protected:
  static void commsTask(void* arg);
  void _initSerial();
  void _task();

public:
  CBlueWireCommsTask();
  void taskStart();

};

extern CBlueWireCommsTask BlueWireCommsTask;  // AltCommsTaskInfo;

#endif