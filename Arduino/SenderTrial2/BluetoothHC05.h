
#include "BluetoothAbstract.h"

// Define the serial port for access to a HC-05 module.
// This is generally Serial2, but different platforms use 
// a different class for the implementation.
#ifdef __arm__
// for Arduino Due
static UARTClass& HC05_SerialPort(Serial2);      
#else
// for Mega, ESP32
static HardwareSerial& HC05_SerialPort(Serial2); 
#endif

// define a derived class that offers bluetooth messaging over the HC-05

class CBluetoothHC05 : public CBluetoothAbstract {
  bool ATCommand(const char* str);
  int _sensePin, _keyPin;
public:
  CBluetoothHC05(int keyPin, int sensePin);
  void init();
  void sendFrame(const char* pHdr, const CProtocol& Frame, bool lineterm=true);
  void check();
protected:
  virtual void openSerial(int baudrate);
};