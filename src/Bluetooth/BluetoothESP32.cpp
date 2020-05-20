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

#include <Arduino.h>
#include "../cfg/pins.h"
#include "../cfg/BTCConfig.h"
#include "../Protocol/Protocol.h"
#include "../Utility/DebugPort.h"
#include "BluetoothESP32.h"


#ifdef ESP32

#if USE_HC05_BLUETOOTH == 1
/////////////////////////////////////////////////////////////////////////////////////////
//                    HC-05 BLUETOOTH with ESP32
//                              |
//                              V
//
CBluetoothESP32HC05::CBluetoothESP32HC05(int keyPin, int sensePin, int rxPin, int txPin) : CBluetoothHC05(keyPin, sensePin)
{
  _rxPin = rxPin;
  _txPin = txPin;

  digitalWrite(_txPin, HIGH);  // set high before making an output to avoid low glitch
  pinMode(_txPin, OUTPUT);
  pinMode(_rxPin, INPUT_PULLUP);
}

void 
CBluetoothESP32HC05::_openSerial(int baudrate)
{
  // Open Serial port on the ESP32
  // best to explicitly specify pins for the pin multiplexer!   
  HC05_SerialPort.begin(baudrate, SERIAL_8N1, _rxPin, _txPin);
  pinMode(_rxPin, INPUT_PULLUP);  // newer modules seem to be open drian - sort of - need a pullup to work properly anyway
}
//                              ^
//                              |
//                    HC-05 BLUETOOTH with ESP32
/////////////////////////////////////////////////////////////////////////////////////////

#endif

#if USE_CLASSIC_BLUETOOTH == 1
/////////////////////////////////////////////////////////////////////////////////////////
//                  CLASSIC BLUETOOTH on ESP32
//                              |
//                              V

void
CBluetoothESP32Classic::begin()
{
  _rxLine.clear();
  DebugPort.println("Initialising ESP32 Classic Bluetooth");

  if(!SerialBT.begin("ESPHEATER")) {
    DebugPort.println("An error occurred initialising Bluetooth");
  }
}

void 
CBluetoothESP32Classic::check()
{
  if(SerialBT.available()) {
    char rxVal = SerialBT.read();
    collectRxData(rxVal);
  }
}

bool 
CBluetoothESP32Classic::send(const char* Str)
{
  if(isConnected()) {

#if BT_LED == 1     
    digitalWrite(LED_Pin, !digitalRead(LED_Pin)); // toggle LED
#endif
    SerialBT.write((uint8_t*)Str, strlen(Str));
    delay(10);
    return true;
  }
  else {
    DebugPort.println("No Bluetooth client");
#if BT_LED == 1     
    digitalWrite(LED_Pin, 0);
#endif
    return false;
  }
}

bool 
CBluetoothESP32Classic::isConnected()
{
  return SerialBT.hasClient();
}

//                              ^
//                              |
//                  CLASSIC BLUETOOTH on ESP32
/////////////////////////////////////////////////////////////////////////////////////////
#endif


#if USE_BLE_BLUETOOTH == 1
/////////////////////////////////////////////////////////////////////////////////////////
//                          BLE on ESP32
//                              |
//                              V

/*#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

void BLE_Send(std::string Data);*/

/*
BLEServer *pServer = NULL;
BLECharacteristic* pTxCharacteristic = NULL;
volatile bool deviceConnected = false;
bool oldDeviceConnected = false;
*/

class MyServerCallbacks : public BLEServerCallbacks {
  volatile bool& _deviceConnected;
public:
  MyServerCallbacks(volatile bool& devConnected) : _deviceConnected(devConnected), BLEServerCallbacks() {};
  
private:
  void onConnect(BLEServer* pServer) {
    _deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    _deviceConnected = false;
  }

};


