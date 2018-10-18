#include "Bluetooth.h"
#include "TxManage.h"
#include "pins.h"

#if defined(ESP32)


// ESP32

void Bluetooth_Interpret();

String BluetoothRxLine;

#ifndef ESP32_USE_BLE_RLJ

/////////////////////////////////////////////////////////////////////////////////////////
//                       CLASSIC BLUETOOTH
//                              |
//                              V


#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void Bluetooth_Init()
{
  if(!SerialBT.begin("ESPHEATER")) {
    Serial.println("An error occurred initialising Bluetooth");
  }
}

void Bluetooth_Check()
{
  if(SerialBT.available()) {
    char rxVal = SerialBT.read();
    if(isControl(rxVal)) {    // "End of Line"
      BluetoothRxLine += '\0';
      Bluetooth_Interpret(BluetoothRxLine);
      BluetoothRxLine = "";
    }
    else {
      BluetoothRxLine += rxVal;   // append new char to our Rx buffer
    }
  }
}

void Bluetooth_Report(const char* pHdr, const unsigned char Data[24])
{
  if(SerialBT.hasClient()) {
    SerialBT.print(pHdr);
    SerialBT.write(Data, 24);
  }
}

//                              ^
//                              |
//                       CLASSIC BLUETOOTH
/////////////////////////////////////////////////////////////////////////////////////////

#else // ESP32_USE_BLE_RLJ

/////////////////////////////////////////////////////////////////////////////////////////
//                             BLE
//                              |
//                              V


#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

void BLE_Send(std::string Data);

BLEServer *pServer = NULL;
BLECharacteristic* pTxCharacteristic = NULL;
volatile bool deviceConnected = false;
bool oldDeviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {

  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }

};


class MyCallbacks : public BLECharacteristicCallbacks {

  // this callback is called when the ESP WRITE characteristic has been written to by a client
  // We need to *read* the new information!
  void onWrite(BLECharacteristic* pCharacteristic) {

    std::string rxValue = pCharacteristic->getValue();

    while(rxValue.length() > 0) {
      char rxVal = rxValue[0];
      if(isControl(rxVal)) {    // "End of Line"
        Bluetooth_Interpret(BluetoothRxLine);
        BluetoothRxLine = "";
      }
      else {
        BluetoothRxLine += rxVal;   // append new char to our Rx buffer
      }
      rxValue.erase(0, 1);
    }
  }

};

void Bluetooth_Init()
{
  // create the BLE device
  BLEDevice::init("DieselHeater");

  // create the BLE server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks);

  // create the BLE service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // create a BLE characteristic
  pTxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pTxCharacteristic->addDescriptor(new BLE2902());


  BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE
  );
  pRxCharacteristic->setCallbacks(new MyCallbacks/*()*/);

  // start the service
  pService->start();
  // start advertising
  pServer->getAdvertising()->start();
  Serial.println("Awaiting a client to notify...");
}

void Bluetooth_Report(const char* pHdr, const unsigned char Data[24])
{
  if(deviceConnected) {
    // BLE can only squirt 20 bytes per packet.
    // build the entire message then divide and conquer
    std::string txData = pHdr;
    txData.append((char*)Data, 24);
    
    BLE_Send(txData);
  }
}

void Bluetooth_Check()
{
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }

}

// break down supplied string into 20 byte chunks (or less)
// BLE can only handle 20 bytes per packet!
void BLE_Send(std::string Data)
{
  while(!Data.empty()) {
    std::string substr = Data.substr(0, 20);
    int len = substr.length();    
    pTxCharacteristic->setValue((uint8_t*)Data.data(), len);
    pTxCharacteristic->notify();
    Data.erase(0, len);
  }
}

//                              ^
//                              |
//                             BLE
/////////////////////////////////////////////////////////////////////////////////////////

#endif // ESP32_USE_BLE_RLJ

#endif  // __ESP32__