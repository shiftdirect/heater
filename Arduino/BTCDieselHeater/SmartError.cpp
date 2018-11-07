#include "SmartError.h"
#include "TxManage.h"

CSmartError::CSmartError()
{
  reset();
}

void 
CSmartError::reset()
{
  m_prevRunState = 0;
  m_Error = 0;
  m_bInhibit = false;
}

// we use inhibit when we manually command the heater off during preheat
// otherwise we'll register an ignition fail event
void
CSmartError::inhibit()
{
  m_bInhibit = true;
  m_Error = 0;
}

// accept a fresh heater frame
// inpsect the advertised run state, tracking when and how it transitions out
// of preheat especially.
// abnormal transitions are registered and becoem our smart m_Error
// In addition, the hetaer frame has the ErrState updated to track the 
// smart error, providing no heater error exists!
void
CSmartError::monitor(CProtocol& heaterFrame)
{
  bool bSilent = true;
  if(heaterFrame.verifyCRC(bSilent)) {  // check but don't report dodgy frames to debug
    // only accept valid heater frames!
    monitor(heaterFrame.getRunState());
    unsigned char HtrErr = heaterFrame.getErrState();
    if((HtrErr & 0xfe) == 0)  {
      // heater is Idle or Normal running state  (E-0X + 1 in protocol!!)
      unsigned char smartErr = getError();
      if(smartErr) {
        heaterFrame.setErrState(smartErr);  // 10 = ign fail, 11 = retry
        heaterFrame.setCRC();              // changed the message, fix the CRC!
      }
    }
  }
}

// test the new run state value, comparing to previous
// detect abnormal transitions
void 
CSmartError::monitor(unsigned char newRunState)
{
  // check if moving away from heater Idle state (S0)
  // especially useful if an OEM controller exists
  if((m_prevRunState == 0) && newRunState) {
    // reset the smart error
    m_Error = 0;
    m_bInhibit = false;
  }

  if(!m_bInhibit) {
    if(m_prevRunState == 2) {   // preheat state (S2)
      if(newRunState == 4) {
        // transitioned from preheat to ignited
        // - all good!
        m_Error = 0;
      }
      else if(newRunState > 5) {
        // transitioned from preheat to post glow
        // - second ignition attempt failed, heater is shutting down
        m_Error = 11;
      }
      else if(newRunState == 3) {
        // transitioned from preheat to retry 
        // - first ignition attempt failed, heater will retry
        m_Error = 12;
      }
    }
  }

  if(m_prevRunState != newRunState) {
    // check for transition to startup 
    // - force cancellation of an on request if we generated it
    if(newRunState == 2) {
      TxManage.queueOnRequest(false);  // ensure ON request is cancelled
    }
    // check for transition to shutdown 
    // - force cancellation of an off request if we generated it
    if(newRunState == 7) {
      TxManage.queueOffRequest(false);  // ensure OFF request is cancelled
    }
  }

  m_prevRunState = newRunState;
}

// return our smart error, if it exists, as the registered code
unsigned char 
CSmartError::getError()
{
  if(m_Error) {
    return m_Error;
  }
  return 0;
}