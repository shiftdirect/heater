

#if defined(__arm__)
// Typically Arduino Due
static UARTClass& DebugPort(Serial);
#else
static HardwareSerial& DebugPort(Serial);   // reference Serial as DebugPort
#endif