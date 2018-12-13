/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2018  Ray Jones <ray@mrjones.id.au>
 * Copyright (C) 2018  James Clark
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */

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
  This allows passive sniffing of the blue wire in a normal system.
  
  The binary data is received from the line.
  If it has been > 100ms since the last blue wire activity this indicates a new frame 
  sequence is starting from the OEM controller.
  Synchronise as such then count off the next 24 bytes storing them in the Controller's 
  data array. These bytes are then reported over Serial to the PC in ASCII.

  It is then expected the heater will respond with it's 24 bytes.
  Capture those bytes and store them in the Heater1 data array.
  Once again these bytes are then reported over Serial to the PC in ASCII.

  If no activity is sensed in a second, it is assumed no OEM controller is attached and we
  have full control over the heater.

  Either way we can now inject a message onto the blue wire allowing our custom 
  on/off control.
  We must remain synchronous with an OEM controller if it exists otherwise E-07 
  faults will be caused.

  Typical data frame timing on the blue wire is then:
  __OEMTx_HtrRx__OurTx_HtrRx____________OEMTx_HtrRx__OurTx_HtrRx____________OEMTx_HtrRx__OurTx_HtrRx_________
    
  The second HtrRx to the next OEMTx delay is always > 100ms and is paced by the OEM controller.
  The delay before seeing Heater Rx data after any Tx is usually much less than 10ms.
  But this does rise if new max/min or voltage settings are sent.
  **The heater only ever sends Rx data in response to a data frame from a controller**

  For Bluetooth connectivity, a HC-05 Bluetooth module is attached to Serial2:
  TXD -> Rx2 (pin 17)
  RXD -> Tx2 (pin 16)
  EN(key) -> pin 15
  STATE -> pin 4
  
 
  This code only works with boards that have more than one hardware serial port like Arduino 
  Mega, Due, Zero, ESP32 etc.


  The circuit:
  - a Tx Rx multiplexer is required to combine the Arduino's Tx1 And Rx1 pins onto the blue wire.
  - a Tx Enable signal from pin 22 controls the multiplexer, high for Tx, low for Rx
  - Serial logging software on Serial0 via USB link

  created 23 Sep 2018 by Ray Jones

  This example code is in the public domain.
*/

#include "BTCWebServer.h"
#include "Protocol.h"
#include "TxManage.h"
#include "pins.h"
#include "NVStorage.h"
#include "debugport.h"
#include "SmartError.h"
#include "BTCWifi.h"
#include "BTCConfig.h"
#include "UtilClasses.h"
#include "BTCota.h"
#include "BTCWebServer.h"
#include "ScreenManager.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "keypad.h"
#include "helpers.h"
#include "Wire.h"
#include "Clock.h"

#define FAILEDSSID "BTCESP32"
#define FAILEDPASSWORD "thereisnospoon"

#define RX_DATA_TIMOUT 50



#ifdef ESP32
#include "BluetoothESP32.h"
#else
#include "BluetoothHC05.h"
#endif

// Setup Serial Port Definitions
#if defined(__arm__)
// Required for Arduino Due, UARTclass is derived from HardwareSerial
static UARTClass& BlueWireSerial(Serial1);
#else
// for ESP32, Mega
// HardwareSerial is it for these boards
static HardwareSerial& BlueWireSerial(Serial1);  
#endif

void initBlueWireSerial();
bool validateFrame(const CProtocol& frame, const char* name);
void checkDisplayUpdate();
void checkDebugCommands();

// DS18B20 temperature sensor support
OneWire  ds(DS18B20_Pin);  // on pin 5 (a 4.7K resistor is necessary)
DallasTemperature TempSensor(&ds);
long lastTemperatureTime;            // used to moderate DS18B20 access
float fFilteredTemperature = -100;   // -100: force direct update uopn first pass
const float fAlpha = 0.95;           // exponential mean alpha

unsigned long lastAnimationTime;     // used to sequence updates to LCD for animation

