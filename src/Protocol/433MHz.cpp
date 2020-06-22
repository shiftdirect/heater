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
#include "433MHz.h"
#include "../cfg/pins.h"
#include "../Utility/macros.h"
#include "../Utility/NVStorage.h"
#include "../Utility/helpers.h" 

#define DEBUG_433MHz

C433MHzRemote UHFremote;

// static void IRAM_ATTR rmt_driver_isr_default(void *arg);


C433MHzRemote::C433MHzRemote()
{
  _rxQueue = NULL;
  _taskHandle = NULL;
  _runState = 0;
  _prevCode = 0;
  _timeout = 0;
  _debug = false;
}

C433MHzRemote::~C433MHzRemote()
{
  end();
}

void 
C433MHzRemote::_staticTask(void* arg)
{
  C433MHzRemote* pThis = (C433MHzRemote*)arg;

  pThis->_task();

  vTaskDelete(NULL);
}


void  
C433MHzRemote::_task()
{
  rmt_rx_start(_rxCfg.channel, true);
  _runState = 1;
  while(_runState == 1) {
    _doComms();
    delay(1);
  }
  rmt_rx_stop(_rxCfg.channel);
  _runState = 0;
}


void 
C433MHzRemote::_doComms()
{
  // wait for ring buffer response, or time out
  size_t rx_size;
  rmt_item32_t* rxItems = (rmt_item32_t *)xRingbufferReceive(_ringbuffer, &rx_size, 45);

  if (rxItems) {
    _decodeRxItems(rxItems, rx_size / 4);
    vRingbufferReturnItem(_ringbuffer, (void *)rxItems);
  }

  if(_timeout) {
    long tDelta = xTaskGetTickCount() - _timeout;
    if(tDelta >= 0) {
      _timeout = 0;
      _prevCode = 0;
      if(_rxQueue) 
        xQueueSend(_rxQueue, &_prevCode, 0);   // inject no button press
    }
  }
}

bool 
C433MHzRemote::_decodeRxItems(const rmt_item32_t* rxItems, int size)
{
// #ifdef DEBUG_433MHz
//   Serial.printf("433MHz RxItems = %d\r\n", size);
//   for(int i=0; i<size; i++) {
//     Serial.printf("[%d] %d:%5d  %d:%5d\r\n", i, rxItems[i].level0, rxItems[i].duration0, rxItems[i].level1, rxItems[i].duration1);
//   }
// #endif
  int workingsize = 0;
  unsigned long newCode = 0;
  if(size == 32)
    workingsize = 23;
  else if(size == 25)
    workingsize = 24;
  else {
    if(_debug)
      Serial.printf("433MHz remote incorrect number of transitions: %d?\r\n", size); 
    return false;
  }
  // start OK, now read the 24 bit payload
  int meanBitTime = 0;
  for (int i = 0; i < workingsize; i++)
  {
    meanBitTime += rxItems[i].duration0 + rxItems[i].duration1;  // add 1st and 2nd part times
  }
  meanBitTime /= workingsize;

  for (int i = 0; i < workingsize; i++)
  {
    int bitTime = rxItems[i].duration0 + rxItems[i].duration1;  // add 1st and 2nd part times
    // if(INBOUNDS(bitTime, 900, 1700)    // confirm duration
    if(INBOUNDS(bitTime, meanBitTime - 500, meanBitTime + 500)    // confirm duration
        && rxItems[i].level0 == 1        // confirm 1st part is high
        && rxItems[i].level1 == 0)       // confirm 2nd part is low
    {

      newCode <<= 1;

      // OK, a 1 is accepted if high > 0.6ms
      // if(rxItems[i].duration0 > 600)
      if(rxItems[i].duration0 > meanBitTime/2)
        newCode |= 0x0001;
    }
    else {
      // Serial.printf("433MHz remote @ transition %d: bitTime=%d lvl0=%d lvl1=%d?\r\n", i, bitTime, rxItems[i].level0, rxItems[i].level1); 
      newCode = 0;
      return false;
    }
  }
  // Serial.printf("433MHz val = 0x%08lX (%d)\r\n", newCode, size);
  if(_prevCode != newCode) {
    _prevCode = newCode;
    if(_rxQueue)
      xQueueSend(_rxQueue, &newCode, 0);   // queue new button press
// #ifdef DEBUG_433MHz
      if(_debug) {
        Serial.printf("433MHz RxItems = %d (%d)\r\n", size, meanBitTime);
        for(int i=0; i<size; i++) {
          Serial.printf("[%2d] %d:%5d  %d:%5d (%d)\r\n", i, rxItems[i].level0, rxItems[i].duration0, rxItems[i].level1, rxItems[i].duration1, rxItems[i].duration0+rxItems[i].duration1);
        }
      }
// #endif
  }
  _timeout = (xTaskGetTickCount() + 100) | 1;  // | 1 ensures non zero - timeout to allow injection of no button press
  return true;
}


