#include <Arduino.h>
#include "Protocol.h"

class CTxManage
{
  const int m_nStartDelay = 20;
  const int m_nFrameTime = 14;
  const int m_nFrontPorch = 2;

public:
  CTxManage(int TxGatePin, HardwareSerial& serial);
  void queueOnRequest();
  void queueOffRequest();
  void PrepareFrame(const CProtocol& Frame, bool isBTCmaster);
  void Start(unsigned long timenow);
  bool CheckTx(unsigned long timenow);
  void begin();
  const CProtocol& getFrame() const { return m_TxFrame; };

private:
  CProtocol m_TxFrame;
  bool m_bOnReq;
  bool m_bOffReq;
  bool m_bTxPending;
  int  m_nTxGatePin;
  unsigned long m_nStartTime;
  HardwareSerial& m_BlueWireSerial;

};

extern CTxManage TxManage;