CommStates CommState;
CTxManage TxManage(TxEnbPin, BlueWireSerial);
CModeratedFrame OEMCtrlFrame;        // data packet received from heater in response to OEM controller packet
CModeratedFrame HeaterFrame1;        // data packet received from heater in response to OEM controller packet
CProtocol HeaterFrame2;              // data packet received from heater in response to our packet 
CProtocol DefaultBTCParams(CProtocol::CtrlMode);  // defines the default parameters, used in case of no OEM controller
CSmartError SmartError;
CKeyPad KeyPad;
CScreenManager ScreenManager;
TelnetSpy DebugPort;


sRxLine PCline;
long lastRxTime;                     // used to observe inter character delays
bool hasOEMController = false;

CProtocolPackage HeaterData;

unsigned long moderator;
bool bUpdateDisplay = false;
bool bHaveWebClient = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////
//               Bluetooth instantiation
//
#ifdef ESP32

// Bluetooth options for ESP32
#if USE_HC05_BLUETOOTH == 1
CBluetoothESP32HC05 Bluetooth(HC05_KeyPin, HC05_SensePin, Rx2Pin, Tx2Pin); // Instantiate ESP32 using a HC-05
#elif USE_BLE_BLUETOOTH == 1
CBluetoothESP32BLE Bluetooth;              // Instantiate ESP32 BLE server
#elif USE_CLASSIC_BLUETOOTH == 1
CBluetoothESP32Classic Bluetooth;          // Instantiate ESP32 Classic Bluetooth server
#else   // none selected
CBluetoothAbstract Bluetooth;           // default no bluetooth support - empty shell
#endif  

#else   //  !ESP32

// Bluetooth for boards other than ESP32
#if USE_HC05_BLUETOOTH == 1
CBluetoothHC05 Bluetooth(HC05_KeyPin, HC05_SensePin);  // Instantiate a HC-05
#else   // none selected  
CBluetoothAbstract Bluetooth;           // default no bluetooth support - empty shell
#endif  // closing USE_HC05_BLUETOOTH

#endif  // closing ESP32
//
//                 END Bluetooth instantiation
////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup Non Volatile storage
// this is very much hardware dependent, we can use the ESP32's FLASH
//
#ifdef ESP32
CESP32HeaterStorage actualNVstore;
#else
CHeaterStorage actualNVstore;   // dummy, for now
#endif

  // create reference to CHeaterStorage
  // via the magic of polymorphism we can use this to access whatever 
  // storage is required for a specific platform in a uniform way
CHeaterStorage& NVstore = actualNVstore;

//
////////////////////////////////////////////////////////////////////////////////////////////////////////

CBluetoothAbstract& getBluetoothClient() 
{
  return Bluetooth;
}

// callback function for Keypad events.
// must be an absolute function, cannot be a class member due the "this" element!
void parentKeyHandler(uint8_t event) 
{
  ScreenManager.keyHandler(event);   // call into the Screen Manager
}

void setup() {

  // initialise TelnetSpy (port 23) as well as Serial to 115200 
  // Serial is the usual USB connection to a PC
  // DO THIS BEFORE WE TRY AND SEND DEBUG INFO!
  
  DebugPort.setWelcomeMsg("*************************************************\r\n"
                          "* Connected to BTC heater controller debug port *\r\n"
                          "*************************************************\r\n");
  DebugPort.setBufferSize(8192);
  DebugPort.begin(115200);


  NVstore.init();
  NVstore.load();

  KeyPad.begin(keyLeft_pin, keyRight_pin, keyCentre_pin, keyUp_pin, keyDown_pin);
  KeyPad.setCallback(parentKeyHandler);

  // Initialize the rtc object
  Clock.begin();
  
  // initialise DS18B20 temperature sensor(s)
  TempSensor.begin();
  TempSensor.setWaitForConversion(false);
  TempSensor.requestTemperatures();
  lastTemperatureTime = millis();
  lastAnimationTime = millis();
  
  ScreenManager.begin();

#if USE_WIFI == 1

  initWifi(WiFi_TriggerPin, FAILEDSSID, FAILEDPASSWORD);
#if USE_OTA == 1
  initOTA();
#endif // USE_OTA
#if USE_WEBSERVER == 1
  initWebServer();
#endif // USE_WEBSERVER

#endif // USE_WIFI

  pinMode(ListenOnlyPin, INPUT_PULLUP);   // pin to enable passive mode
  pinMode(LED_Pin, OUTPUT);               // On board LED indicator
  digitalWrite(LED_Pin, LOW);

  initBlueWireSerial();
  
  // prepare for first long delay detection
  lastRxTime = millis();

  TxManage.begin(); // ensure Tx enable pin is setup

  // define defaults should OEM controller be missing
  DefaultBTCParams.setTemperature_Desired(23);
  DefaultBTCParams.setTemperature_Actual(22);
  DefaultBTCParams.Controller.OperatingVoltage = 120;
  DefaultBTCParams.setPump_Min(1.6f);
  DefaultBTCParams.setPump_Max(5.5f);
  DefaultBTCParams.setFan_Min(1680);
  DefaultBTCParams.setFan_Max(4500);

  Bluetooth.begin();
 
}



