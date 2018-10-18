 /*
  Chinese Heater Half Duplex Serial Data Sending Tool

  Connects to the blue wire of a Chinese heater, which is the half duplex serial link.
  Sends and receives data from serial port 1. 

  Terminology: Tx is to the heater unit, Rx is from the heater unit.
  
  Typical data frame timing on the blue wire is:
  __Tx_Rx____________________________Tx_Rx____________________________Tx_Rx___________
  
  This software can connect to the blue wire in a normal OEM system, detecting the 
  OEM controller and allowing extraction of the data or injecting on/off commands.

  If Pin 21 is grounded on the Due, this simple stream will be reported over USB and
  no control from the Arduino will be allowed.
  This allows sniffing of the blue wire in a normal system.
  
  The binary data is received from the line.
  If it has been > 100ms since the last blue wire activity this indicates a new frame 
  sequence is starting from the OEM controller.
  Synchronise as such then count off the next 24 bytes storing them in the Controller's 
  data array. These bytes are then reported over USB to the PC in ASCII.

  It is then expected the heater will respond with it's 24 bytes.
  Capture those bytes and store them in the Heater1 data array.
  Once again these bytes are then reported over USB to the PC in ASCII.

  If no activity is sensed in a second, it is assumed no controller is attached and we
  have full control over the heater.

  Either way we can now inject a message onto the blue wire allowing our custom 
  on/off control.
  We must remain synchronous with the OEM controller if it exists otherwise E-07 
  faults will be caused.

  Typical data frame timing on the blue wire is then:
  __OEMTx_HtrRx__OurTx_HtrRx____________OEMTx_HtrRx__OurTx_HtrRx____________OEMTx_HtrRx__OurTx_HtrRx_________
    
  The second HtrRx to the next OEMTx delay is always > 100ms and is paced by the OEM controller.
  The delay before seeing Heater Rx data after any Tx is usually much less than 10ms.
  But this does rise if new max/min or voltage settings are sent.
  **The heater only ever sends Rx data in response to a data frame from a controller**

  A HC-05 Bluetooth module is attached to Serial2:
  TXD -> Rx2 (pin 17)
  RXD -> Tx2 (pin 16)
  EN(key) -> pin 15
  
 
  This code only works with boards that have more than one hardware serial port like Arduino 
  Mega, Due, Zero etc.


  The circuit:
  - a Tx Rx multiplexer is required to combine the Arduino's Tx1 And Rx1 pins onto the blue wire.
  - a Tx Enable signal from pin 20 controls the multiplexer
  - Serial logging software on Serial0 via USB link

  created 23 Sep 2018 by Ray Jones

  This example code is in the public domain.
*/

#include "Protocol.h"
#include "TxManage.h"
#include "Bluetooth.h"
#include "pins.h"

#if defined(__arm__)
// for Arduino Due
static UARTClass& BlueWire(Serial1);
#else
// for ESP32, Mega
static HardwareSerial& BlueWire(Serial1);  
#endif

void SerialReport(const char* hdr, const unsigned char* pData, const char* ftr);

