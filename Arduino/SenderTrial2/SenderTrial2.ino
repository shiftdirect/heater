 /*
  Chinese Heater Half Duplex Serial Data Sending Tool

  Connects to the blue wire of a Chinese heater, which is the half duplex serial link.
  Sends and receives data from hardware serial port 1. 

  Terminology: Tx is to the heater unit, Rx is from the heater unit.
  
  Typical data frame timing on the blue wire is:
  __Tx_Rx____________________________Tx_Rx____________________________Tx_Rx___________
  
  This software can connect to the blue wire in a normal OEM system, detecting the 
  OEM controller and allowing extraction of the data or injecting on/off commands.

  If Pin 21 is grounded on the Due, this simple stream will be reported over Serial and
  no control from the Arduino will be allowed.
  This allows sniffing of the blue wire in a normal system.
  
  The binary data is received from the line.
  If it has been > 100ms since the last blue wire activity this indicates a new frame 
  sequence is starting from the OEM controller.
  Synchronise as such then count off the next 24 bytes storing them in the Controller's 
  data array. These bytes are then reported over Serial to the PC in ASCII.

  It is then expected the heater will respond with it's 24 bytes.
  Capture those bytes and store them in the Heater1 data array.
  Once again these bytes are then reported over Serial to the PC in ASCII.

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
#include "pins.h"
#include "NVStorage.h"
#include "debugport.h"

#define BLUETOOTH
#define DEBUG_BTRX
  
#ifdef BLUETOOTH
#include "Bluetooth.h"
#endif

#if defined(__arm__)
// Required for Arduino Due, UARTclass is derived from HardwareSerial
static UARTClass& BlueWire(Serial1);
#else
// for ESP32, Mega
// HardwareSerial is it for these boards
static HardwareSerial& BlueWire(Serial1);  
#endif

void DebugReportFrame(const char* hdr, const CProtocol& Frame, const char* ftr);

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
CTxManage TxManage(TxEnbPin, BlueWire);
CProtocol OEMControllerFrame;        // data packet received from heater in response to OEM controller packet
CProtocol HeaterFrame1;        // data packet received from heater in response to OEM controller packet
CProtocol HeaterFrame2;        // data packet received from heater in response to our packet 
CProtocol DefaultBTCParams(CProtocol::CtrlMode);  // defines the default parameters, used in case of no OEM controller
long lastRxTime;        // used to observe inter character delays

// setup Non Volatile storage
// this is very much hardware dependent, we can use the ESP32's FLASH
#ifdef ESP32
CESP32HeaterStorage NVStorage;
#else
CHeaterStorage NVStorage;   // dummy, for now
#endif
CHeaterStorage* pNVStorage = NULL;


void setup() 
{
  // initialize serial port to interact with the "blue wire"
  // 25000 baud, Tx and Rx channels of Chinese heater comms interface:
  // Tx/Rx data to/from heater, 
  // Note special baud rate for Chinese heater controllers
  pinMode(ListenOnlyPin, INPUT_PULLUP);
  pinMode(KeyPin, OUTPUT);
  digitalWrite(KeyPin, LOW);

#if defined(__arm__) || defined(__AVR__)
  BlueWire.begin(25000);   
  pinMode(Rx1Pin, INPUT_PULLUP);  // required for MUX to work properly
#elif ESP32
  // ESP32
  BlueWire.begin(25000, SERIAL_8N1, Rx1Pin, Tx1Pin);  // need to explicitly specify pins for pin multiplexer!
  pinMode(Rx1Pin, INPUT_PULLUP);  // required for MUX to work properly
#endif
  
  // initialise serial monitor on serial port 0
  // this is the usual USB connection to a PC
  DebugPort.begin(115200);
  
  // prepare for first long delay detection
  lastRxTime = millis();

  TxManage.begin(); // ensure Tx enable pin is setup

  // define defaults should heater controller be missing
  DefaultBTCParams.setTemperature_Desired(23);
  DefaultBTCParams.setTemperature_Actual(22);
  DefaultBTCParams.Controller.OperatingVoltage = 120;
  DefaultBTCParams.setPump_Min(16);
  DefaultBTCParams.setPump_Max(55);
  DefaultBTCParams.setFan_Min(1680);
  DefaultBTCParams.setFan_Max(4500);

#ifdef BLUETOOTH
  Bluetooth_Init();
#endif
 
  // create pointer to CHeaterStorage
  // via the magic of polymorphism we can use this to access whatever 
  // storage is required for a specifc platform in a uniform way
  pNVStorage = &NVStorage;
  pNVStorage->init();
  pNVStorage->load();
}

// main functional loop is based about a state machine approach, waiting for data 
// to appear upon the blue wire, and marshalling into an appropriate receive buffer
// according to the state.


void loop() 
{
  unsigned long timenow = millis();

  // check for test commands received from PC Over USB
  if(DebugPort.available()) {
    char rxval = DebugPort.read();
    if(rxval  == '+') {
      TxManage.queueOnRequest();
    }
    if(rxval  == '-') {
      TxManage.queueOffRequest();
    }
  }

#ifdef BLUETOOTH
  Bluetooth_Check();    // check for Bluetooth activity
#endif

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
    // OEM controller is probably not connected. 
    // Skip state machine immediately to BTC_Tx, sending our own settings.
    CommState.set(CommStates::BTC_Tx);
    bool isBTCmaster = true;
    TxManage.PrepareFrame(DefaultBTCParams, isBTCmaster);  // use our parameters, and mix in NV storage values
    TxManage.Start(timenow);
#ifdef BLUETOOTH
    Bluetooth_SendFrame("[BTC]", TxManage.getFrame());    //  BTC => Bluetooth Controller :-)
#endif
  }

  // precautionary action if all 24 bytes were not received whilst expecting them
  if(RxTimeElapsed > 50) {              
    if( CommState.is(CommStates::ControllerRx) || 
        CommState.is(CommStates::HeaterRx1) ||  
        CommState.is(CommStates::HeaterRx2) ) {

      CommState.set(CommStates::Idle);
    }
  }

  // read data from Serial port 1, the "blue wire" (to/from heater), store according to CommState
  if (BlueWire.available()) {
  
    lastRxTime = timenow;

    // detect start of a new frame sequence from OEM controller
    if( CommState.is(CommStates::Idle) && (RxTimeElapsed > 100)) {       
      CommState.set(CommStates::ControllerRx);
    }
    
    int inByte = BlueWire.read(); // read hex byte

    if( CommState.is(CommStates::ControllerRx) ) {
      if(CommState.saveData(OEMControllerFrame.Data, inByte) ) {
        CommState.set(CommStates::ControllerReport);
      }
    }

    else if( CommState.is(CommStates::HeaterRx1) ) {
      if( CommState.saveData(HeaterFrame1.Data, inByte) ) {
        CommState.set(CommStates::HeaterReport1);
      }
    }

    else if( CommState.is(CommStates::HeaterRx2) ) {
      if( CommState.saveData(HeaterFrame2.Data, inByte) ) {
        CommState.set(CommStates::HeaterReport2);
      }
    }  

  } // BlueWire.available


  if( CommState.is(CommStates::ControllerReport) ) {  
    // filled controller frame, report
#ifdef BLUETOOTH
    // echo received OEM controller frame over Bluetooth, using [OEM] header
    Bluetooth_SendFrame("[OEM]", OEMControllerFrame);
#endif
    DebugReportFrame("OEM  ", OEMControllerFrame, "  ");
    CommState.set(CommStates::HeaterRx1);
  }
    
  else if(CommState.is(CommStates::HeaterReport1) ) {
    // received heater frame (after controller message), report
    DebugReportFrame("Htr1  ", HeaterFrame1, "\r\n");
#ifdef BLUETOOTH
    // echo heater reponse data to Bluetooth client
    Bluetooth_SendFrame("[HTR]", HeaterFrame1);
#endif

    if(digitalRead(ListenOnlyPin)) {
      bool isBTCmaster = false;
      TxManage.PrepareFrame(OEMControllerFrame, isBTCmaster);  // parrot OEM parameters, but block NV modes
      TxManage.Start(timenow);
      CommState.set(CommStates::BTC_Tx);
    }
    else {
      CommState.set(CommStates::Idle);    // "Listen Only" input is  held low, don't send out Tx
    }
  }
    
  else if( CommState.is(CommStates::HeaterReport2) ) {
    // received heater frame (after our control message), report
    DebugReportFrame("Htr2  ", HeaterFrame2, "\r\n");
//    if(!digitalRead(ListenOnlyPin)) {
#ifdef BLUETOOTH
      Bluetooth_SendFrame("[HTR]", HeaterFrame2);    // pin not grounded, suppress duplicate to BT
#endif
//    }
    CommState.set(CommStates::Idle);
  }
    
}  // loop

void DebugReportFrame(const char* hdr, const CProtocol& Frame, const char* ftr)
{
  DebugPort.print(hdr);                     // header
  for(int i=0; i<24; i++) {
    char str[16];
    sprintf(str, "%02X ", Frame.Data[i]);  // build 2 dig hex values
    DebugPort.print(str);                   // and print     
  }
  DebugPort.print(ftr);                     // footer
}

void Command_Interpret(String line)
{
  unsigned char cVal;
  unsigned short sVal;
  
  #ifdef DEBUG_BTRX
    DebugPort.println(line);
    DebugPort.println();
  #endif

  if(line.startsWith("[CMD]") ) {
    DebugPort.write("BT command Rx'd: ");
    // incoming command from BT app!
    line.remove(0, 5);   // strip away "[CMD]" header
    if(line.startsWith("ON") ) {
      DebugPort.write("ON\n");
      TxManage.queueOnRequest();
    }
    else if(line.startsWith("OFF")) {
      DebugPort.write("OFF\n");
      TxManage.queueOffRequest();
    }
    else if(line.startsWith("Pmin")) {
      line.remove(0, 4);
      DebugPort.write("Pmin=");
      cVal = (line.toFloat() * 10) + 0.5;
      DebugPort.println(cVal);
      pNVStorage->setPmin(cVal);
    }
    else if(line.startsWith("Pmax")) {
      line.remove(0, 4);
      DebugPort.write("Pmax=");
      cVal = (line.toFloat() * 10) + 0.5;
      DebugPort.println(cVal);
      pNVStorage->setPmax(cVal);
    }
    else if(line.startsWith("Fmin")) {
      line.remove(0, 4);
      DebugPort.print("Fmin=");
      sVal = line.toInt();
      DebugPort.println(sVal);
      pNVStorage->setFmin(sVal);
    }
    else if(line.startsWith("Fmax")) {
      line.remove(0, 4);
      DebugPort.print("Fmax=");
      sVal = line.toInt();
      DebugPort.println(sVal);
      pNVStorage->setFmax(sVal);
    }
    else if(line.startsWith("save")) {
      line.remove(0, 4);
      DebugPort.write("save\n");
      pNVStorage->save();
    }
    else if(line.startsWith("degC")) {
      line.remove(0, 4);
      DebugPort.write("degC=");
      cVal = line.toInt();
      DebugPort.println(cVal);
      pNVStorage->setTemperature(cVal);
    }
    else if(line.startsWith("Mode")) {
      line.remove(0, 4);
      DebugPort.write("Mode=");
      cVal = !pNVStorage->getThermostatMode();
      pNVStorage->setThermostatMode(cVal);
      DebugPort.println(cVal);
    }

  }
}