void 
C433MHzRemote::begin(gpio_num_t pin, rmt_channel_t channel)
{
  _readNV();

  _rxCfg.rmt_mode = RMT_MODE_RX;
  _rxCfg.channel = channel;
  _rxCfg.gpio_num = pin;
  _rxCfg.mem_block_num = 1;
  _rxCfg.clk_div = 80;  // 1us / clock
  _rxCfg.rx_config.filter_en = true;
  _rxCfg.rx_config.filter_ticks_thresh = 250;
  _rxCfg.rx_config.idle_threshold = 6000;    // > 6ms no transitions => end of Rx

  ESP_ERROR_CHECK(rmt_config(&_rxCfg));
  ESP_ERROR_CHECK(rmt_driver_install(_rxCfg.channel, 512, 0));
  // ringbuffer for rx
  ESP_ERROR_CHECK(rmt_get_ringbuf_handle(_rxCfg.channel, &_ringbuffer));
  
  _rxQueue = xQueueCreate(4, sizeof(unsigned long));

  xTaskCreate(_staticTask,              
              "UHFremoteTask",
              4000,
              this,
              TASK_PRIORITY_HEATERCOMMS,
              &_taskHandle);
  
}

void 
C433MHzRemote::end()
{
  DebugPort.printf("Stopping UHF remote task %d\r\n", _runState);
  if(_runState == 1) {       // check task is running
    _runState = 2;           // ask task to stop
    DebugPort.println("Stopping UHF remote task wait");
    while(_runState != 0) {
      vTaskDelay(1);
    }
    _taskHandle = NULL;
  }  

  ESP_ERROR_CHECK(rmt_driver_uninstall(_rxCfg.channel));
  _ringbuffer = NULL;
}

bool 
C433MHzRemote::available()
{
  unsigned long test;
  return xQueuePeek(_rxQueue, &test, 0) != 0;
}

bool 
C433MHzRemote::read(unsigned long& val)
{
  return xQueueReceive(_rxQueue, &val, 0) != 0;
}

// Data word in NV ram is stored as follows
//
//  | 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
//  |                        |                       |                       |                       |
//  |                        |                       |           |  Enabled  |       Key Codes       |
//  |                    Unique ID (20 bits)                     | U  D  R  S| KCU | KCD | KCR | KCS | 
//
//  Key enabled bits
//     U = Up     - key code in b7-b6
//     D = Down   - key code in b5-b5
//     R = Run    - key code in b3-b2
//     S = Stop   - key code in b1-b0
//
//  key code bits
//     00 => 0x01
//     01 => 0x02
//     10 => 0x04
//     11 => 0x08
//
void
C433MHzRemote::_readNV()
{
  for(int rmt=0; rmt<3; rmt++) {
    unsigned long code = NVstore.getUserSettings().UHFcode[rmt];
    for(int i=0; i<4; i++) {
      int mask = 0x100 << i;
      if(code & mask) {
        int uniqueID = (code >> 8) & 0xFFFFF0;
        int shift = (code >> (i*2)) & 0x3;
        int keyCode = 1 << shift;
        _rawCodes[rmt][i] = uniqueID | keyCode;
      }
      else 
        _rawCodes[rmt][i] = 0;
    }
    DebugPort.printf("0x%08lX => 0x%08lX 0x%08lX 0x%08lX 0x%08lX\r\n", code, _rawCodes[rmt][0], _rawCodes[rmt][1], _rawCodes[rmt][2], _rawCodes[rmt][3]);
  }
}

