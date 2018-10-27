#include "Bluetooth.h"
#include "pins.h"
#include "Protocol.h"
#include "debugport.h"

#ifdef ESP32

#define ESP32_USE_HC05

const int LED = 2;

// ESP32

sRxLine RxLine;

#ifdef ESP32_USE_HC05

//static HardwareSerial& Bluetooth(Serial2); // TODO: make proper ESP32 BT client

bool Bluetooth_ATCommand(const char* cmd);

const int BTRates[] = {
  9600, 38400, 115200, 19200, 57600, 2400, 4800, 1200
};

bool bHC05Available = false;

void Bluetooth_Init()
{
  RxLine.clear();
  
  // search for BlueTooth adapter, trying the common baud rates, then less common
  // as the device cannot be guaranteed to power up with the key pin high
  // we are at the mercy of the baud rate stored in the module.
  digitalWrite(KeyPin, HIGH);
  delay(5000);
  Serial2.begin(9600, SERIAL_8N1, Rx2Pin, Tx2Pin);  // need to explicitly specify pins for pin multiplexer!);   

  DebugPort.println("\r\n\r\nAttempting to detect HC-05 Bluetooth module...");

  int BTidx = 0;
  int maxTries =  sizeof(BTRates)/sizeof(int);
  for(BTidx = 0; BTidx < maxTries; BTidx++) {
    DebugPort.print("  @ ");
    DebugPort.print(BTRates[BTidx]);
    DebugPort.print(" baud... ");
    Serial2.begin(BTRates[BTidx], SERIAL_8N1, Rx2Pin, Tx2Pin);   // open serial port at a certain baud rate
    delay(100);
    Serial2.print("\r\n");
    Serial2.setTimeout(100);

    if(Bluetooth_ATCommand("AT\r\n")) {
      DebugPort.println(" OK.");
      break;
    }
    if(Bluetooth_ATCommand("AT\r\n")) {
      DebugPort.println(" OK.");
      break;
    }
    // failed, try another baud rate
    DebugPort.println("");
    Serial2.flush();
    Serial2.end();
    delay(1000);
  }

  DebugPort.println("");
  if(BTidx == maxTries) {
    DebugPort.println("FAILED to detect HC-05 Bluetooth module :-(");
  }
  else {
/*//    if(BTRates[BTidx] == 115200) {
    if(BTRates[BTidx] == 9600) {
//      DebugPort.println("HC-05 found and already set to 115200 baud, skipping Init.");
      DebugPort.println("HC-05 found and already set to 9600 baud, skipping Init.");
      bHC05Available = true;
    }
    else*/ {
      do {
        DebugPort.println("HC-05 found");

        DebugPort.print("  Setting Name to \"Diesel Heater\"... ");
        if(!Bluetooth_ATCommand("AT+NAME=\"Diesel Heater\"\r\n")) {
          DebugPort.println("FAILED");
          break;
        }
        DebugPort.println("OK");

//        DebugPort.print("  Setting baud rate to 115200N81...");
//        if(!Bluetooth_ATCommand("AT+UART=115200,1,0\r\n")) {
        DebugPort.print("  Setting baud rate to 9600N81...");
        if(!Bluetooth_ATCommand("AT+UART=9600,1,0\r\n")) {
          DebugPort.println("FAILED");
          break;
        };
        DebugPort.println("OK");

//        Serial2.begin(115200, SERIAL_8N1, Rx2Pin, Tx2Pin);
        Serial2.begin(9600, SERIAL_8N1, Rx2Pin, Tx2Pin);
        bHC05Available = true;

      } while(0);

    }
  }
  digitalWrite(KeyPin, LOW);  // leave HC-05 command mode

  delay(500);

  if(!bHC05Available)
    Serial2.end();    // close serial port if no module found

  DebugPort.println("");
}