class CommStates {
  public:
    // comms states
    enum eCS { 
      Idle, ControllerRx, ControllerReport, HeaterRx1, HeaterReport1, BTC_Tx, HeaterRx2, HeaterReport2 
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
  bool saveData(unsigned char* pData, unsigned char val, int limit = 24) {   // returns true when buffer filled
    pData[m_Count++] = val;
    return m_Count == limit;
  }
private:
  int m_State;
  int m_Count;
};


CommStates CommState;
CTxManage TxManage(TxEnbPin, Serial1);
CProtocol OEMController;     // most recent data packet received from OEM controller found on blue wire
CProtocol Heater1;        // data packet received from heater in response to OEM controller packet
CProtocol Heater2;        // data packet received from heater in response to our packet 
CProtocol BTCParams(CProtocol::CtrlMode);  // holds our local parameters, used in case of no OEM controller
long lastRxTime;        // used to observe inter character delays


void setup() 
{
  // initialize listening serial port
  // 25000 baud, Tx and Rx channels of Chinese heater comms interface:
  // Tx/Rx data to/from heater, special baud rate for Chinese heater controllers
  pinMode(ListenOnlyPin, INPUT_PULLUP);
  pinMode(KeyPin, OUTPUT);
//  pinMode(Tx2Pin, OUTPUT);
  digitalWrite(KeyPin, LOW);
//  digitalWrite(Tx2Pin, HIGH);

#if defined(__arm__) || defined(__AVR__)
  BlueWire.begin(25000);   
  pinMode(19, INPUT_PULLUP);  // required for MUX to work properly
#else if defined(__ESP32__)
  // ESP32
#define RXD2 16
#define TXD2 17
  BlueWire.begin(25000, SERIAL_8N1, Rx1Pin, Tx1Pin);  
#endif
  
  // initialise serial monitor on serial port 0
  USB.begin(115200);
  
  // prepare for first long delay detection
  lastRxTime = millis();

  TxManage.begin(); // ensure Tx enable pin setup

  // define defaults should heater controller be missing
  BTCParams.setTemperature_Desired(23);
  BTCParams.setTemperature_Actual(22);
  BTCParams.Controller.OperatingVoltage = 120;
  BTCParams.setPump_Min(16);
  BTCParams.setPump_Max(55);
  BTCParams.setFan_Min(1680);
  BTCParams.setFan_Max(4500);

  Bluetooth_Init();
}

void loop() 
{
  unsigned long timenow = millis();

  // check for test commands received from PC Over USB
  if(USB.available()) {
    char rxval = USB.read();
    if(rxval  == '+') {
      TxManage.RequestOn();
    }
    if(rxval  == '-') {
      TxManage.RequestOff();
    }
  }

  // check for Bluetooth activity
  Bluetooth_Check();

  // Handle time interval where we send data to the blue wire
  if(CommState.is(CommStates::BTC_Tx)) {
    lastRxTime = timenow;                  // we are pumping onto blue wire, track this activity!
    if(TxManage.CheckTx(timenow) ) {       // monitor our data delivery
      CommState.set(CommStates::HeaterRx2);   // then await heater repsonse
    }
  }


  // calc elapsed time since last rxd byte to detect no other controller, or start of frame sequence
  unsigned long RxTimeElapsed = timenow - lastRxTime;

  // check for no rx traffic => no OEM controller
  if(CommState.is(CommStates::Idle) && (RxTimeElapsed >= 970)) {
    // have not seen any receive data for a second.
    // OEM controller probably not connected. 
    // Skip to BTC_Tx, sending our own settings.
    CommState.set(CommStates::BTC_Tx);
    bool bOurParams = true;
    TxManage.Start(BTCParams, timenow, bOurParams);
    Bluetooth_Report("[BTC]", BTCParams.Data);    //  BTC => Bluetooth Controller :-)
  }

  // precautionary action if all 24 bytes were not received whilst expecting them
  if(RxTimeElapsed > 50) {              
    if( CommState.is(CommStates::ControllerRx) || 
        CommState.is(CommStates::HeaterRx1) || 
        CommState.is(CommStates::HeaterRx2) ) {

      CommState.set(CommStates::Idle);
    }
  }

  // read from port 1, the "blue wire" (to/from heater), store according to CommState
  if (BlueWire.available()) {
  
    lastRxTime = timenow;

    // detect start of a new frame sequence from OEM controller
    if( CommState.is(CommStates::Idle) && (RxTimeElapsed > 100)) {       
      CommState.set(CommStates::ControllerRx);
    }
    
    int inByte = BlueWire.read(); // read hex byte

    if( CommState.is(CommStates::ControllerRx) ) {
      if(CommState.saveData(OEMController.Data, inByte) ) {
        CommState.set(CommStates::ControllerReport);
      }
    }

    else if( CommState.is(CommStates::HeaterRx1) ) {
      if( CommState.saveData(Heater1.Data, inByte) ) {
        CommState.set(CommStates::HeaterReport1);
      }
    }

    else if( CommState.is(CommStates::HeaterRx2) ) {
      if( CommState.saveData(Heater2.Data, inByte) ) {
        CommState.set(CommStates::HeaterReport2);
      }
    }  

  } // BlueWire.available


  if( CommState.is(CommStates::ControllerReport) ) {  
    // filled controller frame, report
    Bluetooth_Report("[OEM]", OEMController.Data);
    SerialReport("Ctrl  ", OEMController.Data, "  ");
    CommState.set(CommStates::HeaterRx1);
  }
    
  else if(CommState.is(CommStates::HeaterReport1) ) {
    // received heater frame (after controller message), report
    SerialReport("Htr1  ", Heater1.Data, "\r\n");
    Bluetooth_Report("[HTR]", Heater1.Data);

    if(digitalRead(ListenOnlyPin)) {
      bool bOurParams = false;
      TxManage.Start(OEMController, timenow, bOurParams);
      CommState.set(CommStates::BTC_Tx);
    }
    else {
      CommState.set(CommStates::Idle);    // "Listen Only" input held low, don't send out Tx
    }
  }
    
  else if( CommState.is(CommStates::HeaterReport2) ) {
    // received heater frame (after our control message), report
    SerialReport("Htr2  ", Heater2.Data, "\r\n");
//    if(!digitalRead(ListenOnlyPin)) {
      Bluetooth_Report("[HTR]", Heater2.Data);    // pin not grounded, suppress duplicate to BT
//    }
    CommState.set(CommStates::Idle);
  }
    
}  // loop

void SerialReport(const char* hdr, const unsigned char* pData, const char* ftr)
{
  USB.print(hdr);                     // header
  for(int i=0; i<24; i++) {
    char str[16];
    sprintf(str, "%02X ", pData[i]);  // build 2 dig hex values
    USB.print(str);                   // and print     
  }
  USB.print(ftr);                     // footer
}