// main functional loop is based about a state machine approach, waiting for data 
// to appear upon the blue wire, and marshalling into an appropriate receive buffers
// according to the state.

void loop() 
{

  float fTemperature;
  unsigned long timenow = millis();

  DebugPort.handle();    // keep telnet spy alive

#if USE_WIFI == 1

  doWiFiManager();
#if USE_OTA == 1
  DoOTA();
#endif // USE_OTA 
#if USE_WEBSERVER == 1
  bHaveWebClient = doWebServer();
#endif //USE_WEBSERVER

#endif // USE_WIFI

  checkDebugCommands();

  KeyPad.update();      // scan keypad - key presses handler via callback functions!

  Bluetooth.check();    // check for Bluetooth activity


  //////////////////////////////////////////////////////////////////////////////////////
  // Blue wire data reception
  //  Reads data from the "blue wire" Serial port, (to/from heater)
  //  If an OEM controller exists we will also see it's data frames
  //  Note that the data is read now, then held for later use in the state machine
  //
  sRxData BlueWireData;

  if (BlueWireSerial.available()) {
    // Data is avaialable, read and store it now, use it later
    // Note that if not in a recognised data receive frame state, the data 
    // will be deliberately lost!
    BlueWireData.setValue(BlueWireSerial.read());  // read hex byte, store for later use
      
    lastRxTime = timenow;    // tickle last rx time, for rx data timeout purposes
  } 


  // calc elapsed time since last rxd byte
  // used to detect no OEM controller, or the start of an OEM frame sequence
  unsigned long RxTimeElapsed = timenow - lastRxTime;

  // precautionary state machine action if all 24 bytes were not received 
  // whilst expecting a frame from the blue wire
  if(RxTimeElapsed > RX_DATA_TIMOUT) {              
    if( CommState.is(CommStates::OEMCtrlRx) || 
        CommState.is(CommStates::HeaterRx1) ||  
        CommState.is(CommStates::HeaterRx2) ) {

      if(RxTimeElapsed >= moderator) {
        moderator += 10;
        DebugPort.print(RxTimeElapsed);
        DebugPort.print("ms - ");
        if(CommState.is(CommStates::OEMCtrlRx)) {
          DebugPort.println("Timeout collecting OEM controller data, returning to Idle State");
        }
        else if(CommState.is(CommStates::HeaterRx1)) {
          DebugPort.println("Timeout collecting OEM heater response data, returning to Idle State");
        }
        else {
          DebugPort.println("Timeout collecting BTC heater response data, returning to Idle State");
        }
      }

      DebugPort.println("\007Recycling blue wire serial interface");
      initBlueWireSerial();
      CommState.set(CommStates::Idle);  // revert to idle mode
    }
  }

  ///////////////////////////////////////////////////////////////////////////////////////////
  // do our state machine to track the reception and delivery of blue wire data

  long tDelta;
  switch(CommState.get()) {

    case CommStates::Idle:

      moderator = 50;

#if RX_LED == 1
      digitalWrite(LED_Pin, LOW);
#endif
      // Detect the possible start of a new frame sequence from an OEM controller
      // This will be the first activity for considerable period on the blue wire
      // The heater always responds to a controller frame, but otherwise never by itself
      if(RxTimeElapsed >= 970) {
        // have not seen any receive data for a second.
        // OEM controller is probably not connected. 
        // Skip state machine immediately to BTC_Tx, sending our own settings.
        hasOEMController = false;
        bool isBTCmaster = true;
        TxManage.PrepareFrame(DefaultBTCParams, isBTCmaster);  // use our parameters, and mix in NV storage values
        TxManage.Start(timenow);
        CommState.set(CommStates::BTC_Tx);
        break;
      } 

#if SUPPORT_OEM_CONTROLLER == 1
      if(BlueWireData.available() && (RxTimeElapsed > RX_DATA_TIMOUT+10)) {  
#ifdef REPORT_OEM_RESYNC
        DebugPort.print("Re-sync'd with OEM Controller. ");
        DebugPort.print(RxTimeElapsed);
        DebugPort.println("ms Idle time.");
#endif
        hasOEMController = true;
        CommState.set(CommStates::OEMCtrlRx);   // we must add this new byte!
        //
        //  ** IMPORTANT - we must drop through to OEMCtrlRx *NOW* (skipping break) **
        //
      }
      else {
        Clock.update();
        checkDisplayUpdate();    
        break;  // only break if we fail all Idle state tests
      }
#else
      Clock.update();
      checkDisplayUpdate();
      break;  
#endif


    case CommStates::OEMCtrlRx:

#if RX_LED == 1
    digitalWrite(LED_Pin, HIGH);
#endif
      // collect OEM controller frame
      if(BlueWireData.available()) {
        if(CommState.collectData(OEMCtrlFrame, BlueWireData.getValue()) ) {
          CommState.set(CommStates::OEMCtrlReport);  // collected 24 bytes, move on!
        }
      }
      break;


    case CommStates::OEMCtrlReport:
#if RX_LED == 1
    digitalWrite(LED_Pin, LOW);
#endif
      // test for valid CRC, abort and restarts Serial1 if invalid
      if(!validateFrame(OEMCtrlFrame, "OEM")) {
        break;
      }

      // filled OEM controller frame, report
      // echo received OEM controller frame over Bluetooth, using [OEM] header
      // note that Rotary Knob and LED OEM controllers can flood the Bluetooth 
      // handling at the client side, moderate OEM Bluetooth delivery
      if(OEMCtrlFrame.elapsedTime() > OEM_TO_BLUETOOTH_MODERATION_TIME) {
        Bluetooth.sendFrame("[OEM]", OEMCtrlFrame, TERMINATE_OEM_LINE);
        OEMCtrlFrame.setTime();
      }
      else {
#if REPORT_SUPPRESSED_OEM_DATA_FRAMES != 0
        DebugPort.println("Suppressed delivery of OEM frame");
#endif
      }
      CommState.set(CommStates::HeaterRx1);
      break;


    case CommStates::HeaterRx1:
#if RX_LED == 1
    digitalWrite(LED_Pin, HIGH);
#endif
      // collect heater frame, always in response to an OEM controller frame
      if(BlueWireData.available()) {
        if( CommState.collectData(HeaterFrame1, BlueWireData.getValue()) ) {
          CommState.set(CommStates::HeaterReport1);
        }
      }
      break;


    case CommStates::HeaterReport1:
#if RX_LED == 1
    digitalWrite(LED_Pin, LOW);
#endif

      // test for valid CRC, abort and restarts Serial1 if invalid
      if(!validateFrame(HeaterFrame1, "RX1")) {
        break;
      }

      // received heater frame (after controller message), report
    
      // do some monitoring of the heater state variable
      // if abnormal transitions, introduce a smart error!
      // This will also cancel ON/OFF requests if runstate in startup/shutdown
      SmartError.monitor(HeaterFrame1);

      // echo heater reponse data to Bluetooth client
      // note that Rotary Knob and LED OEM controllers can flood the Bluetooth 
      // handling at the client side, moderate OEM Bluetooth delivery
      if(HeaterFrame1.elapsedTime() > OEM_TO_BLUETOOTH_MODERATION_TIME) {
        Bluetooth.sendFrame("[HTR]", HeaterFrame1, true);
        HeaterFrame1.setTime();
      }
      else {
#if REPORT_SUPPRESSED_OEM_DATA_FRAMES != 0
        DebugPort.println("Suppressed delivery of OEM heater response frame");
#endif
      }

      if(digitalRead(ListenOnlyPin)) {
        bool isBTCmaster = false;
        while(BlueWireSerial.available()) {
          DebugPort.println("DUMPED ROGUE RX DATA");
          BlueWireSerial.read();
        }
        BlueWireSerial.flush();
        TxManage.PrepareFrame(OEMCtrlFrame, isBTCmaster);  // parrot OEM parameters, but block NV modes
        TxManage.Start(timenow);
        CommState.set(CommStates::BTC_Tx);
      }
      else {
//        CommState.set(CommStates::Idle);    // "Listen Only" input is  held low, don't send out Tx
        HeaterData.set(HeaterFrame1, OEMCtrlFrame);
//        pRxFrame = &HeaterFrame1;
//        pTxFrame = &OEMCtrlFrame;
        CommState.set(CommStates::TemperatureRead);    // "Listen Only" input is  held low, don't send out Tx
      }
      break;
    

    case CommStates::BTC_Tx:
      // Handle time interval where we send data to the blue wire
      lastRxTime = timenow;                     // *we* are pumping onto blue wire, track this activity!
      if(TxManage.CheckTx(timenow) ) {          // monitor progress of our data delivery
        if(!hasOEMController) {
          // only convey this frames to Bluetooth when NOT using an OEM controller!
          Bluetooth.sendFrame("[BTC]", TxManage.getFrame(), TERMINATE_BTC_LINE);    //  BTC => Bluetooth Controller :-)
        }
        CommState.set(CommStates::HeaterRx2);   // then await heater repsonse
      }
      break;


    case CommStates::HeaterRx2:
#if RX_LED == 1
    digitalWrite(LED_Pin, HIGH);
#endif
      // collect heater frame, in response to our control frame
      if(BlueWireData.available()) {
#ifdef BADSTARTCHECK
        if(!CommState.checkValidStart(BlueWireData.getValue())) {
          DebugPort.println("***** \007 Invalid start of frame - restarting Serial port *****");    
          initBlueWireSerial();
          CommState.set(CommStates::Idle);
        }
        else {
          if( CommState.collectData(HeaterFrame2, BlueWireData.getValue()) ) {
            CommState.set(CommStates::HeaterReport2);
          }
        }
#else
        if( CommState.collectData(HeaterFrame2, BlueWireData.getValue()) ) {
          CommState.set(CommStates::HeaterReport2);
        }
#endif
      } 
      break;


    case CommStates::HeaterReport2:
#if RX_LED == 1
    digitalWrite(LED_Pin, LOW);
#endif

      // test for valid CRC, abort and restarts Serial1 if invalid
      if(!validateFrame(HeaterFrame2, "RX2")) {
        break;
      }

      // received heater frame (after our control message), report

      // do some monitoring of the heater state variables
      // if abnormal transitions, introduce a smart error!
      SmartError.monitor(HeaterFrame2);

      delay(5);
      if(!hasOEMController) {
        // only convey these frames to Bluetooth when NOT using an OEM controller!
        Bluetooth.sendFrame("[HTR]", HeaterFrame2, true);    // pin not grounded, suppress duplicate to BT
      }
      CommState.set(CommStates::TemperatureRead);
      HeaterData.set(HeaterFrame2, TxManage.getFrame());
//      pRxFrame = &HeaterFrame2;
//      pTxFrame = &TxManage.getFrame();
      break;

    case CommStates::TemperatureRead:
      // update temperature reading, 
      // synchronised with serial reception as interrupts do get disabled in the OneWire library
      tDelta = timenow - lastTemperatureTime;
      if(tDelta > TEMPERATURE_INTERVAL) {               // maintain a minimum holdoff period
        lastTemperatureTime += TEMPERATURE_INTERVAL;    // reset time to observe temeprature
        fTemperature = TempSensor.getTempCByIndex(0);    // read sensor
        // initialise filtered temperature upon very first pass
        if(fFilteredTemperature <= -90) {              // avoid FP exactness issues
          fFilteredTemperature = fTemperature;         // prime with initial reading
        }
        // exponential mean to stabilse readings
        fFilteredTemperature = fFilteredTemperature * fAlpha + (1-fAlpha) * fTemperature;
        DefaultBTCParams.setTemperature_Actual((unsigned char)(fFilteredTemperature + 0.5));  // update [BTC] frame to send
        TempSensor.requestTemperatures();               // prep sensor for future reading
        ScreenManager.reqUpdate();
      }
      CommState.set(CommStates::Idle);
      break;
  }  // switch(CommState)

  BlueWireData.reset();   // ensure we flush any used data

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


void interpretCommand(const char* pLine)
{
  unsigned char cVal;
  unsigned short sVal;
  float fVal;

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
      SmartError.reset();
      Bluetooth.setRefTime();
    }
    else if(strncmp(pLine, "OFF", 3) == 0) {
      TxManage.queueOffRequest();
      DebugPort.println("Heater OFF");
      SmartError.inhibit();
      Bluetooth.setRefTime();
    }
    else if(strncmp(pLine, "Pmin", 4) == 0) {
      pLine += 4;
      fVal = atof(pLine);
      NVstore.setPmin(fVal);
      DebugPort.print("Pump min = "); DebugPort.println(fVal);
    }
    else if(strncmp(pLine, "Pmax", 4) == 0) {
      pLine += 4;
      fVal = atof(pLine);
      NVstore.setPmax(fVal);
      DebugPort.print("Pump max = "); DebugPort.println(fVal);
    }
    else if(strncmp(pLine, "Fmin", 4) == 0) {
      pLine += 4;
      sVal = atoi(pLine);
      NVstore.setFmin(sVal);
      DebugPort.print("Fan min = "); DebugPort.println(sVal);
    }
    else if(strncmp(pLine, "Fmax", 4) == 0) {
      pLine += 4;
      sVal = atoi(pLine);
      NVstore.setFmax(sVal);
      DebugPort.print("Fan max = "); DebugPort.println(int(sVal));
    }
    else if(strncmp(pLine, "save", 4) == 0) {
      NVstore.save();
      DebugPort.println("NV save");
    }
    else if(strncmp(pLine, "degC", 4) == 0) {
      pLine += 4;
      cVal = atoi(pLine);
      NVstore.setTemperature(cVal);
      DebugPort.print("degC = "); DebugPort.println(cVal);
    }
    else if(strncmp(pLine, "Mode", 4) == 0) {
      pLine += 4;
      cVal = !NVstore.getThermostatMode();
      NVstore.setThermostatMode(cVal);
      DebugPort.print("Mode now "); DebugPort.println(cVal ? "Thermostat" : "Fixed Hz");
    }
    else {
      DebugPort.print(pLine); DebugPort.println(" ????");
    }

  }
}

