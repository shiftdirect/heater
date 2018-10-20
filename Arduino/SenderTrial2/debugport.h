
class CProtocol;

#if defined(__arm__)
// Typically Arduino Due
static UARTClass& DebugPort(Serial);
#else
static HardwareSerial& DebugPort(Serial);   // reference Serial as DebugPort
#endif

void DebugReportFrame(const char* hdr, const CProtocol& Frame, const char* ftr);

