#include <Arduino.h>
#include "CFrame.h"

class CTxManage
{
public:
  CTxManage(int TxEnbPin, USARTClass& serial);
  void RequestOn();
  void RequestOff();
  void Copy(CFrame& ref);
  bool isBusy();
  void Tick(unsigned long timenow);
  void Send(unsigned long timenow, bool self);
  void Report();
  void begin();

private:
  CFrame m_Frame;
  bool m_bOnReq;
  bool m_bOffReq;
  bool m_bTxPending;
  bool m_bSelf;
  int  m_nTxEnbPin;
  unsigned long m_nStartTime;
  USARTClass& m_Serial;

  const int m_nStartDelay = 20;
  const int m_nFrameTime = 14;
  const int m_nFrontPorch = 2;

  void _send();
};