void initBlueWireSerial()
{
  // initialize serial port to interact with the "blue wire"
  // 25000 baud, Tx and Rx channels of Chinese heater comms interface:
  // Tx/Rx data to/from heater, 
  // Note special baud rate for Chinese heater controllers
#if defined(__arm__) || defined(__AVR__)
  BlueWireSerial.begin(25000);   
  pinMode(Rx1Pin, INPUT_PULLUP);  // required for MUX to work properly
#elif ESP32
  // ESP32
  BlueWireSerial.begin(25000, SERIAL_8N1, Rx1Pin, Tx1Pin);  // need to explicitly specify pins for pin multiplexer!
  pinMode(Rx1Pin, INPUT_PULLUP);  // required for MUX to work properly
#endif
}

bool validateFrame(const CProtocol& frame, const char* name)
{
  if(!frame.verifyCRC()) {
    // Bad CRC - restart blue wire Serial port
    DebugPort.print("\007Bad CRC detected for ");
    DebugPort.print(name);
    DebugPort.println(" frame - restarting blue wire's serial port");
    char header[16];
    sprintf(header, "[CRC_%s]", name);
    DebugReportFrame(header, frame, "\r\n");
    initBlueWireSerial();
    CommState.set(CommStates::Idle);
    return false;
  }
  return true;
}


