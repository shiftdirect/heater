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


#include "BluetoothHC05.h"
#include "BluetoothSerial.h"

class CBluetoothESP32HC05 : public CBluetoothHC05 {
  int _rxPin, _txPin;
public:
  CBluetoothESP32HC05(int keyPin, int sensePin, int rxPin, int txPin);
protected:
  void openSerial(int baudrate);
};

class CBluetoothESP32Classic : public CBluetoothAbstract {
  BluetoothSerial SerialBT;
public:
  virtual void init();
  virtual void sendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm=true);
  virtual void check();
};

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class CBluetoothESP32BLE : public CBluetoothAbstract {
  const char* SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"; // UART service UUID
  const char* CHARACTERISTIC_UUID_RX = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
  const char* CHARACTERISTIC_UUID_TX = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";
  BLEServer *_pServer;
  BLECharacteristic* _pTxCharacteristic;
  volatile bool _deviceConnected;
  bool _oldDeviceConnected;
  void BLE_Send(std::string Data);
public:
  CBluetoothESP32BLE();
  virtual ~CBluetoothESP32BLE();
  virtual void init();
  virtual void sendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm=true);
  virtual void check();
};