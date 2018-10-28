#include "Protocol.h"

class CSmartError {
  unsigned char m_prevRunState;
  unsigned char m_Error;
  bool m_bInhibit;
public: 
  CSmartError();
  void reset();
  void inhibit();
  void monitor(CProtocol& heaterFrame);
  void monitor(unsigned char runstate);
  unsigned char getError();
};