void requestOn()
{
  TxManage.queueOnRequest();
  SmartError.reset();
  Bluetooth.setRefTime();
}

void requestOff()
{
  TxManage.queueOffRequest();
  SmartError.inhibit();
  Bluetooth.setRefTime();
}

void ToggleOnOff()
{
  if(HeaterData.getRunState()) {
    DebugPort.println("ToggleOnOff: Heater OFF");
    requestOff();
  }
  else {
    DebugPort.println("ToggleOnOff: Heater ON");
    requestOn();
  }
}


void reqTemp(unsigned char newTemp)
{
  unsigned char max = DefaultBTCParams.getTemperature_Max();
  unsigned char min = DefaultBTCParams.getTemperature_Min();
  if(newTemp >= max)
    newTemp = max;
  if(newTemp <= min)
    newTemp = min;
     
  NVstore.setTemperature(newTemp);

  ScreenManager.reqUpdate();
}

void reqTempDelta(int delta)
{
  unsigned char newTemp = getSetTemp() + delta;

  reqTemp(newTemp);
}

int getSetTemp()
{
  return NVstore.getTemperature();
}

void reqThermoToggle()
{
  setThermostatMode(getThermostatMode() ? 0 : 1);
}

void setThermostatMode(unsigned char val)
{
  NVstore.setThermostatMode(val);
}


