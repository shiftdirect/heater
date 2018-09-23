#include <Arduino.h>
#include "TxManage.h"

extern void SerialReport(const char* hdr, const unsigned char* pData, const char* ftr);

CTxManage::CTxManage(int TxEnbPin, USARTClass& serial) : 
  m_Serial(serial),
  m_Frame(CFrame::TxMode)
{
  m_bOnReq = false;
  m_bOffReq = false;
  m_bTxPending = false;
  m_bSelf = true;
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
CTxManage::Copy(CFrame& ref)
{
  m_Frame = ref;
}

bool
CTxManage::isBusy()
{
  return m_nStartTime != 0;
}

void
CTxManage::Send(unsigned long timenow, bool self)
{
  if(timenow == 0)
    timenow++;

  m_nStartTime = timenow;
  m_bSelf = self;
  m_bTxPending = true;
}

void
CTxManage::Tick(unsigned long timenow)
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
}

void
CTxManage::_send()
{
  if(m_bSelf) {
    m_Frame.Data[0] = 0x76;   // required for heater to use the max min information
    m_Frame.Data[1] = 0x16;
    m_Frame.setTemperature_Desired(35);
    m_Frame.setTemperature_Actual(22);
    m_Frame.Tx.OperatingVoltage = 120;
    m_Frame.setPump_Min(16);
    m_Frame.setPump_Max(55);
    m_Frame.setFan_Min(1680);
    m_Frame.setFan_Max(4500);
  }
  else {
    m_Frame.Data[0] = 0x78;  // this prevents the controller trying to show bum information, heater uses controller max/min settings
  }

  if(m_bOnReq) {
    m_bOnReq = false;
    m_Frame.Tx.Command = 0xa0;
  }
  else if(m_bOffReq) {
    m_bOffReq = false;
    m_Frame.Tx.Command = 0x05;
  }
  else {
    m_Frame.Tx.Command = 0x00;
  }
    
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
