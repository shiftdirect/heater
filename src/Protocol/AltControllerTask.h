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
#ifndef __ALTCTRLTASK_H__
#define __ALTCTRLTASK_H__

#include <FreeRTOS.h>
#include "../Utility/UtilClasses.h"
#include "CommsTask.h"
#include "driver/rmt.h"

struct sAltHeaterData {
private:
  int  BodyT;
public:
  bool thermoMode;
  unsigned long Active;
  bool HeaterOn;
  bool Stopping;
  bool GlowOn;
  bool FanOn;
  bool PumpOn;
  bool Plateau;
  int  Demand;
  int  Volts;
  int  Error;
  int  pumpRate[6];
  sAltHeaterData();
  void init();
  int  runState();
  int  errState();
  float  getDesiredPumpRate(int idx);
  float  getPumpRate();
  float  getBodyTemp();
  void   decodeBodyT(int rxData);
  void   decodeVolts(int rxData);
  void   decodeStatus(int rxData);
  void   decodePumpRate(int rxData);
  void   decodeThermoDemand(int rxData);
  void   decodeError(int rxData);
  void   report();
};



class CAltCommsTask : public CCommsTask {
protected:
  static void commsTask(void* arg);
  static void RmtSendDone(rmt_channel_t channel, void *arg);
  static rmt_channel_t __txChannel;
  volatile static int _txPending;
  rmt_config_t _txCfg;
  rmt_config_t _rxCfg;
  RingbufHandle_t _ringbuffer;

  void  _task();

  void doComms(int command);
  bool decodeRxItems(rmt_item32_t* rxItems, int size);
  sAltHeaterData _htrData;
  int  _connState;
  unsigned long _tPause;
  struct {
    int  state;
    int  pumpIdx;
  } _startup;

  void   _doStartupProbe();
  void   _decode(int rxData);  // interpret data word received from heater
  void   _reqStatus();
  void   _reqVolts();
  void   _reqBodyT();
  void   _reqDemand(int dir);
  void   _reqThermo(int delta);
  void   _doThermo();
  void   _matchDemand(int desired);

public:
  CAltCommsTask();
  void taskStart();

  void manage();
  void checkEvents();
  bool isActive();   // comms active and connected
  void reqPower();
  void reportDecode();

  float getFanRPM();
  float getDesiredPumpRate(int idx);
  float getActualPumpRate();
  float getGlow();
  float getBodyTemp();
  int   getRunState();
  int   getErrState();

  void putTxQueue(int command) {
    CCommsTask::putTxQueue(&command);
  }
  bool getTxQueue(int& command) {
    return CCommsTask::getTxQueue(&command);
  }
  void putRxQueue(int response) {
    CCommsTask::putRxQueue(&response);
  }
  bool getRxQueue(int& response) {
    return CCommsTask::getRxQueue(&response); 
  }

};

extern CAltCommsTask AltCommsTask;


#endif