bool getThermostatMode()
{
  return NVstore.getThermostatMode() != 0;
}

void checkDisplayUpdate()
{
  // only update OLED when not processing blue wire
  if(ScreenManager.checkUpdate()) {
    lastAnimationTime = millis() + 100;
    ScreenManager.animate();
    ScreenManager.refresh();   // always refresh post major update
  }
 

  long tDelta = millis() - lastAnimationTime;
  if(tDelta >= 100) {
    lastAnimationTime = millis() + 100;
    if(ScreenManager.animate())
      ScreenManager.refresh();
  }
}

void reqPumpPrime(bool on)
{
  DefaultBTCParams.setPump_Prime(on);
}

float getActualTemperature()
{
  return fFilteredTemperature;
}

void  setPumpMin(float val)
{
  NVstore.setPmin(val);
}

void  setPumpMax(float val)
{
  NVstore.setPmax(val);
}

void  setFanMin(short cVal)
{
  NVstore.setFmin(cVal);
}

void  setFanMax(short cVal)
{
  NVstore.setFmax(cVal);
}

void saveNV() 
{
  NVstore.save();
}

const CProtocolPackage& getHeaterInfo()
{
  return HeaterData;
}

bool isWebClientConnected()
{
  return bHaveWebClient;
}

void checkDebugCommands()
{
  // check for test commands received from PC Over USB
  if(DebugPort.available()) {
    static int mode = 0;
    static int val = 0;

    char rxVal = DebugPort.read();

    bool bSendVal = false;
    if(isControl(rxVal)) {    // "End of Line"
      String convert(PCline.Line);
      val = convert.toInt();
      bSendVal = true;
      PCline.clear();
    }
    else {
      if(isDigit(rxVal)) {
        PCline.append(rxVal);
      }
      else if((rxVal == 'p') || (rxVal == 'P')) {
        DebugPort.println("Test Priming Byte... ");
        mode = 1;
      }
      else if((rxVal == 'g') || (rxVal == 'G')) {
        DebugPort.println("Test glow power byte... ");
        mode = 2;
      }
      else if((rxVal == 'i') || (rxVal == 'I')) {
        DebugPort.println("Test fan bytes");
        mode = 3;
      }
      else if(rxVal  == '+') {
        TxManage.queueOnRequest();
        Bluetooth.setRefTime();             // reset time reference "run time"
      }
      else if(rxVal  == '-') {
        TxManage.queueOffRequest();
        Bluetooth.setRefTime();
      }
      else if(rxVal == ']') {
        val++;
        bSendVal = true;
      }
      else if(rxVal == '[') {
        val--;
        bSendVal = true;
      }
    }
    if(bSendVal) {
      switch(mode) {
        case 1:
          DefaultBTCParams.Controller.Prime = val & 0xff;     // always  0x32:Thermostat, 0xCD:Fixed
          break;
        case 2:
          DefaultBTCParams.Controller.MinTempRise = val & 0xff;     // always 0x05
          break;
        case 3:
          DefaultBTCParams.Controller.Unknown2_MSB = (val >> 8) & 0xff;     // always 0x0d
          DefaultBTCParams.Controller.Unknown2_LSB = (val >> 0) & 0xff;     // always 0xac  16bit: "3500" ??  Ignition fan max RPM????
          break;
      }
    }
  }
}
