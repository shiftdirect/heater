 
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
#include "BTCWifi.h"
#define HOST_NAME "remotedebug-sample"
#define TRIGGER_PIN 0

#define FAILEDSSID "BTCESP32"
#define FAILEDPASSWORD "thereisnospoon"

//comment this out to remove TELNET

//#define TELNET

#ifdef TELNET
#define DebugPort Debug
#endif

#ifndef TELNET
#define DebugPort DebugPort
#endif

#define DEBUG_BTRX  
#include "Bluetooth.h"

#if defined(__arm__)
// Required for Arduino Due, UARTclass is derived from HardwareSerial
static UARTClass& BlueWireSerial(Serial1);
#else
// for ESP32, Mega
// HardwareSerial is it for these boards
static HardwareSerial& BlueWireSerial(Serial1);  
#endif

class CommStates {
  public:
    // comms states
    enum eCS { 
      Idle, OEMCtrlRx, OEMCtrlReport, HeaterRx1, HeaterReport1, BTC_Tx, HeaterRx2, HeaterReport2 
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
  bool collectData(CProtocol& Frame, unsigned char val, int limit = 24) {   // returns true when buffer filled
    Frame.Data[m_Count++] = val;
    return m_Count == limit;
  }
private:
  int m_State;
  int m_Count;
};

// a class to collect a new data byte from the blue wire
class sRxData {
  bool newData;
  int  Value;
public:
  sRxData() {
    reset();
  }
  void reset() {
    newData = false;
  }
  void setValue(int value) {
    newData = true;
    Value = value;
  }
  bool available() {
    return newData;
  }
  int getValue() {
    return Value;
  }
};


CommStates CommState;
CTxManage TxManage(TxEnbPin, BlueWireSerial);
CProtocol OEMCtrlFrame;        // data packet received from heater in response to OEM controller packet
CProtocol HeaterFrame1;        // data packet received from heater in response to OEM controller packet
CProtocol HeaterFrame2;        // data packet received from heater in response to our packet 
CProtocol DefaultBTCParams(CProtocol::CtrlMode);  // defines the default parameters, used in case of no OEM controller
long lastRxTime;        // used to observe inter character delays
bool hasOEMController = false;

// setup Non Volatile storage
// this is very much hardware dependent, we can use the ESP32's FLASH
#ifdef ESP32
CESP32HeaterStorage NVStorage;
#else
CHeaterStorage NVStorage;   // dummy, for now
#endif
CHeaterStorage* pNVStorage = NULL;

void PrepareTxFrame(const CProtocol& basisFrame, CProtocol& TxFrame, bool isBTCmaster);

void setup() 
{
  initWifi(TRIGGER_PIN, FAILEDSSID, FAILEDPASSWORD);

  // initialize serial port to interact with the "blue wire"
  // 25000 baud, Tx and Rx channels of Chinese heater comms interface:
  // Tx/Rx data to/from heater, 
  // Note special baud rate for Chinese heater controllers
  pinMode(Tx2Pin, OUTPUT);
  digitalWrite(Tx2Pin, HIGH);
  pinMode(Rx2Pin, INPUT_PULLUP);
  pinMode(ListenOnlyPin, INPUT_PULLUP);
  pinMode(KeyPin, OUTPUT);
  digitalWrite(KeyPin, LOW);

#if defined(__arm__) || defined(__AVR__)
  BlueWireSerial.begin(25000);   
  pinMode(Rx1Pin, INPUT_PULLUP);  // required for MUX to work properly
#elif ESP32
  // ESP32
  BlueWireSerial.begin(25000, SERIAL_8N1, Rx1Pin, Tx1Pin);  // need to explicitly specify pins for pin multiplexer!
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

  Bluetooth_Init();
 
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
  doWiFiManager();

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

  Bluetooth_Check();    // check for Bluetooth activity

  // calc elapsed time since last rxd byte
  // used to detect no OEM controller, or the start of an OEM frame sequence
  unsigned long RxTimeElapsed = timenow - lastRxTime;

  // precautionary state machine action if all 24 bytes were not received 
  // whilst expecting a frame from the blue wire
  if(RxTimeElapsed > 50) {              
    if( CommState.is(CommStates::OEMCtrlRx) || 
        CommState.is(CommStates::HeaterRx1) ||  
        CommState.is(CommStates::HeaterRx2) ) {

      CommState.set(CommStates::Idle);  // revert to idle mode
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////
  // Blue wire data reception
  //  Reads data from the "blue wire" Serial port, (to/from heater)
  //  If an OEM controller exists we will also see it's data frames
  //
  sRxData BlueWireData;

  if (BlueWireSerial.available()) {
    // Data is avaialable, read and store it now, use it later
    // Note that if not in a recognised data receive frame state, the data 
    // will be deliberately lost!
  
    lastRxTime = timenow;    // tickle last rx time, for rx data timeout purposes
    BlueWireData.setValue(BlueWireSerial.read());  // read hex byte, store for later use
  } 

  ///////////////////////////////////////////////////////////////////////////////////////////
  // do our state machine to track the reception and delivery of blue wire data

  // we may need to transit to a OEMCtrlRx if we captured a new byte, so stand alone if here!
  if( CommState.is(CommStates::Idle)) {

    // Detect the possible start of a new frame sequence from an OEM controller
    // This will be the first activity for considerable period on the blue wire
    // The heater always responds to a controller frame, but otherwise never by itself
    if(BlueWireData.available() && (RxTimeElapsed > 100)) {  
      DebugPort.print("Re-sync'd with OEM Controller. ");
      DebugPort.print(RxTimeElapsed);
      DebugPort.println("ms Idle time.");
      hasOEMController = true;
      CommState.set(CommStates::OEMCtrlRx);   // we must add this new byte!
    }

    if(RxTimeElapsed >= 970) {
      // have not seen any receive data for a second.
      // OEM controller is probably not connected. 6
      // Skip state machine immediately to BTC_Tx, sending our own settings.
      hasOEMController = false;
      CommState.set(CommStates::BTC_Tx);
      bool isBTCmaster = true;
      TxManage.PrepareFrame(DefaultBTCParams, isBTCmaster);  // use our parameters, and mix in NV storage values
      TxManage.Start(timenow);
    }

  } // CommState::Idle

  if( CommState.is(CommStates::OEMCtrlRx) ) {
    // collect OEM controller frame
    if(BlueWireData.available()) {
      if(CommState.collectData(OEMCtrlFrame, BlueWireData.getValue()) ) {
        CommState.set(CommStates::OEMCtrlReport);  // collected 24 bytes, move on!
      }
    }
  }

  else if( CommState.is(CommStates::OEMCtrlReport) ) {  
    // filled OEM controller frame, report
    // echo received OEM controller frame over Bluetooth, using [OEM] header
    Bluetooth_SendFrame("[OEM]", OEMCtrlFrame, true);
    CommState.set(CommStates::HeaterRx1);
  }

  else if( CommState.is(CommStates::HeaterRx1) ) {
    // collect heater frame, always in response to an OEM controller frame
    if(BlueWireData.available()) {
      if( CommState.collectData(HeaterFrame1, BlueWireData.getValue()) ) {
        CommState.set(CommStates::HeaterReport1);
      }
    }
  }

  else if(CommState.is(CommStates::HeaterReport1) ) {
    // received heater frame (after controller message), report
    // echo heater reponse data to Bluetooth client
    Bluetooth_SendFrame("[HTR]", HeaterFrame1);

    if(digitalRead(ListenOnlyPin)) {
      bool isBTCmaster = false;
      TxManage.PrepareFrame(OEMCtrlFrame, isBTCmaster);  // parrot OEM parameters, but block NV modes
      TxManage.Start(timenow);
      CommState.set(CommStates::BTC_Tx);
    }
    else {
      CommState.set(CommStates::Idle);    // "Listen Only" input is  held low, don't send out Tx
    }
  }
    
  // Handle time interval where we send data to the blue wire
  else if(CommState.is(CommStates::BTC_Tx)) {
    lastRxTime = timenow;                     // *we* are pumping onto blue wire, track this activity!
    if(TxManage.CheckTx(timenow) ) {          // monitor progress of our data delivery
      CommState.set(CommStates::HeaterRx2);   // then await heater repsonse
    }
  }

  else if( CommState.is(CommStates::HeaterRx2) ) {
    // collect heater frame, in response to our control frame
    if(BlueWireData.available()) {
      if( CommState.collectData(HeaterFrame2, BlueWireData.getValue()) ) {
        CommState.set(CommStates::HeaterReport2);
      }
    } 
  }

  else if( CommState.is(CommStates::HeaterReport2) ) {
    // received heater frame (after our control message), report
    delay(5);
    if(!hasOEMController) {
      // only convey these frames to Bluetooth when NOT using an OEM controller!
      Bluetooth_SendFrame("[BTC]", TxManage.getFrame(), true);    //  BTC => Bluetooth Controller :-)
      Bluetooth_SendFrame("[HTR]", HeaterFrame2);    // pin not grounded, suppress duplicate to BT
    }
    CommState.set(CommStates::Idle);

#ifdef SHOW_HEAP
    Serial.printf("Free heap ");
    Serial.println(ESP.getFreeHeap());
#endif
  }
    
}  // loop

void DebugReportFrame(const char* hdr, const CProtocol& Frame, const char* ftr)
{
  DebugPort.print(hdr);                     // header
  for(int i=0; i<24; i++) {
    char str[16];
    sprintf(str, " %02X", Frame.Data[i]);  // build 2 dig hex values
    DebugPort.print(str);                   // and print     
  }
  DebugPort.print(ftr);                     // footer
}


void Command_Interpret(const char* pLine)
{
  unsigned char cVal;
  unsigned short sVal;

  if(strlen(pLine) == 0)
    return;
  
  #ifdef DEBUG_BTRX
    DebugPort.println(pLine);
  #endif

  if(strncmp(pLine, "[CMD]", 5) == 0) {
    // incoming command from BT app!
    DebugPort.write("  Command decode: ");

    pLine += 5;   // skip past "[CMD]" header
    if(strncmp(pLine, "ON", 2) == 0) {
      TxManage.queueOnRequest();
      DebugPort.println("Heater ON");
    }
    else if(strncmp(pLine, "OFF", 3) == 0) {
      TxManage.queueOffRequest();
      DebugPort.println("Heater OFF");
    }
    else if(strncmp(pLine, "Pmin", 4) == 0) {
      pLine += 4;
      cVal = (unsigned char)((atof(pLine) * 10.0) + 0.5);
      pNVStorage->setPmin(cVal);
      DebugPort.print("Pump min = ");
      DebugPort.println(cVal);
    }
    else if(strncmp(pLine, "Pmax", 4) == 0) {
      pLine += 4;
      cVal = (unsigned char)((atof(pLine) * 10.0) + 0.5);
      pNVStorage->setPmax(cVal);
      DebugPort.print("Pump max = ");
      DebugPort.println(cVal);
    }
    else if(strncmp(pLine, "Fmin", 4) == 0) {
      pLine += 4;
      sVal = atoi(pLine);
      pNVStorage->setFmin(sVal);
      DebugPort.print("Fan min = ");
      DebugPort.println(sVal);
    }
    else if(strncmp(pLine, "Fmax", 4) == 0) {
      pLine += 4;
      sVal = atoi(pLine);
      pNVStorage->setFmax(sVal);
      DebugPort.print("Fan max = ");
      DebugPort.println(int(sVal));
    }
    else if(strncmp(pLine, "save", 4) == 0) {
      pNVStorage->save();
      DebugPort.println("NV save");
    }
    else if(strncmp(pLine, "degC", 4) == 0) {
      pLine += 4;
      cVal = atoi(pLine);
      pNVStorage->setTemperature(cVal);
      DebugPort.print("degC = ");
      DebugPort.println(cVal);
    }
    else if(strncmp(pLine, "Mode", 4) == 0) {
      pLine += 4;
      cVal = !pNVStorage->getThermostatMode();
      pNVStorage->setThermostatMode(cVal);
      DebugPort.print("Mode now ");
      DebugPort.println(cVal ? "Thermostat" : "Fixed Hz");
    }
    else {
      DebugPort.print(pLine);
      DebugPort.println(" ????");
    }

  }
}
