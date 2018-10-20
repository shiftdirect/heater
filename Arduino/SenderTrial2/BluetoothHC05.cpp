#include "Bluetooth.h"
#include "pins.h"
#include "Protocol.h"
#include "debugport.h"

// Bluetooth access via HC-05 Module, using a UART

#ifndef ESP32
// NOTE: ESP32 uses an entirely different mechanism, please refer to BluetoothESP32.cpp/.h

#ifdef __arm__
// for Arduino Due
static UARTClass& Bluetooth(Serial2);
#else
// for Mega
static HardwareSerial& Bluetooth(Serial2); // TODO: make proper ESP32 BT client
#endif

bool Bluetooth_ATCommand(const char* cmd);

String BluetoothRxData;

const int BTRates[] = {
  9600, 38400, 115200, 19200, 57600, 2400, 4800
};

bool bHC05Available = false;

void Bluetooth_Init()
{

  // search for BlueTooth adapter, trying the common baud rates, then less common
  // as the device cannot be guaranteed to power up with the key pin high
  // we are at the mercy of the baud rate stored in the module.
  Bluetooth.begin(9600);   
  digitalWrite(KeyPin, HIGH);
  delay(500);

  DebugPort.println("\r\n\r\nAttempting to detect HC-05 Bluetooth module...");

  int BTidx = 0;
  int maxTries =  sizeof(BTRates)/sizeof(int);
  for(BTidx = 0; BTidx < maxTries; BTidx++) {
    DebugPort.print("  @ ");
    DebugPort.print(BTRates[BTidx]);
    DebugPort.print(" baud... ");
    Bluetooth.begin(BTRates[BTidx]);   // open serial port at a certain baud rate
    Bluetooth.print("\r\n");
    Bluetooth.setTimeout(50);

    if(Bluetooth_ATCommand("AT\r\n")) {
      DebugPort.println(" OK.");
      break;
    }
    // failed, try another baud rate
    DebugPort.println("");
    Bluetooth.flush();
  }

  DebugPort.println("");
  if(BTidx == maxTries) {
    DebugPort.println("FAILED to detect HC-05 Bluetooth module :-(");
  }
  else {
    if(BTRates[BTidx] == 115200) {
      DebugPort.println("HC-05 found and already set to 115200 baud, skipping Init.");
      bHC05Available = true;
    }
    else {
      do {
        DebugPort.println("HC-05 found");

        DebugPort.print("  Setting Name to \"DieselHeater\"... ");
        if(!Bluetooth_ATCommand("AT+NAME=\"DieselHeater\"\r\n")) {
          DebugPort.println("FAILED");
          break;
        }
        DebugPort.println("OK");

        DebugPort.print("  Setting baud rate to 115200N81...");
        if(!Bluetooth_ATCommand("AT+UART=115200,1,0\r\n")) {
          DebugPort.println("FAILED");
          break;
        };
        DebugPort.println("OK");

        Bluetooth.begin(115200);
        bHC05Available = true;

      } while(0);

    }
  }
  digitalWrite(KeyPin, LOW);  // leave HC-05 command mode

  delay(500);

  if(!bHC05Available)
    Bluetooth.end();    // close serial port if no module found

  DebugPort.println("");
}

void Bluetooth_Check()
{
  // check for data coming back over Bluetooth
  if(bHC05Available) {
    if(Bluetooth.available()) {
      char rxVal = Bluetooth.read();
      if(isControl(rxVal)) {    // "End of Line"
        BluetoothRxData += '\0';
        Command_Interpret(BluetoothRxData);
        BluetoothRxData = "";
      }
      else {
        BluetoothRxData += rxVal;   // append new char to our Rx buffer
      }
    }
  }
}


void Bluetooth_SendFrame(const char* pHdr, const CProtocol& Frame)
{
  if(bHC05Available) {
    if(Frame.verifyCRC()) {
      Bluetooth.print(pHdr);
      Bluetooth.write(Frame.Data, 24);
    }
    else {
      DebugPort.print("Bluetooth data not sent, CRC error ");
      DebugPort.println(pHdr);
    }
  }
}

// local function, typically to perform Hayes commands with HC-05
bool Bluetooth_ATCommand(const char* cmd)
{
  if(bHC05Available) {
    Bluetooth.print(cmd);
    char RxBuffer[16];
    memset(RxBuffer, 0, 16);
    int read = Bluetooth.readBytesUntil('\n', RxBuffer, 16);  // \n is not included in returned string!
    if((read == 3) && (0 == strcmp(RxBuffer, "OK\r")) ) {
      return true;
    }
    return false;
  }
  return false;
}

#endif


