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

#include "Protocol.h"

class CTxManage
{
  const int m_nStartDelay = 20; 
  const int m_nFrameTime = 14;  
  const int m_nFrontPorch = 0;  
  int m_sysUpdate;

public:
  CTxManage(int TxGatePin, HardwareSerial& serial);
  void queueOnRequest(bool set = true);   // use false to remove repeating command
  void queueOffRequest(bool set = true);  // use false to remove repeating command
  void queueRawCommand(uint8_t val);
  void PrepareFrame(const CProtocol& Frame, bool isBTCmaster);
  void Start(unsigned long timenow);
  bool CheckTx(unsigned long timenow);
  void begin();
  const CProtocol& getFrame() const { return m_TxFrame; };
  static void IRAM_ATTR callbackGateTerminate();
  void queueSysUpdate();  // use to implant NV settings into heater

private:
  HardwareSerial& m_BlueWireSerial;
  CProtocol m_TxFrame;
  bool m_bOnReq;
  bool m_bOffReq;
  bool m_bTxPending;
  static int  m_nTxGatePin;
  uint8_t _rawCommand;
  static unsigned long m_nStartTime;
  hw_timer_t *m_HWTimer;


};

extern CTxManage TxManage;

