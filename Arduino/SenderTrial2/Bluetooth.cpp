#include "Bluetooth.h"
#include "TxManage.h"
#include "pins.h"

#if defined (ESP32)
// NOTE: ESP32 uses an entirely different mechanism, please refer to BluetoothESP32.cpp/.h
#else

#if defined(__arm__)
// for Arduino Due
static UARTClass& Bluetooth(Serial2);
#else
// for Mega
static HardwareSerial& Bluetooth(Serial2);  // TODO: make proper ESP32 BT client
#endif

bool Bluetooth_Command(const char* cmd);
void Bluetooth_Interpret();

String BluetoothRxData;

const int BTRates[] = {
  9600, 38400, 115200, 19200, 57600, 2400, 4800
};

bool bHC05Available = false;

void Bluetooth_Init()
{
  #ifndef __ESP32__
  
  // search for BlueTooth adapter, trying the common baud rates, then less common
  // as the device cannot be guaranteed to power up with the key pin high
  // we are at the mercy of the baud rate stored in the module.
  Bluetooth.begin(9600);   
  digitalWrite(KeyPin, HIGH);
  delay(500);

  USB.println("\r\n\r\nAttempting to detect HC-05 Bluetooth module...");

  int BTidx = 0;
  int maxTries =  sizeof(BTRates)/sizeof(int);
  for(BTidx = 0; BTidx < maxTries; BTidx++) {
    USB.print("  @ ");
    USB.print(BTRates[BTidx]);
    USB.print(" baud... ");
    Bluetooth.begin(BTRates[BTidx]);   // open serial port at a certain baud rate
    Bluetooth.print("\r\n");
    Bluetooth.setTimeout(50);

    if(Bluetooth_Command("AT\r\n")) {
      USB.println(" OK.");
      break;
    }
    // failed, try another baud rate
    USB.println("");
    Bluetooth.flush();
  }

  USB.println("");
  if(BTidx == maxTries) {
    USB.println("FAILED to detect HC-05 Bluetooth module :-(");
  }
  else {
    if(BTRates[BTidx] == 115200) {
      USB.println("HC-05 found and already set to 115200 baud, skipping Init.");
      bHC05Available = true;
    }
    else {
      do {
        USB.println("HC-05 found");

        USB.print("  Setting Name to \"DieselHeater\"... ");
        if(!Bluetooth_Command("AT+NAME=\"DieselHeater\"\r\n")) {
          USB.println("FAILED");
          break;
        }
        USB.println("OK");

        USB.print("  Setting baud rate to 115200N81...");
        if(!Bluetooth_Command("AT+UART=115200,1,0\r\n")) {
          USB.println("FAILED");
          break;
        };
        USB.println("OK");

        Bluetooth.begin(115200);
        bHC05Available = true;

      } while(0);

    }
  }
  digitalWrite(KeyPin, LOW);  // leave HC-05 command mode

  delay(500);

  if(!bHC05Available)
    Bluetooth.end();    // close serial port if no module found

  USB.println("");
#endif
}

void Bluetooth_Check()
{
  // check for data coming back over Bluetooth
  if(bHC05Available) {
    if(Bluetooth.available()) {
      char rxVal = Bluetooth.read();
      if(isControl(rxVal)) {    // "End of Line"
        BluetoothRxData += '\0';
        Bluetooth_Interpret(BluetoothRxData);
        BluetoothRxData = "";
      }
      else {
        BluetoothRxData += rxVal;   // append new char to our Rx buffer
      }
    }
  }
}


void Bluetooth_Report(const char* pHdr, const unsigned char Data[24])
{
  if(bHC05Available) {
    Bluetooth.print(pHdr);
    Bluetooth.write(Data, 24);
  }
}

// local function, typically to perform Hayes commands with HC-05
bool Bluetooth_Command(const char* cmd)
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


void Bluetooth_Interpret(String line)
{
  if(line.startsWith("[CMD]") ) {
    USB.write("BT command Rx'd: ");
    // incoming command from BT app!
    line.remove(0, 5);   // strip away "[CMD]" header
    if(line.startsWith("ON") ) {
      USB.write("ON\n");
      TxManage.RequestOn();
    }
    else if(line.startsWith("OFF")) {
      USB.write("OFF\n");
      TxManage.RequestOff();
    }
    else if(line.startsWith("Pmin")) {
      line.remove(0, 4);
      USB.write("Pmin\n");
    }
    else if(line.startsWith("Pmax")) {
      line.remove(0, 4);
      USB.write("Pmax\n");
    }
    else if(line.startsWith("Fmin")) {
      line.remove(0, 4);
      USB.write("Fmin\n");
    }
    else if(line.startsWith("Fmax")) {
      line.remove(0, 4);
      USB.write("Fmax\n");
    }
  }
}

