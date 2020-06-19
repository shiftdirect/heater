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

#include "CommsTask.h"
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

#define RX_DATA_TIMOUT 50
// #define ALTCTRL_DEBUG


char altdbgMsg[COMMS_MSGQUEUESIZE];

CAltCommsTask AltCommsTask;  // AltCommsTaskInfo;

volatile int nRunAltController = 0;
volatile bool bAltCommsOnline = false;

void buildTxItems(int val, rmt_item32_t txItems[9]);

rmt_channel_t CAltCommsTask::__txChannel;
volatile int CAltCommsTask::_txPending = 0;

CAltCommsTask::CAltCommsTask() : CCommsTask()
{
  _connState = 0;
  _startup.state = 0;
  _tPause = 0;
  _startup.pumpIdx = 0;
}

void 
CAltCommsTask::taskStart() 
{
  CCommsTask::taskStart();
  _runState = 0;           
  xTaskCreate(commsTask,              
              "AltCtrlrTask",
              4000,
              this,
              TASK_PRIORITY_HEATERCOMMS,
              &_taskHandle);
}

void 
CAltCommsTask::commsTask(void* arg) {
  //////////////////////////////////////////////////////////////////////////////////////
  // Alternate controller data reception
  //
  CAltCommsTask* pThis = (CAltCommsTask*)arg;

  pThis->_task();

  vTaskDelete(NULL);   // NEVER fall out from a task!
  for(;;);
}

void 
CAltCommsTask::_task()
{  // create FreeRTOS queues etc
  // pThis->create(ALTCTRL_DATAQUEUESIZE);
  create(ALTCTRL_DATAQUEUESIZE);

  pinMode(Tx1Pin, OUTPUT);  
  pinMode(Rx1Pin, INPUT_PULLUP);  
  pinMode(TxEnbPin, OUTPUT);

  // RMT Tx configuration
  _txCfg.rmt_mode = RMT_MODE_TX;
  _txCfg.channel = __txChannel = RMT_CHANNEL_2;
  _txCfg.gpio_num = Tx1Pin;
  _txCfg.mem_block_num = 1;
  _txCfg.tx_config.loop_en = 0;
  _txCfg.tx_config.idle_output_en = 1;
  _txCfg.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
  _txCfg.tx_config.carrier_en = 0;
  _txCfg.tx_config.carrier_duty_percent = 50;
  _txCfg.tx_config.carrier_freq_hz = 10000;
  _txCfg.tx_config.carrier_level = RMT_CARRIER_LEVEL_LOW;
  _txCfg.clk_div = 80;  // 1us / tick

  // RMT Rx configuration
  _rxCfg.rmt_mode = RMT_MODE_RX;
  _rxCfg.channel = RMT_CHANNEL_3;
  _rxCfg.gpio_num = Rx1Pin;
  _rxCfg.mem_block_num = 1;
  _rxCfg.clk_div = 80;  // 1us / clock
  _rxCfg.rx_config.filter_en = true;
  _rxCfg.rx_config.filter_ticks_thresh = 100;
  _rxCfg.rx_config.idle_threshold = 32000;    // > 32ms no transitions => end of Rx

  ESP_ERROR_CHECK(rmt_config(&_txCfg));
  ESP_ERROR_CHECK(rmt_driver_install(_txCfg.channel, 0, 0));
  
  ESP_ERROR_CHECK(rmt_config(&_rxCfg));
  ESP_ERROR_CHECK(rmt_driver_install(_rxCfg.channel, 512, 0));
  // ringbuffer for rx
  ESP_ERROR_CHECK(rmt_get_ringbuf_handle(_rxCfg.channel, &_ringbuffer));

  // setup call back to terminate tx gate
  rmt_register_tx_end_callback(RmtSendDone, this);  
  // rmt_register_tx_end_callback(RmtSendDone, pThis);  

  _runState = 1;
  putTxQueue(0xA6);  // initial poll to detect heater
  while(_runState == 1) {

    int command;
    if(getTxQueue(command)) {
      doComms(command);  
    }
    delay(1);
  }
  // pThis->_runState = 1;
  // while(pThis->_runState == 1) {

  //   int command;
  //   if(pThis->getTxQueue(command)) {
  //     pThis->doComms(command);  
  //   }
  //   delay(1);
  // }

  // disconnect from RMT peripheral
  rmt_register_tx_end_callback(NULL, NULL);  
  ESP_ERROR_CHECK(rmt_driver_uninstall(_txCfg.channel));
  ESP_ERROR_CHECK(rmt_driver_uninstall(_rxCfg.channel));
  _ringbuffer = NULL;

  // return pins to standard GPIO functions
  pinMode(Tx1Pin, OUTPUT);  
  pinMode(Rx1Pin, INPUT_PULLUP);  // required for MUX to work properly
  pinMode(TxEnbPin, OUTPUT);
  digitalWrite(Tx1Pin, HIGH);
  digitalWrite(TxEnbPin, LOW);

  // pThis->_runState = 0;
  _runState = 0;
}

// static callback for end of RMT transmission - used to terminate Tx Gate pulse
void 
CAltCommsTask::RmtSendDone(rmt_channel_t channel, void *arg)
{
  if(channel == __txChannel) {
    gpio_set_level(TxEnbPin, 0);
    _txPending = 2;
  }
}


