#if defined(__ESP32__)
const int TxEnbPin = 22;
#else
const int TxEnbPin = 20;
#endif
const int ListenOnlyPin = 21;
const int KeyPin = 15;
const int Tx1Pin = 18;
const int Rx1Pin = 19;
const int Tx2Pin = 16;
const int Rx2Pin = 17;

#if defined(__arm__)
// for Arduino Due
static UARTClass& USB(Serial);  // TODO: make proper ESP32 BT client
#else
// for ESP32, Mega
static HardwareSerial& USB(Serial);  // TODO: make proper ESP32 BT client
#endif
