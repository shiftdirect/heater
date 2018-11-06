#include "pins.h"
#include "Protocol.h"
#include "debugport.h"
#include "BluetoothHC05.h"
#include "BTCConfig.h"

// Bluetooth access via HC-05 Module, using a UART


CBluetoothHC05::CBluetoothHC05(int keyPin, int sensePin)
{
  // extra control pins required to fully drive a HC05 module 
  _keyPin = keyPin;      // used to enable AT command mode (ONLY ON SUPPORTED MODULES!!!!)
  _sensePin = sensePin;  // feedback signal used to sense if a client is connected
  
  pinMode(_keyPin, OUTPUT);              
  digitalWrite(_keyPin, LOW);              // request HC-05 module to enter data mode
  // attach to the SENSE line from the HC-05 module
  // this line goes high when a BT client is connected :-)
  pinMode(_sensePin, INPUT);              
}


void 
CBluetoothHC05::init()
{
  const int BTRates[] = {
    9600, 38400, 115200, 19200, 57600, 2400, 4800, 1200
  };

  _rxLine.clear();

  digitalWrite(_keyPin, HIGH);              // request HC-05 module to enter command mode

  openSerial(9600); // virtual function, may call derived class method here

  DebugPort.println("\r\n\r\nAttempting to detect HC-05 Bluetooth module...");

  int BTidx = 0;
  int maxTries =  sizeof(BTRates)/sizeof(int);
  for(BTidx = 0; BTidx < maxTries; BTidx++) {
    DebugPort.print("  @ ");
    DebugPort.print(BTRates[BTidx]);
    DebugPort.print(" baud... ");
    openSerial(BTRates[BTidx]);      // open serial port at a std.baud rate
    delay(10);
    HC05_SerialPort.print("\r\n");   // clear the throat!
    delay(100);
    HC05_SerialPort.setTimeout(100);

    if(ATCommand("AT\r\n")) {        // probe with a simple "AT"
      DebugPort.println(" OK.");     // got a response - woo hoo found the module!
      break;
    }
    if(ATCommand("AT\r\n")) {        // sometimes a second try is good...
      DebugPort.println(" OK.");
      break;
    }

    // failed, try another baud rate
    DebugPort.println("");
    HC05_SerialPort.flush();
    HC05_SerialPort.end();
    delay(100);
  }

  DebugPort.println("");
  if(BTidx == maxTries) {
    // we could not get anywhere with the AT commands, but maybe this is the other module
    // plough on and assume 9600 baud, but at the mercy of whatever the module name is...
    DebugPort.println("FAILED to detect a HC-05 Bluetooth module :-(");
    // leave the EN pin high - if other style module keeps it powered!
    // assume it is 9600, and just (try to) use it like that...
    // we will sense the STATE line to prove a client is hanging off the link...
    DebugPort.println("ASSUMING a HC-05 module @ 9600baud (Unknown name)");
    openSerial(9600); 
  }
  else {
    // found a HC-05 module at one of its supported baud rates.
    // now program it's name and force a 9600 baud data interface.
    // this is the defacto standard as shipped!

    DebugPort.println("HC-05 found");

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

    openSerial(9600); 

    // leave HC-05 command mode, return to data mode
    digitalWrite(_keyPin, LOW);  
  }

  delay(50);

  DebugPort.println("");
}


void 
CBluetoothHC05::check()
{  
  // check for data coming back over Bluetooth
  if(HC05_SerialPort.available()) {           // serial rx data is available
    char rxVal = HC05_SerialPort.read();
    collectRxData(rxVal);
  }
}


void
CBluetoothHC05::sendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm)
{
  // report to debug port
  CBluetoothAbstract::sendFrame(pHdr, Frame, false);

  if(digitalRead(_sensePin)) {
    if(Frame.verifyCRC()) {
      // send data frame to HC-05
      HC05_SerialPort.print(pHdr);
      HC05_SerialPort.write(Frame.Data, 24);
      // toggle LED
#if BT_LED == 1     
      digitalWrite(LED_Pin, !digitalRead(LED_Pin)); // toggle LED
#endif
    }
    else {
      DebugPort.print("Bluetooth data not sent, CRC error ");
    }
  }
  else {
    if(lineterm) {    // only report no client if this will be at end of line (long line support)
      DebugPort.print("No Bluetooth client");
    }
      // force LED off
#if BT_LED == 1
    digitalWrite(LED_Pin, LOW);
#endif
  }
  if(lineterm)
    DebugPort.println("");
}

void 
CBluetoothHC05::openSerial(int baudrate)
{
  // standard serial port for Due, Mega (ESP32 uses virtual, derived from this class)
  HC05_SerialPort.begin(baudrate);
}

// protected function, to perform Hayes commands with HC-05
bool 
CBluetoothHC05::ATCommand(const char* cmd)
{
  HC05_SerialPort.print(cmd);
  char RxBuffer[16];
  memset(RxBuffer, 0, 16);
  int read = HC05_SerialPort.readBytesUntil('\n', RxBuffer, 16);  // \n is not included in returned string!
  if((read == 3) && (0 == strcmp(RxBuffer, "OK\r")) ) {
    return true;
  }
  return false;
}