void 
CAltCommsTask::doComms(int command)
{
  // ensure the ringbuffer is cleared
  void *p;
  size_t s;
  while ((p = xRingbufferReceive(_ringbuffer, &s, 0)))
  {
    ESP_LOGD(TAG, "flushing entry");
    vRingbufferReturnItem(_ringbuffer, p);
  }

  // send the command, and wait for a response
  rmt_item32_t txItems[9];
  buildTxItems(command, txItems);  // create transition list
  digitalWrite(TxEnbPin, HIGH);    // enable Tx gate

  NVstore.takeSemaphore();   // an issue with RmtSDendDone occuring during NV saves exists - block NV saves whilst we send
  _txPending = 1;
  rmt_write_items(_txCfg.channel, txItems, 9, 0);  // send the transitions

  // await reception, TxGate holds Rx high during Tx
  rmt_rx_start(_rxCfg.channel, true);

  // wait for ring buffer response, or time out
  size_t rx_size;
  int toCount = 50;
  rmt_item32_t* rxItems = NULL;
  while(--toCount) {
    rxItems = (rmt_item32_t *)xRingbufferReceive(_ringbuffer, &rx_size, 10);
    if(_txPending == 2) {
      NVstore.giveSemaphore();
      _txPending = 0;
    }
    if(rxItems != NULL) 
      break;
  }

  if(_txPending) {
    NVstore.giveSemaphore();
    _txPending = 0;
  }

#ifdef ALTCTRL_DEBUG
  Serial.printf("RxItems = %d\r\n", rx_size/4);
  for(int i=0; i<rx_size/4; i++) {
    Serial.printf("[%d] %d:%5d  %d:%5d\r\n", i, rxItems[i].level0, rxItems[i].duration0, rxItems[i].level1, rxItems[i].duration1);
  }
#endif
  if (rxItems) {
    bAltCommsOnline |= decodeRxItems(rxItems, rx_size / 4);
    vRingbufferReturnItem(_ringbuffer, (void *)rxItems);
  }
  rmt_rx_stop(_rxCfg.channel);
}


void buildTxItems(int val, rmt_item32_t txItems[9])
{
  txItems[0].duration0 = 30000;    // 30ms low sync pulse
  txItems[0].level0 = 0;

  int mask = 0x80;
  for(int i = 0; i <8 ; i++) {
    txItems[i].duration1 = mask & val ? 8000 : 4000;  // HIGH interval: 8ms for a 1, 4ms for a zero
    txItems[i].level1 = 1;
    txItems[i+1].duration0 = mask & val ? 4000 : 8000;  // LOW interval: 4ms for a 1, 8ms for a zero
    txItems[i+1].level0 = 0;
    mask >>= 1;
  }
  txItems[8].duration1 = 250;    // 250us to drive line high - not relying upon pull up so much
  txItems[8].level1 = 1;
}

bool 
CAltCommsTask::decodeRxItems(rmt_item32_t* rxItems, int size)
{
  int read_data = 0;
  if (size >= 17)
  {

    // _          __   __   _    __   _              __________________
    //  |________| x|_| x|_| |x_| x|_| |x_|~~~~~~~|_|
    //           .    .    .    .    .    .
    //   Start    '1'  '1'   '0' '1'   '0'  ..... Last
    //
    // Start is held in rxItems[0].duration0 & level0, it is ~30ms long
    // Each data bit is always ~12ms long
    //   a '1' is 8ms high, 4ms low
    //   a '0' is 4ms high, 8ms low
    // Data bits are held in rxItems[n].level1 for 1st part & rxItems[n+1].duration0 for 2nd half

    // confirm valid start
    if(rxItems[0].level0 == 0 && INBOUNDS(rxItems[0].duration0, 29500, 30500)) {
      // start OK, now read the 16 bit payload
      for (int i = 0; i < 16; i++)
      {
        read_data <<= 1;

        // total bit time should be ~12ms
        int bitTime = rxItems[i].duration1 + rxItems[i+1].duration0;  // add 1st and 2nd part times
        if(INBOUNDS(bitTime, 11500, 12500)    // confirm duration
           && rxItems[i].level1 == 1          // confirm 1st part is high
           && rxItems[i+1].level0 == 0)       // confirm 2nd part is low
        {
          // OK, a 1 is accepted if high > 6ms
          if(rxItems[i].duration1 > 6000)
            read_data |= 0x0001;
        }
        else {
          sprintf(altdbgMsg, "Alt controller @%d bitTime=%d lvl1=%d lvl0=%d?\r\n", i, bitTime, rxItems[i].level1, rxItems[i+1].level0); 
          putMsgQueue(altdbgMsg);
          read_data = 0;
          return false;
        }
      }
    }
    else {
      strcpy(altdbgMsg, "Alt Controller decode Invalid start pulse\r\n");
      putMsgQueue(altdbgMsg);
      return false;
    }
  }
  // sprintf(altdbgMsg, "Alt controller read: 0x%04X\r\n", read_data); 
  // putMsgQueue(altdbgMsg);

  _online = true;
  putRxQueue(read_data);
  xSemaphoreGive(_semaphore);
  return true;
}

