#include <Arduino.h>
#include "Protocol.h"

class CTxManage
{
  const int m_nStartDelay = 20;
  const int m_nFrameTime = 14;
  const int m_nFrontPorch = 2;

public:
  CTxManage(int TxEnbPin, USARTClass& serial);
  void RequestOn();
  void RequestOff();
  void Start(const CProtocol& ref, unsigned long timenow, bool self);
  bool CheckTx(unsigned long timenow);
  void Report();
  void begin();

private:
  CProtocol m_Frame;
  bool m_bOnReq;
  bool m_bOffReq;
  bool m_bTxPending;
  int  m_nTxEnbPin;
  unsigned long m_nStartTime;
  USARTClass& m_Serial;

  void _send();
};