class MyCallbacks : public BLECharacteristicCallbacks {
  CBluetoothESP32BLE* _pHost;
public:
  MyCallbacks(CBluetoothESP32BLE* pHost) : BLECharacteristicCallbacks() { 
    _pHost = pHost; 
  };
private:
  // this callback is called when the ESP WRITE characteristic has been written to by a client
  // We need to *read* the new information!
  void onWrite(BLECharacteristic* pCharacteristic) {

    std::string rxValue = pCharacteristic->getValue();

    while(rxValue.length() > 0) {
      char rxVal = rxValue[0];
      if(_pHost) _pHost->collectRxData(rxVal);
      rxValue.erase(0, 1);
    }
  }

};

CBluetoothESP32BLE::CBluetoothESP32BLE()
{
  _pServer = NULL;
  _pTxCharacteristic = NULL;
  _deviceConnected = false;
  _oldDeviceConnected = false;
}

CBluetoothESP32BLE::~CBluetoothESP32BLE()
{

}

void 
CBluetoothESP32BLE::begin()
{
  DebugPort.println("Initialising ESP32 BLE");
  // create the BLE device
  BLEDevice::init("DieselHeater");

  // create the BLE server
  _pServer = BLEDevice::createServer();
  _pServer->setCallbacks(new MyServerCallbacks(_deviceConnected));

  // create the BLE service
  BLEService *pService = _pServer->createService(SERVICE_UUID);

  // create a BLE characteristic
  _pTxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  _pTxCharacteristic->addDescriptor(new BLE2902());


  BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE
  );
  pRxCharacteristic->setCallbacks(new MyCallbacks(this));

  // start the service
  pService->start();
  // start advertising
  _pServer->getAdvertising()->start();
  DebugPort.println("Awaiting a client to notify...");
}

/*void 
CBluetoothESP32BLE::sendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm)
{
  char fullMsg[32];

  // report to debug port
  CBluetoothAbstract::sendFrame(pHdr, Frame, lineterm);

  delay(40);
  if(isConnected()) {

    if(Frame.verifyCRC()) {
#if BT_LED == 1     
      digitalWrite(LED_Pin, !digitalRead(LED_Pin)); // toggle LED
#endif
      std::string txData = pHdr;
      txData.append((char*)Frame.Data, 24);

      BLE_Send(txData);
      delay(10);
    }
    else {
      DebugPort.println("Data not sent to Bluetooth, CRC error!");
    }
  }
  else {
    DebugPort.println("No Bluetooth client");
#if BT_LED == 1     
    digitalWrite(LED_Pin, 0);
#endif
  }
}
*/
bool 
CBluetoothESP32BLE::send(const char* Str)
{
  char fullMsg[32];

  if(isConnected()) {

#if BT_LED == 1     
    digitalWrite(LED_Pin, !digitalRead(LED_Pin)); // toggle LED
#endif
    std::string txData = Str;

    BLE_Send(txData);
    delay(10);
    return true;
  }
  else {
    DebugPort.println("No Bluetooth client");
#if BT_LED == 1     
    digitalWrite(LED_Pin, 0);
#endif
    return false;
  }
}

bool
CBluetoothESP32BLE::isConnected()
{
  return _deviceConnected;
}

void 
CBluetoothESP32BLE::check()
{
  // disconnecting
  if (!_deviceConnected && _oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    _pServer->startAdvertising(); // restart advertising
    DebugPort.println("start advertising");
    DebugPort.println("CLIENT DISCONNECTED");
    _oldDeviceConnected = _deviceConnected;
  }
  // connecting
  if (_deviceConnected && !_oldDeviceConnected) {
    // do stuff here on connecting
    DebugPort.println("CLIENT CONNECTED");
    _oldDeviceConnected = _deviceConnected;
  }

}

// break down supplied string into 20 byte chunks (or less)
// BLE can only handle 20 bytes per packet!
void 
CBluetoothESP32BLE::BLE_Send(std::string Data)
{
  while(!Data.empty()) {
    std::string substr = Data.substr(0, 20);
    int len = substr.length();    
    _pTxCharacteristic->setValue((uint8_t*)Data.data(), len);
    _pTxCharacteristic->notify();
    Data.erase(0, len);
  }
}

//                              ^
//                              |
//                          BLE on ESP32
/////////////////////////////////////////////////////////////////////////////////////////
#endif



#endif  // __ESP32__

