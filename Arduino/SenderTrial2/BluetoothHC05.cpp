#include "Bluetooth.h"
#include "pins.h"
#include "Protocol.h"
#include "debugport.h"
#include "BluetoothHC05.h"

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

sRxLine RxLine;

const int BTRates[] = {
  9600, 38400, 115200, 19200, 57600, 2400, 4800
};

bool bHC05Available = false;


void Bluetooth_Init()
{
  RxLine.clear();
  
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
    Bluetooth.end();
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

        Bluetooth.end();
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
        Command_Interpret(RxLine.Line);
        RxLine.clear();
      }
      else {
        RxLine.append(rxVal);   // append new char to our Rx buffer
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


CBluetoothHC05::CBluetoothHC05(int keyPin, int sensePin)
{
  _keyPin = keyPin;
  _sensePin = sensePin;
}


void 
CBluetoothHC05::Init()
{
  const int BTRates[] = {
    9600, 38400, 115200, 19200, 57600, 2400, 4800, 1200
  };

  RxLine.clear();

  // attach to the SENSE line from the HC-05 module
  // this line goes high when a BT client is connected :-)
  pinMode(_sensePin, INPUT);              
  
  digitalWrite(_keyPin, HIGH);              // request HC-05 module to enter command mode
  // Open Serial2, explicitly specify pins for pin multiplexer!);   
  OpenSerial(9600); // Serial2.begin(9600, SERIAL_8N1, _rxPin, _txPin);  

  DebugPort.println("\r\n\r\nAttempting to detect HC-05 Bluetooth module...");

  int BTidx = 0;
  int maxTries =  sizeof(BTRates)/sizeof(int);
  for(BTidx = 0; BTidx < maxTries; BTidx++) {
    DebugPort.print("  @ ");
    DebugPort.print(BTRates[BTidx]);
    DebugPort.print(" baud... ");
    OpenSerial(BTRates[BTidx]); // Serial2.begin(BTRates[BTidx], SERIAL_8N1, _rxPin, _txPin);   // open serial port at a std.baud rate
    delay(10);
    Serial2.print("\r\n");      // clear the throat!
    delay(100);
    Serial2.setTimeout(100);

    if(ATCommand("AT\r\n")) {   // probe with a simple "AT"
      DebugPort.println(" OK.");          // got a response - woo hoo found the module!
      break;
    }
    if(ATCommand("AT\r\n")) {   // sometimes a second try is good...
      DebugPort.println(" OK.");
      break;
    }

    // failed, try another baud rate
    DebugPort.println("");
    Serial2.flush();
    Serial2.end();
    delay(100);
  }

  DebugPort.println("");
  if(BTidx == maxTries) {
    // we could not get anywhere with teh AT commands, but maybe this is the other module
    // plough on and assume 9600 baud, but at the mercy of whatever the module name is...
    DebugPort.println("FAILED to detect a HC-05 Bluetooth module :-(");
    // leave the EN pin high - if other style module keeps it powered!
    // assume it is 9600, and just (try to) use it like that...
    // we will sense the STATE line to prove a client is hanging off the link...
    DebugPort.println("ASSUMING a HC-05 module @ 9600baud (Unknown name)");
    OpenSerial(9600); // Serial2.begin(9600, SERIAL_8N1, _rxPin, _txPin);
  }
  else {
    // found a HC-05 module at one of its supported baud rates.
    // now program it's name and force a 9600 baud data interface.
    // this is the defacto standard as shipped!

    DebugPort.println("HC-05 found");

    do {   // so we can break!
      DebugPort.print("  Setting Name to \"Diesel Heater\"... ");
      if(!ATCommand("AT+NAME=\"Diesel Heater\"\r\n")) {
        DebugPort.println("FAILED");
      }
      else {
        DebugPort.println("OK");
      }

      DebugPort.print("  Setting baud rate to 9600N81...");
      if(!ATCommand("AT+UART=9600,1,0\r\n")) {
        DebugPort.println("FAILED");
      }
      else {
        DebugPort.println("OK");
      }

      OpenSerial(9600); // Serial2.begin(9600, SERIAL_8N1, _rxPin, _txPin);

      // leave HC-05 command mode, return to data mode
      digitalWrite(_keyPin, LOW);  
    } while (0);   // yeah lame, allows break prior though :-)
  }

  delay(50);

  DebugPort.println("");
}


void 
CBluetoothHC05::Check()
{  
  // check for data coming back over Bluetooth
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


void
CBluetoothHC05::SendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm)
{
  DebugPort.print(millis());
  DebugPort.print("ms ");
  DebugReportFrame(pHdr, Frame, "   ");

  if(digitalRead(_sensePin)) {
    if(Frame.verifyCRC()) {
      // send data frame to HC-05
      Serial2.print(pHdr);
      Serial2.write(Frame.Data, 24);
      // toggle LED
#ifdef BT_LED      
      digitalWrite(LED, !digitalRead(LED)); // toggle LED
#endif
    }
    else {
      DebugPort.print("Bluetooth data not sent, CRC error ");
    }
  }
  else {
    DebugPort.print("No Bluetooth client");
      // force LED off
#ifdef BT_LED      
    digitalWrite(LED, 0);
#endif
  }
  if(lineterm)
    DebugPort.println("");
}

// protected function, typically to perform Hayes commands with HC-05
bool 
CBluetoothHC05::ATCommand(const char* cmd)
{
  Serial2.print(cmd);
  char RxBuffer[16];
  memset(RxBuffer, 0, 16);
  int read = Serial2.readBytesUntil('\n', RxBuffer, 16);  // \n is not included in returned string!
  if((read == 3) && (0 == strcmp(RxBuffer, "OK\r")) ) {
    return true;
  }
  return false;
}

void 
CBluetoothHC05::OpenSerial(int baudrate)
{
  Serial2.begin(baudrate);
}
