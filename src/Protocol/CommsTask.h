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
#ifndef __COMMSTASK_H__
#define __COMMSTASK_H__

#include <FreeRTOS.h>
#include "../Utility/UtilClasses.h"

const int COMMS_MSGQUEUESIZE = 192;
const int ALTCTRL_DATAQUEUESIZE = sizeof(int);

extern void AltControllerTask(void*);

class CCommsTask {
protected:
  TaskHandle_t _taskHandle;
  QueueHandle_t _msgQueue;
  QueueHandle_t _rxQueue;
  QueueHandle_t _txQueue;
  SemaphoreHandle_t _semaphore;
  volatile int _runState;
  volatile bool _online;

public:
public:
  CCommsTask() {
    _taskHandle = NULL;
    _msgQueue = NULL;
    _rxQueue = NULL;
    _txQueue = NULL;
    _semaphore = NULL;
    _runState = 0;
    _online = false;
  }
  virtual ~CCommsTask() {
    taskStop();
    destroy();
  }
  void create(int dataElementSize) {
    if(_msgQueue == NULL) _msgQueue = xQueueCreate(20, COMMS_MSGQUEUESIZE);
    if(_rxQueue == NULL) _rxQueue = xQueueCreate(4, dataElementSize);
    if(_txQueue == NULL) _txQueue = xQueueCreate(4, dataElementSize);
    if(_semaphore == NULL) _semaphore = xSemaphoreCreateBinary();
  }
  void destroy() {
    vQueueDelete(_msgQueue);  _msgQueue = NULL;
    vQueueDelete(_rxQueue);   _rxQueue = NULL;
    vQueueDelete(_txQueue);   _txQueue = NULL;
    vSemaphoreDelete(_semaphore); _semaphore = NULL;
  }
  void putTxQueue(void* pData) {
    if(_txQueue)
      xQueueSend(_txQueue, pData, 0);
  }
  bool getTxQueue(void* pData) {
    if(_txQueue && xQueueReceive(_txQueue, pData, 0)) {
      return true;
    }
    return false;
  }
  void putRxQueue(void* pData) {
    if(_rxQueue)
      xQueueSend(_rxQueue, pData, 0);
  }
  bool getRxQueue(void* pData) {
    if(_rxQueue && xQueueReceive(_rxQueue, pData, 0)) 
      return true;
    return false;
  }
  void putMsgQueue(const char msg[COMMS_MSGQUEUESIZE]) {
    if(_msgQueue)
      xQueueSend(_msgQueue, msg, 0);
  }
  bool getMsgQueue(char msg[COMMS_MSGQUEUESIZE]) {
    if(_msgQueue && xQueueReceive(_msgQueue, msg, 0)) {
      return true;
    }
    return false;
  }
  TaskHandle_t getTaskHandle() const {
    return _taskHandle;
  }
  SemaphoreHandle_t getSemaphore() const {
    return  _semaphore;
  }
  virtual void taskStart() {
    _online = false;
  }
  virtual void taskStop()          // create task to run blue wire interface
  {
    DebugPort.printf("Stopping comms task %d\r\n", _runState);
    if(_runState == 1) {       // check task is running
      _runState = 2;           // ask task to stop
      DebugPort.println("Stopping comms task wait");
      while(_runState != 0) {
        vTaskDelay(1);
      }
      _taskHandle = NULL;
    }
  }  
  bool isOnline() const {
    return _online;
  }
};

#endif