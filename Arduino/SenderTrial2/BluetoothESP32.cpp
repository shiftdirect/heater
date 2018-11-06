#include <Arduino.h>
#include "pins.h"
#include "Protocol.h"
#include "debugport.h"
#include "BluetoothESP32.h"

#ifdef TELNET
#define DebugPort Debug
#endif

#ifndef TELNET
#define DebugPort Serial
#endif


#ifdef ESP32

#ifdef BT_LED      
const int LED = 2;
#endif

/////////////////////////////////////////////////////////////////////////////////////////
//                    HC-05 BLUETOOTH with ESP32
//                              |
//                              V
//
CBluetoothESP32HC05::CBluetoothESP32HC05(int keyPin, int sensePin, int rxPin, int txPin) : CBluetoothHC05(keyPin, sensePin)
{
  _rxPin = rxPin;
  _txPin = txPin;
}

void 
CBluetoothESP32HC05::openSerial(int baudrate)
{
  // Open Serial port on the ESP32
  // best to explicitly specify pins for the pin multiplexer!   
  HC05_SerialPort.begin(baudrate, SERIAL_8N1, _rxPin, _txPin);
}
//                              ^
//                              |
//                    HC-05 BLUETOOTH with ESP32
/////////////////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////////////////////////////
//                  CLASSIC BLUETOOTH on ESP32
//                              |
//                              V

void
CBluetoothESP32Classic::init()
{
  _rxLine.clear();
#ifdef BT_LED      
  pinMode(LED, OUTPUT);
#endif

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

void 
CBluetoothESP32Classic::sendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm)
{
  char fullMsg[32];

  // report to debug port
  CBluetoothAbstract::sendFrame(pHdr, Frame, lineterm);
  
  delay(40);
  if(SerialBT.hasClient()) {

    if(Frame.verifyCRC()) {
#ifdef BT_LED      
      digitalWrite(LED, !digitalRead(LED)); // toggle LED
#endif
      int len = strlen(pHdr);
      if(len < 8) {
        strcpy(fullMsg, pHdr);
        memcpy(&fullMsg[len], Frame.Data, 24);

        SerialBT.write((uint8_t*)fullMsg, 24+len);
      }
      delay(10);
    }
    else {
      DebugPort.println("Data not sent to Bluetooth, CRC error!");
    }
  }
  else {
    DebugPort.println("No Bluetooth client");
#ifdef BT_LED      
    digitalWrite(LED, 0);
#endif
  }
}
 
//                              ^
//                              |
//                  CLASSIC BLUETOOTH on ESP32
/////////////////////////////////////////////////////////////////////////////////////////


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
CBluetoothESP32BLE::init()
{
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

void 
CBluetoothESP32BLE::sendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm)
{
  char fullMsg[32];

  // report to debug port
  CBluetoothAbstract::sendFrame(pHdr, Frame, lineterm);

  delay(40);
  if(_deviceConnected) {

    if(Frame.verifyCRC()) {
#ifdef BT_LED      
      digitalWrite(LED, !digitalRead(LED)); // toggle LED
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
#ifdef BT_LED      
    digitalWrite(LED, 0);
#endif
  }
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



#endif  // __ESP32__