void Bluetooth_Check()
{
  // check for data coming back over Bluetooth
  if(bHC05Available) {
    if(Serial2.available()) {
      char rxVal = Serial2.read();
      if(isControl(rxVal)) {    // "End of Line"
        Command_Interpret(RxLine.Line);
        RxLine.clear();
      }
      else {
        RxLine.append(rxVal);   // append new char to our Rx buffer
      }
    }
  }
}


void Bluetooth_SendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm)
{
  DebugPort.print(millis());
  DebugReportFrame(pHdr, Frame, lineterm ? "\r\n" : "   ");

  if(bHC05Available) {
    if(Frame.verifyCRC()) {
      digitalWrite(LED, !digitalRead(LED)); // toggle LED
      Serial2.print(pHdr);
      Serial2.write(Frame.Data, 24);
    }
    else {
      DebugPort.print("Bluetooth data not sent, CRC error ");
      DebugPort.println(pHdr);
    }
  }
  else {
    DebugPort.println("No Bluetooth client");
    digitalWrite(LED, 0);
  }
}

// local function, typically to perform Hayes commands with HC-05
bool Bluetooth_ATCommand(const char* cmd)
{
//  if(bHC05Available) {
    Serial2.print(cmd);
    char RxBuffer[16];
    memset(RxBuffer, 0, 16);
    int read = Serial2.readBytesUntil('\n', RxBuffer, 16);  // \n is not included in returned string!
    if((read == 3) && (0 == strcmp(RxBuffer, "OK\r")) ) {
      return true;
    }
    return false;
//  }
//  return false;
}

#else // ESP32_USE_HC05
#ifndef ESP32_USE_BLE_RLJ

/////////////////////////////////////////////////////////////////////////////////////////
//                       CLASSIC BLUETOOTH
//                              |
//                              V


#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void Bluetooth_Init()
{
  RxLine.clear();
  pinMode(LED, OUTPUT);

  if(!SerialBT.begin("ESPHEATER")) {
    DebugPort.println("An error occurred initialising Bluetooth");
  }
}

void Bluetooth_Check()
{
  if(SerialBT.available()) {
    char rxVal = SerialBT.read();
    if(isControl(rxVal)) {    // "End of Line"
      Command_Interpret(RxLine.Line);
      RxLine.clear();
    }
    else {
      RxLine.append(rxVal);
    }
  }
}

void Bluetooth_SendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm)
{
  char fullMsg[32];

  DebugPort.print(millis());
  DebugReportFrame(pHdr, Frame, lineterm ? "\r\n" : "   ");
  delay(40);
  if(SerialBT.hasClient()) {

    if(Frame.verifyCRC()) {
      digitalWrite(LED, !digitalRead(LED)); // toggle LED
      int len = strlen(pHdr);
      if(len < 8) {
        strcpy(fullMsg, pHdr);
        memcpy(&fullMsg[len], Frame.Data, 24);
        SerialBT.write((uint8_t*)fullMsg, 24+len);
      }
/*      SerialBT.print(pHdr);
      delay(1);
      SerialBT.write(Frame.Data, 24);*/
      delay(10);
    }
    else {
      DebugPort.println("Data not sent to Bluetooth, CRC error!");
    }
  }
  else {
    DebugPort.println("No Bluetooth client");
    digitalWrite(LED, 0);
  }
}

void Bluetooth_SendACK()
{
 /* if(SerialBT.hasClient()) {
    SerialBT.print("[ACK]");
  }*/
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
        Command_Interpret(BluetoothRxLine);
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
  DebugPort.println("Awaiting a client to notify...");
}

void Bluetooth_Report(const char* pHdr, const CProtocol& Frame)
{
  if(deviceConnected) {
    if(Frame.verifyCRC()) {
      // BLE can only squirt 20 bytes per packet.
      // build the entire message then divide and conquer
      std::string txData = pHdr;
      txData.append((char*)Frame.Data, 24);
    
      BLE_Send(txData);
    }
  }
}

void Bluetooth_Check()
{
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    DebugPort.println("start advertising");
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

#endif // ESP32_USE_HC05

#endif  // __ESP32__
