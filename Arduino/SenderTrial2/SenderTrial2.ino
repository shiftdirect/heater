 /*
  Chinese Heater Half Duplex Serial Data Send Test Tool

  Connects to the blue wire of a Chinese heater, which is the half duplex serial link.
  Receives data from serial port 1. 
  
  Terminology: Tx is to the heater unit, Rx is from the heater unit.
  
  The binary data is received from the line.
  If it has been > 100ms since the last activity this indicates a new frame 
  sequence is starting, synchronise as such then count off the next 48 bytes 
  storing them in the Data array. 

  The "outer loop" then detects the count of 48 and packages the data to be sent 
  over Serial to the USB attached PC.

  Typical data frame timing on the blue wire is:
  __Tx_Rx____________________________Tx_Rx____________________________Tx_Rx___________
    
  Rx to next Tx delay is always > 100ms and is paced by the controller.
  The delay before seeing Rx data after Tx is usually much less than 10ms.
  **The heater only ever sends Rx data in response to a data frame from the controller**
  
  Resultant data is tagged and sent out on serial port 0 (the default debug port), 
  along with a timestamp for relative timing.
  
  This example works only with boards with more than one serial port like Arduino 
  Mega, Due, Zero etc.

  The circuit:
  - Blue wire connected to Serial 1 Rx input - preferably with series 680ohm resistor.
  - Serial logging software on Serial port 0 via USB link

  created 24 Aug 2018 by Ray Jones

  modified 25 Aug by Ray Jones
    - simplifed to read 48 bytes, synchronised by observing a long pause 
      between characters. The heater only sends when prompted.
      No longer need to discrimate which packet of data would be present.

  This example code is in the public domain.
*/

#include "CFrame.h"
#include "TxManage.h"

void SerialReport(const char* hdr, const unsigned char* pData, const char* ftr);

class CommStates {
  public:
    enum eCS { 
      Idle, CtrlRx, CtrlRpt, HtrRx1, HtrRpt1, SelfTx, HtrRx2, HtrRpt2 
    };
  CommStates() {
    set(Idle);
  }
  void set(eCS eState) {
    m_State = eState;
    m_Count = 0;
  }
  bool is(eCS eState) {
    return m_State == eState;
  }
  bool rxData(unsigned char* pData, unsigned char val, int limit = 24) {   // return true if buffer filled
    pData[m_Count++] = val;
    return m_Count == limit;
  }
private:
  int m_State;
  int m_Count;
};

const int TxEnbPin = 17;
CommStates CommState;
CFrame Controller(CFrame::TxMode);
CFrame TxFrame(CFrame::TxMode);
CTxManage TxManage(TxEnbPin, Serial1);
CFrame Heater1;
CFrame Heater2;
long lastRxTime;        // used to calculate inter character delay
long TxEnbTime;       // used to reset TxEnb low
bool bOnEvent = false;
bool bOffEvent = false;


void setup() 
{
  // initialize listening serial port
  // 25000 baud, Tx and Rx channels of Chinese heater comms interface:
  // Tx/Rx data to/from heater, special baud rate for Chinese heater controllers
  Serial1.begin(25000);   
  pinMode(19, INPUT_PULLUP);
  
  // initialise serial monitor on serial port 0
  Serial.begin(115200);
  
  // prepare for first long delay detection
  lastRxTime = millis();

  TxManage.begin();

}

void loop() 
{
  unsigned long timenow = millis();

  // check for test commands received from PC Over USB
  if(Serial.available()) {
    char rxval = Serial.read();
    if(rxval  == '+') {
      TxManage.RequestOn();
    }
    if(rxval  == '-') {
      TxManage.RequestOff();
    }
  }

  if(CommState.is(CommStates::SelfTx)) {
    // Interval where we should send data to the blue wire
    lastRxTime = timenow;               // not expecting rx data, but we are pumping onto blue wire!
    TxManage.Tick(timenow);             // keep trying to send our data 
    if(!TxManage.isBusy()) {            // until completed
      CommState.set(CommStates::HtrRx2);   // then await heater repsonse
    }
  }


  // calc elapsed time since last rxd byte to detect no other controller, or start of frame sequence
  unsigned long RxTimeElapsed = timenow - lastRxTime;

  // check for no rx traffic => no OEM controller
  if(CommState.is(CommStates::Idle) && (RxTimeElapsed >= 970)) {
    // have not seen any receive data for a second.
    // OEM controller probably not connected. 
    // Skip to SelfTx, sending our own settings.
    CommState.set(CommStates::SelfTx);
    bool bSelfParams = true;
    TxManage.Send(timenow, bSelfParams);
  }

  // precaution if 24 bytes were not received whilst expecting them
  if(RxTimeElapsed > 50) {              
    if( CommState.is(CommStates::CtrlRx) || 
        CommState.is(CommStates::HtrRx1) || 
        CommState.is(CommStates::HtrRx2) ) {

      CommState.set(CommStates::Idle);
    }
  }

  // read from port 1, the "blue wire" (to/from heater), store according to CommState
  if (Serial1.available()) {
  
    lastRxTime = timenow;

    if( CommState.is(CommStates::Idle) && (RxTimeElapsed > 100)) {       // this indicates the start of a new frame sequence from another controller
      CommState.set(CommStates::CtrlRx);
    }
    
    int inByte = Serial1.read(); // read hex byte

    if( CommState.is(CommStates::CtrlRx) ) {
      if(CommState.rxData(Controller.Data, inByte) ) {
        CommState.set(CommStates::CtrlRpt);
      }
    }

    else if( CommState.is(CommStates::HtrRx1) ) {
      if( CommState.rxData(Heater1.Data, inByte) ) {
        CommState.set(CommStates::HtrRpt1);
      }
    }

    else if( CommState.is(CommStates::HtrRx2) ) {
      if( CommState.rxData(Heater2.Data, inByte) ) {
        CommState.set(CommStates::HtrRpt2);
      }
    }  

  } // Serial1.available


  if( CommState.is(CommStates::CtrlRpt) ) {  
    // filled controller frame, report
    SerialReport("Ctrl  ", Controller.Data, "  ");
    CommState.set(CommStates::HtrRx1);
  }
    
  else if(CommState.is(CommStates::HtrRpt1) ) {
    // received heater frame (after controller message), report
    SerialReport("Htr1  ", Heater1.Data, "\r\n");

    TxManage.Copy(Controller);  // replicate last obtained controller data
    TxManage.Send(timenow, false);
    CommState.set(CommStates::SelfTx);
  }
    
  else if( CommState.is(CommStates::HtrRpt2) ) {
    // received heater frame (after our control message), report
  
    SerialReport("Htr2  ", Heater2.Data, "\r\n");

    CommState.set(CommStates::Idle);
  }
    
}  // loop

void SerialReport(const char* hdr, const unsigned char* pData, const char* ftr)
{
  Serial.print(hdr);                 // header
  for(int i=0; i<24; i++) {
    char str[16];
    sprintf(str, "%02X ", pData[i]); // build 2 dig hex values
    Serial.print(str);               // and print     
  }
  Serial.print(ftr);                 // footer
}