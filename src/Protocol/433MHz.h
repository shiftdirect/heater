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
#ifndef __433MHzREMOTE_H__
#define __433MHzREMOTE_H__

#include <FreeRTOS.h>
#include "../Utility/UtilClasses.h"
#include "driver/rmt.h"

class C433MHzRemote {
protected:
  static void _staticTask(void* arg);
  rmt_config_t _rxCfg;
  RingbufHandle_t _ringbuffer;
  QueueHandle_t _rxQueue;
  TaskHandle_t _taskHandle;
  int _runState;
  unsigned long _prevCode;
  unsigned long _timeout;
  unsigned long _rawCodes[3][4];
  bool _debug;

  void  _task();

  void _doComms();
  bool _decodeRxItems(const rmt_item32_t* rxItems, int size);
  // NV storage
  void _readNV();

public:
  C433MHzRemote();
  ~C433MHzRemote();
  void begin(gpio_num_t pin, rmt_channel_t channel);
  void end();

  bool available();
  bool read(unsigned long& val);
  void manage();

  void getCodes(unsigned long codes[3][4]);

  // NV storage
  int  saveNV(unsigned long codes[3][4]);
  void enableISR(bool state);
};

extern C433MHzRemote UHFremote;


#endif