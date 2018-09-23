#include <Arduino.h>
#include "TxManage.h"

extern void SerialReport(const char* hdr, const unsigned char* pData, const char* ftr);

CTxManage::CTxManage(int TxEnbPin, USARTClass& serial) : 
  m_Serial(serial),
  m_Frame(CProtocol::CtrlMode)
{
  m_bOnReq = false;
  m_bOffReq = false;
  m_bTxPending = false;
  m_nStartTime = 0;
  m_nTxEnbPin = TxEnbPin;
}

void CTxManage::begin()
{
  pinMode(m_nTxEnbPin, OUTPUT);
  digitalWrite(m_nTxEnbPin, LOW);
}

void
CTxManage::RequestOn()
{
  m_bOnReq = true;
}

void 
CTxManage::RequestOff()
{
  m_bOffReq = true;
}

void
CTxManage::Start(const CProtocol& ref, unsigned long timenow, bool self)
{
  m_Frame = ref;
  // 0x78 prevents the controller showing bum information when we parrot the OEM controller
  // heater is happy either way, the OEM controller has set the max/min stuff already
  m_Frame.Data[0] = self ? 0x76 : 0x78;  

  if(timenow == 0)
    timenow++;

  m_nStartTime = timenow;
  m_bTxPending = true;
}

bool
CTxManage::CheckTx(unsigned long timenow)
{
  if(m_nStartTime) {

    long diff = timenow - m_nStartTime;

    if(diff > m_nStartDelay) {
      // begin front porch of Tx gating pulse
      digitalWrite(m_nTxEnbPin, HIGH);
    }
    if(m_bTxPending && (diff > (m_nStartDelay + m_nFrontPorch))) {
      // begin serial transmission
      m_bTxPending = false;
      _send();
    }
    if(diff > (m_nStartDelay + m_nFrameTime)) {
      // conclude Tx gating
      m_nStartTime = 0;
      digitalWrite(m_nTxEnbPin, LOW);
    }
  }
  return m_nStartTime == 0;
}

void
CTxManage::_send()
{
  // install on/off commands if required
  if(m_bOnReq) {
    m_bOnReq = false;
    m_Frame.Controller.Command = 0xa0;
  }
  else if(m_bOffReq) {
    m_bOffReq = false;
    m_Frame.Controller.Command = 0x05;
  }
  else {
    m_Frame.Controller.Command = 0x00;
  }
    
  // ensure CRC valid
  m_Frame.setCRC();
    
  // send to heater - using binary 
  for(int i=0; i<24; i++) {
    m_Serial.write(m_Frame.Data[i]);                  // write native binary values
  }

  Report();
}

void
CTxManage::Report()
{
  SerialReport("Self  ", m_Frame.Data, "  ");  
}