int
C433MHzRemote::saveNV(unsigned long codes[3][4])
{
  sUserSettings userSettings = NVstore.getUserSettings();

  for(int rmt=0; rmt<3; rmt++) {
    unsigned long uniqueCode = codes[rmt][0] & 0xFFFFF0;
    
    // confirm all recorded keys share the same unique code
    for(int i=1; i<4; i++) {
      if(codes[rmt][i] && (uniqueCode != (codes[rmt][i] & 0xFFFFF0))) {
        return -1;
      }
    }

    // start building the encoded value for NV storage
    unsigned long encoded = uniqueCode << 8;
    for(int i=0; i<4; i++) {
      if(codes[rmt][i]) {
        int keyCode = codes[rmt][i] & 0xf;
        switch(keyCode) {
          case 1:
            encoded |= (0 << i*2);
            break;
          case 2:
            encoded |= (1 << i*2);
            break;
          case 4:
            encoded |= (2 << i*2);
            break;
          case 8:
            encoded |= (3 << i*2);
            break;
          default:
            return -2;
            break;
        }
        encoded |= (0x100 << i);
      }
    }
    userSettings.UHFcode[rmt] = encoded;

    DebugPort.printf("0x%08lX 0x%08lX 0x%08lX 0x%08lX => 0x%08lX\r\n", codes[rmt][0], codes[rmt][1], codes[rmt][2], codes[rmt][3], encoded);
  }

  NVstore.setUserSettings(userSettings);
  NVstore.save();

  for(int rmt=0; rmt<3; rmt++) {
    for(int i=0; i<4; i++) {
      _rawCodes[rmt][i] = codes[rmt][i];
    }
  }

  return 0;
}

void 
C433MHzRemote::getCodes(unsigned long codes[3][4])
{
  for(int rmt=0; rmt<3; rmt++) {
    for(int i=0; i<4; i++) {
      codes[rmt][i] = _rawCodes[rmt][i];
    }
  }
}

void 
C433MHzRemote::manage()
{
  if(available()) {
    unsigned long code;
    read(code);
    DebugPort.printf("UHF remote code = %08lX\r\n", code);

    if(code) {  // only react to an actual code, not release
      const int IDmatch = (code << 8) & 0xfffff000;
      int rmt;
      // find a mtaching unique ID
      for(rmt=0; rmt<3; rmt++) {
        if( IDmatch == (NVstore.getUserSettings().UHFcode[rmt] & 0xfffff000) ) {
          break; 
        }
      }
      if(rmt == 3)
        return;  // match not found - abort

      const int subCode = code & 0xf;
      if(subCode == (_rawCodes[rmt][0] & 0xf) ) {
        DebugPort.println("UHF stop request!");
        requestOff();
      }
      if(subCode == (_rawCodes[rmt][1] & 0xf) ) {
        DebugPort.println("UHF start request!");
        requestOn();
      }
      if(subCode == (_rawCodes[rmt][2] & 0xf) ) {
        DebugPort.println("UHF dec temp request!");
        CDemandManager::deltaDemand(-1);
      }
      if(subCode == (_rawCodes[rmt][3] & 0xf) ) {
        DebugPort.println("UHF inc temp request!");
        CDemandManager::deltaDemand(+1);
      }
    }
  }
}

void 
C433MHzRemote::enableISR(bool state)
{
  rmt_set_rx_intr_en(_rxCfg.channel, state);
  rmt_set_err_intr_en(_rxCfg.channel, state);
}

extern C433MHzRemote UHFremote;

