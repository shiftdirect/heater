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

//#include "src/WiFi/ABMqtt.h"
#include "src/cfg/BTCConfig.h"
#include "src/cfg/pins.h"
#include "src/RTC/Timers.h"
#include "src/RTC/Clock.h"
#include "src/WiFi/BTCWebServer.h"
#include "src/WiFi/BTCota.h"
#include "src/Protocol/Protocol.h"
#include "src/Protocol/TxManage.h"
#include "src/Protocol/SmartError.h"
#include "src/Utility/helpers.h"
#include "src/Utility/NVStorage.h"
#include "src/Utility/DebugPort.h"
#include "src/Utility/UtilClasses.h"
#include "src/Utility/BTC_JSON.h"
#include "src/Utility/BTC_GPIO.h"
#include "src/Utility/BoardDetect.h"
#include "src/OLED/ScreenManager.h"
#include "src/OLED/KeyPad.h"
#include "src/Utility/TempSense.h"
#include <rom/rtc.h>
#include <esp_spiffs.h>
#include <SPIFFS.h>

// SSID & password now stored in NV storage - these are still the default values.
//#define AP_SSID "Afterburner"
//#define AP_PASSWORD "thereisnospoon"

#define RX_DATA_TIMOUT 50

const int FirmwareRevision = 23;
const int FirmwareSubRevision = 5;
const char* FirmwareDate = "30 Jun 2019";


#ifdef ESP32
#include "src/Bluetooth/BluetoothESP32.h"
#else
#include "src/Bluetooth/BluetoothHC05.h"
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
void manageCyclicMode();
void doStreaming();
void heaterOn();
void heaterOff();

// DS18B20 temperature sensor support
// Uses the RMT timeslot driver to operate as a one-wire bus
CTempSense TempSensor;
long lastTemperatureTime;            // used to moderate DS18B20 access
float fFilteredTemperature = -100;   // -100: force direct update uopn first pass
const float fAlpha = 0.95;           // exponential mean alpha
int DS18B20holdoff = 2;

int BoardRevision = 0;

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
CGPIOin GPIOin;
CGPIOout GPIOout;
CGPIOalg GPIOalg;

sRxLine PCline;
long lastRxTime;                     // used to observe inter character delays
bool bHasOEMController = false;
bool bHasOEMLCDController = false;
bool bHasHtrData = false;

// these variables will persist over a soft reboot.
__NOINIT_ATTR bool bForceInit; // = false;
__NOINIT_ATTR bool bUserON; // = false;
__NOINIT_ATTR uint8_t demandDegC;
__NOINIT_ATTR uint8_t demandPump;

bool bReportBlueWireData = REPORT_RAW_DATA;
bool bReportJSONData = REPORT_JSON_TRANSMIT;
bool bReportRecyleEvents = REPORT_BLUEWIRE_RECYCLES;
bool bReportOEMresync = REPORT_OEM_RESYNC;

CProtocolPackage reportHeaterData;
CProtocolPackage primaryHeaterData;

unsigned long moderator;
bool bUpdateDisplay = false;
bool bHaveWebClient = false;
bool bBTconnected = false;

hw_timer_t *watchdogTimer = NULL;

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


void interruptReboot()
{     
  ets_printf("Software watchdog reboot......\r\n");
  esp_restart();
}

void setup() {

  // ensure cyclic mode is disabled after power on
  if(rtc_get_reset_reason(0) == 1 || bForceInit) {
    bForceInit = false;
    bUserON = false;   
    demandPump = demandDegC = 22;
  }

  // initially, ensure the GPIO outputs are not activated during startup
  // (GPIO2 tends to be one with default chip startup)
  pinMode(GPIOout1_pin, OUTPUT);  
  pinMode(GPIOout2_pin, OUTPUT);  
  digitalWrite(GPIOout1_pin, LOW);
  digitalWrite(GPIOout2_pin, LOW);

  // initialise TelnetSpy (port 23) as well as Serial to 115200 
  // Serial is the usual USB connection to a PC
  // DO THIS BEFORE WE TRY AND SEND DEBUG INFO!
  
  DebugPort.setWelcomeMsg((char*)(
                          "*************************************************\r\n"
                          "* Connected to BTC heater controller debug port *\r\n"
                          "*************************************************\r\n"
                          ));
  DebugPort.setBufferSize(8192);
  DebugPort.begin(115200);
  DebugPort.println("_______________________________________________________________");
  
  DebugPort.printf("Reset reason: core0:%d, core1:%d\r\n", rtc_get_reset_reason(0), rtc_get_reset_reason(0));
  DebugPort.printf("Previous user ON = %d\r\n", bUserON);   // state flag required for cyclic mode to persist properly after a WD reboot :-)

  // initialise DS18B20 sensor interface
  TempSensor.begin(DS18B20_Pin);
  TempSensor.startConvert();  // kick off initial temperature sample


  lastTemperatureTime = millis();
  lastAnimationTime = millis();
  
  BoardRevision = BoardDetect();
  DebugPort.printf("Board revision: V%.1f\r\n", float(BoardRevision) * 0.1);

  DebugPort.printf("ESP32 IDF Version: %s\r\n", esp_get_idf_version());

 // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    DebugPort.println("An Error has occurred while mounting SPIFFS");
  }
  else {
    DebugPort.println("Mounted SPIFFS OK");
    DebugPort.printf("SPIFFS usage: %d/%d\r\n", SPIFFS.usedBytes(), SPIFFS.totalBytes());
    DebugPort.println("Listing SPIFFS contents:");
    String report;
    listDir(SPIFFS, "/", 2, report);
  }

  NVstore.init();
  NVstore.load();
  
  initMQTTJSONmoderator();   // prevents JSON for MQTT unless requested
  initTimerJSONmoderator();  // prevents JSON for timers unless requested


  KeyPad.begin(keyLeft_pin, keyRight_pin, keyCentre_pin, keyUp_pin, keyDown_pin);
  KeyPad.setCallback(parentKeyHandler);

  // Initialize the rtc object
  Clock.begin();

  bool bNoClock = true;
  const BTCDateTime& now = Clock.get();
  if(now.day() != 0xa5)
    bNoClock = false;
  
  ScreenManager.begin(bNoClock);

#if USE_WIFI == 1

  sCredentials creds = NVstore.getCredentials();

  if(NVstore.getUserSettings().enableWifi) {
    initWifi(WiFi_TriggerPin, creds.SSID, creds.APpassword);
#if USE_OTA == 1
    if(NVstore.getUserSettings().enableOTA) {
      initOTA();
    }
#endif // USE_OTA
#if USE_WEBSERVER == 1
    initWebServer();
#endif // USE_WEBSERVER
  }

#endif // USE_WIFI

//  pinMode(ListenOnlyPin, INPUT_PULLUP);   // pin to enable passive mode
  pinMode(LED_Pin, OUTPUT);               // On board LED indicator
  digitalWrite(LED_Pin, LOW);

  initBlueWireSerial();
  
  // prepare for first long delay detection
  lastRxTime = millis();

  TxManage.begin(); // ensure Tx enable pin is setup

  // define defaults should OEM controller be missing
  DefaultBTCParams.setHeaterDemand(23);
  DefaultBTCParams.setTemperature_Actual(22);
  DefaultBTCParams.setSystemVoltage(12.0);
  DefaultBTCParams.setPump_Min(1.6f);
  DefaultBTCParams.setPump_Max(5.5f);
  DefaultBTCParams.setFan_Min(1680);
  DefaultBTCParams.setFan_Max(4500);
  DefaultBTCParams.Controller.FanSensor = 1;

  bBTconnected = false;
  Bluetooth.begin();

  setupGPIO(); 

#if USE_SW_WATCHDOG == 1
  // create a watchdog timer
  watchdogTimer = timerBegin(0, 80, true); //timer 0, divisor 80     
  timerAlarmWrite(watchdogTimer, 15000000, false); //set time in uS must be fed within this time or reboot     
  timerAttachInterrupt(watchdogTimer, &interruptReboot, true);     
  timerAlarmEnable(watchdogTimer); //enable interrupt
#endif

  delay(1000); // just to hold the splash screeen for while
}



// main functional loop is based about a state machine approach, waiting for data 
// to appear upon the blue wire, and marshalling into an appropriate receive buffers
// according to the state.

void loop() 
{

  float fTemperature;
  unsigned long timenow = millis();

  // DebugPort.handle();    // keep telnet spy alive

  //////////////////////////////////////////////////////////////////////////////////////
  // Blue wire data reception
  //  Reads data from the "blue wire" Serial port, (to/from heater)
  //  If an OEM controller exists we will also see it's data frames
  //  Note that the data is read now, then held for later use in the state machine
  //
  sRxData BlueWireData;

  // calc elapsed time since last rxd byte
  // used to detect no OEM controller, or the start of an OEM frame sequence
  unsigned long RxTimeElapsed = timenow - lastRxTime;

  if (BlueWireSerial.available()) {
    // Data is available, read and store it now, use it later
    // Note that if not in a recognised data receive frame state, the data 
    // will be deliberately lost!
    BlueWireData.setValue(BlueWireSerial.read());  // read hex byte, store for later use
      
    lastRxTime = timenow;    // tickle last rx time, for rx data timeout purposes
  } 


  // precautionary state machine action if all 24 bytes were not received 
  // whilst expecting a frame from the blue wire
  if(RxTimeElapsed > RX_DATA_TIMOUT) {              
    if( CommState.is(CommStates::OEMCtrlRx) || 
        CommState.is(CommStates::HeaterRx1) ||  
        CommState.is(CommStates::HeaterRx2) ) {

      if(RxTimeElapsed >= moderator) {
        moderator += 10;
        if(bReportRecyleEvents) {
          DebugPort.printf("%ldms - ", RxTimeElapsed);
        }
        if(CommState.is(CommStates::OEMCtrlRx)) {
          bHasOEMController = false;
          bHasOEMLCDController = false;
          if(bReportRecyleEvents) 
            DebugPort.println("Timeout collecting OEM controller data, returning to Idle State");
        }
        else if(CommState.is(CommStates::HeaterRx1)) {
          bHasHtrData = false;
          if(bReportRecyleEvents) 
            DebugPort.println("Timeout collecting OEM heater response data, returning to Idle State");
        }
        else {
          bHasHtrData = false;
          if(bReportRecyleEvents) 
            DebugPort.println("Timeout collecting BTC heater response data, returning to Idle State");
        }
      }

      if(bReportRecyleEvents) 
        DebugPort.println("Recycling blue wire serial interface");
      initBlueWireSerial();
      CommState.set(CommStates::TemperatureRead);    // revert to idle mode, after passing thru temperature mode
    }
  }

  ///////////////////////////////////////////////////////////////////////////////////////////
  // do our state machine to track the reception and delivery of blue wire data

  long tDelta;
  switch(CommState.get()) {

    case CommStates::Idle:

#if USE_SW_WATCHDOG == 1
      feedWatchdog(); //reset timer (feed watchdog)  
#endif
      
      doStreaming();   // do wifi, BT tx etc when NOT in midst of handling blue wire
                       // this especially avoids E-07 faults due to larger data transfers

      moderator = 50;

#if RX_LED == 1
      digitalWrite(LED_Pin, LOW);
#endif
      // Detect the possible start of a new frame sequence from an OEM controller
      // This will be the first activity for considerable period on the blue wire
      // The heater always responds to a controller frame, but otherwise never by itself
      
      if(RxTimeElapsed >= (NVstore.getUserSettings().FrameRate - 60)) {  // compensate for the time spent just doing things in this state machine
        // have not seen any receive data for a second.
        // OEM controller is probably not connected. 
        // Skip state machine immediately to BTC_Tx, sending our own settings.
        bHasHtrData = false;
        bHasOEMController = false;
        bHasOEMLCDController = false;
        bool isBTCmaster = true;
        TxManage.PrepareFrame(DefaultBTCParams, isBTCmaster);  // use our parameters, and mix in NV storage values
        TxManage.Start(timenow);
        CommState.set(CommStates::BTC_Tx);
        break;
      } 

#if SUPPORT_OEM_CONTROLLER == 1
      if(BlueWireData.available() && (RxTimeElapsed > RX_DATA_TIMOUT+10)) {  

        if(bReportOEMresync) {
          DebugPort.printf("Re-sync'd with OEM Controller. %ldms Idle time.\r\n", RxTimeElapsed);
        }

        bHasHtrData = false;
        bHasOEMController = true;
        CommState.set(CommStates::OEMCtrlRx);   // we must add this new byte!
        //
        //  ** IMPORTANT - we must drop through to OEMCtrlRx *NOW* (skipping break) **
        //  **             otherwise the first byte will be lost!                   **
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
          CommState.set(CommStates::OEMCtrlValidate);  // collected 24 bytes, move on!
        }
      }
      break;


    case CommStates::OEMCtrlValidate:
#if RX_LED == 1
    digitalWrite(LED_Pin, LOW);
#endif
      // test for valid CRC, abort and restarts Serial1 if invalid
      if(!validateFrame(OEMCtrlFrame, "OEM")) {
        break;
      }

      // filled OEM controller frame
      OEMCtrlFrame.setTime();
      // LCD controllers use 0x76 as first byte, rotary knobs use 0x78
      bHasOEMLCDController = (OEMCtrlFrame.Controller.Byte0 != 0x78);

      CommState.set(CommStates::HeaterRx1);
      break;


    case CommStates::HeaterRx1:
#if RX_LED == 1
    digitalWrite(LED_Pin, HIGH);
#endif
      // collect heater frame, always in response to an OEM controller frame
      if(BlueWireData.available()) {
        if( CommState.collectData(HeaterFrame1, BlueWireData.getValue()) ) {
          CommState.set(CommStates::HeaterValidate1);
        }
      }
      break;


    case CommStates::HeaterValidate1:
#if RX_LED == 1
    digitalWrite(LED_Pin, LOW);
#endif

      // test for valid CRC, abort and restarts Serial1 if invalid
      if(!validateFrame(HeaterFrame1, "RX1")) {
        bHasHtrData = false;
        break;
      }
      bHasHtrData = true;

      // received heater frame (after controller message), report
    
      // do some monitoring of the heater state variable
      // if abnormal transitions, introduce a smart error!
      // This routine also cancels ON/OFF requests if runstate in startup/shutdown periods
      SmartError.monitor(HeaterFrame1);

      HeaterFrame1.setTime();

      while(BlueWireSerial.available()) {
        DebugPort.println("DUMPED ROGUE RX DATA");
        BlueWireSerial.read();
      }
      BlueWireSerial.flush();

      primaryHeaterData.set(HeaterFrame1, OEMCtrlFrame);  // OEM is always *the* controller
      if(bReportBlueWireData) {
        primaryHeaterData.reportFrames(true);
        CommState.setDelay(20);          // let serial get sent before we send blue wire
      }
      else {
        CommState.setDelay(0);
      }
      CommState.set(CommStates::HeaterReport1);
      break;


    case CommStates::HeaterReport1:
      if(CommState.delayExpired()) {
/*        if(digitalRead(ListenOnlyPin)) {  // pin open, pulled high (STANDARD OPERATION)*/
          bool isBTCmaster = false;
          TxManage.PrepareFrame(OEMCtrlFrame, isBTCmaster);  // parrot OEM parameters, but block NV modes
          TxManage.Start(timenow);
          CommState.set(CommStates::BTC_Tx);
/*        }
        else {   // pin shorted to ground
          CommState.set(CommStates::TemperatureRead);    // "Listen Only" input is  held low, don't send our Tx
        }*/
      }
      break;
    

    case CommStates::BTC_Tx:
      // Handle time interval where we send data to the blue wire
      lastRxTime = timenow;                     // *we* are pumping onto blue wire, track this activity!
      if(TxManage.CheckTx(timenow) ) {          // monitor progress of our data delivery
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
          DebugPort.println("***** Invalid start of frame - restarting Serial port *****");    
          initBlueWireSerial();
          CommState.set(CommStates::Idle);
        }
        else {
          if( CommState.collectData(HeaterFrame2, BlueWireData.getValue()) ) {
            CommState.set(CommStates::HeaterValidate2);
          }
        }
#else
        if( CommState.collectData(HeaterFrame2, BlueWireData.getValue()) ) {
          CommState.set(CommStates::HeaterValidate2);
        }
#endif
      } 
      break;


    case CommStates::HeaterValidate2:
#if RX_LED == 1
      digitalWrite(LED_Pin, LOW);
#endif

      // test for valid CRC, abort and restart Serial1 if invalid
      if(!validateFrame(HeaterFrame2, "RX2")) {
        bHasHtrData = false;
        break;
      }
      bHasHtrData = true;

      // received heater frame (after our control message), report

      // do some monitoring of the heater state variables
      // if abnormal transitions, introduce a smart error!
      SmartError.monitor(HeaterFrame2);

      if(!bHasOEMController)              // no OEM controller - BTC is *the* controller
        primaryHeaterData.set(HeaterFrame2, TxManage.getFrame());
       
      if(bReportBlueWireData) {
        reportHeaterData.set(HeaterFrame2, TxManage.getFrame());
        reportHeaterData.reportFrames(false);
        CommState.setDelay(20);          // let serial get sent before we send blue wire
      }
      else {
        CommState.setDelay(0);
      }
      CommState.set(CommStates::HeaterReport2);
      break;


    case CommStates::HeaterReport2:
      if(CommState.delayExpired()) {
        CommState.set(CommStates::TemperatureRead);
      }
      break;


    case CommStates::TemperatureRead:
      // update temperature reading, 
      // synchronised with serial reception as interrupts do get disabled in the OneWire library
      tDelta = timenow - lastTemperatureTime;
      if(tDelta > MIN_TEMPERATURE_INTERVAL) {               // maintain a minimum holdoff period
        lastTemperatureTime = millis();    // reset time to observe temeprature        

        if(TempSensor.readTemperature(fTemperature)) {
          if(DS18B20holdoff) {
            DS18B20holdoff--; 
            DebugPort.printf("Skipped initial DS18B20 reading: %f\r\n", fTemperature);
          }                           // first value upon sensor connect is bad
          else {
        // initialise filtered temperature upon very first pass
            if(fFilteredTemperature < -90) {            // avoid FP exactness issues - starts as -100 on boot
              fFilteredTemperature = fTemperature;       // prime with first *valid* reading
            }
            // exponential mean to stabilse readings
            fFilteredTemperature = fFilteredTemperature * fAlpha + (1-fAlpha) * fTemperature;

            manageCyclicMode();
          }
        }
        else {
          DS18B20holdoff = 3;
            fFilteredTemperature = -100;
          }

        TempSensor.startConvert();  // request a new conversion, will be ready by the time we loop back around

        ScreenManager.reqUpdate();
      }
      updateJSONclients(bReportJSONData);
      CommState.set(CommStates::Idle);
      NVstore.doSave();   // now is a good time to store to the NV storage, well away from any blue wire activity
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

void manageCyclicMode()
{
  const sCyclicThermostat& cyclic = NVstore.getCyclicMode();
  if(cyclic.Stop && bUserON) {   // cyclic mode enabled, and user has started heater
    int stopDeltaT = cyclic.Stop + 1;  // bump up by 1 degree - no point invoking at 1 deg over!
    float deltaT = fFilteredTemperature - getDemandDegC();
//    DebugPort.printf("Cyclic=%d bUserOn=%d deltaT=%d\r\n", cyclic, bUserON, deltaT);

    // ensure we cancel user ON mode if heater throws an error
    int errState = getHeaterInfo().getErrState();
    if((errState > 1) && (errState < 12) && (errState != 8)) {
      // excludes errors 0,1(OK), 12(E1-11,Retry) & 8(E-07,Comms Error)
      DebugPort.println("CYCLIC MODE: cancelling user ON status"); 
      requestOff();   // forcibly cancel cyclic operation - pretend user pressed OFF
    }
    int heaterState = getHeaterInfo().getRunState();
    // check if over temp, turn off heater
    if(deltaT > stopDeltaT) {
      if(heaterState > 0 && heaterState <= 5) {  
        DebugPort.printf("CYCLIC MODE: Stopping heater, deltaT > +%d\r\n", stopDeltaT);
        heaterOff();    // over temp - request heater stop
      }
    }
    // check if under temp, turn on heater
    if(deltaT < cyclic.Start) {
      // typ. 1 degree below set point - restart heater
      if(heaterState == 0) {
        DebugPort.printf("CYCLIC MODE: Restarting heater, deltaT <%d\r\n", cyclic.Start);
        heaterOn();
      }
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
    DebugPort.printf("\007Bad CRC detected for %s frame - restarting blue wire's serial port\r\n", name);
    DebugReportFrame("BAD CRC:", frame, "\r\n");
    initBlueWireSerial();
    CommState.set(CommStates::TemperatureRead);
    return false;
  }
  return true;
}


void requestOn()
{
  heaterOn();
  bUserON = true;    // for cyclic mode
}

void requestOff()
{
  heaterOff();
  bUserON = false;   // for cyclic mode
}

void heaterOn() 
{
  TxManage.queueOnRequest();
  SmartError.reset();
}

void heaterOff()
{
  TxManage.queueOffRequest();
  SmartError.inhibit();
}


bool reqTemp(unsigned char newTemp, bool save)
{
  if(bHasOEMController)
    return false;

  unsigned char max = DefaultBTCParams.getTemperature_Max();
  unsigned char min = DefaultBTCParams.getTemperature_Min();
  if(newTemp >= max)
    newTemp = max;
  if(newTemp <= min)
    newTemp = min;
  
  // set and save the demand to NV storage
  // note that we now maintain fixed Hz and Thermostat set points seperately
  if(getThermostatModeActive())
    demandDegC = newTemp;
  else 
    demandPump = newTemp;

  ScreenManager.reqUpdate();
  return true;
}

bool reqTempDelta(int delta)
{
  unsigned char newTemp;
  if(getThermostatModeActive()) 
    newTemp = demandDegC + delta;
  else
    newTemp = demandPump + delta;

  return reqTemp(newTemp);
}

bool reqThermoToggle()
{
  return setThermostatMode(getThermostatModeActive() ? 0 : 1);
}

bool setThermostatMode(unsigned char val)
{
  if(bHasOEMController)
    return false;

  sUserSettings settings = NVstore.getUserSettings();
  settings.useThermostat = val;
  NVstore.setUserSettings(settings);
  return true;
}

void setDegFMode(bool state)
{
  sUserSettings settings = NVstore.getUserSettings();
  settings.degF = state ? 0x01 : 0x00;
  NVstore.setUserSettings(settings);
}


bool getThermostatModeActive()
{
  if(bHasOEMController) {
    return getHeaterInfo().isThermostat();
  }
  else {
    return NVstore.getUserSettings().useThermostat != 0;
  }
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

void forceBootInit()
{
  bForceInit = true;
}

uint8_t getDemandDegC() 
{
  return demandDegC;
}

uint8_t getDemandPump() 
{
  return demandPump;
}


float getTemperatureDesired()
{
  if(bHasOEMController) {
    return getHeaterInfo().getHeaterDemand();
  }
  else {
    if(getThermostatModeActive()) 
      return demandDegC;
    else 
      return demandPump;
  }
}

float getTemperatureSensor()
{
  return fFilteredTemperature;
}

void  setPumpMin(float val)
{
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  tuning.setPmin(val);
  NVstore.setHeaterTuning(tuning);
}

void  setPumpMax(float val)
{
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  tuning.setPmax(val);
  NVstore.setHeaterTuning(tuning);
}

void  setFanMin(short cVal)
{
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  tuning.Fmin = cVal;
  NVstore.setHeaterTuning(tuning);
}

void  setFanMax(short cVal)
{
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  tuning.Fmax = cVal;
  NVstore.setHeaterTuning(tuning);
}

void setFanSensor(unsigned char cVal)
{
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  tuning.fanSensor = cVal;
  NVstore.setHeaterTuning(tuning);
}

void setSystemVoltage(float val) {
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  tuning.setSysVoltage(val);
  NVstore.setHeaterTuning(tuning);
}

void setGlowDrive(unsigned char val) {
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  tuning.glowDrive = val;
  NVstore.setHeaterTuning(tuning);
} 


void saveNV() 
{
  NVstore.save();
}

const CProtocolPackage& getHeaterInfo()
{
  return primaryHeaterData;
}

bool isWebClientConnected()
{
  return bHaveWebClient;
}

void checkDebugCommands()
{
  // check for test commands received from PC Over USB
  if(DebugPort.available()) {
#ifdef PROTOCOL_INVESTIGATION    
    static int mode = 0;
    static int val = 0;
#endif

    char rxVal = DebugPort.read();

    rxVal = toLowerCase(rxVal);

#ifdef PROTOCOL_INVESTIGATION    
    bool bSendVal = false;
#endif
    if(rxVal == '\n') {    // "End of Line"
#ifdef PROTOCOL_INVESTIGATION    
      String convert(PCline.Line);
      val = convert.toInt();
      bSendVal = true;
      PCline.clear();
#endif
    }
    else {
      if(rxVal == ' ') {   // SPACE to bring up menu
        DebugPort.print("\014");
        DebugPort.println("MENU options");
        DebugPort.println("");
        DebugPort.printf("  <B> - toggle raw blue wire data reporting, currently %s\r\n", bReportBlueWireData ? "ON" : "OFF");
        DebugPort.printf("  <J> - toggle output JSON reporting, currently %s\r\n", bReportJSONData ? "ON" : "OFF");
        DebugPort.printf("  <W> - toggle reporting of blue wire timeout/recycling event, currently %s\r\n", bReportRecyleEvents ? "ON" : "OFF");
        DebugPort.printf("  <O> - toggle reporting of OEM resync event, currently %s\r\n", bReportOEMresync ? "ON" : "OFF");        
        DebugPort.printf("  <S> - toggle reporting of state machine transits %s\r\n", CommState.isReporting() ? "ON" : "OFF");        
        DebugPort.println("  <+> - request heater turns ON");
        DebugPort.println("  <-> - request heater turns OFF");
        DebugPort.println("  <R> - restart the ESP");
        DebugPort.println("");
        DebugPort.println("");
        DebugPort.println("");
        DebugPort.println("");
        DebugPort.println("");
        DebugPort.println("");
        DebugPort.println("");
      }
#ifdef PROTOCOL_INVESTIGATION    
      else if(isDigit(rxVal)) {
        PCline.append(rxVal);
      }
      else if(rxVal == 'p') {
        DebugPort.println("Test Priming Byte... ");
        mode = 1;
      }
      else if(rxVal == 'g') {
        DebugPort.println("Test glow power byte... ");
        mode = 2;
      }
      else if(rxVal == 'i') {
        DebugPort.println("Test fan bytes");
        mode = 3;
      }
      else if(rxVal == 'c') {
        DebugPort.println("Test Command Byte... ");
        mode = 4;
      }
      else if(rxVal == 'x') {
        DebugPort.println("Special mode cancelled");
        val = 0;
        mode = 0;
        DefaultBTCParams.Controller.Command = 0;
      }
      else if(rxVal == ']') {
        val++;
        bSendVal = true;
      }
      else if(rxVal == '[') {
        val--;
        bSendVal = true;
      }
#endif
      else if(rxVal == 'b') { 
        bReportBlueWireData = !bReportBlueWireData;
        DebugPort.printf("Toggled raw blue wire data reporting %s\r\n", bReportBlueWireData ? "ON" : "OFF");
      }
      else if(rxVal == 'j')  {
        bReportJSONData = !bReportJSONData;
        DebugPort.printf("Toggled JSON data reporting %s\r\n", bReportJSONData ? "ON" : "OFF");
      }
      else if(rxVal == 'w')  {
        bReportRecyleEvents = !bReportRecyleEvents;
        DebugPort.printf("Toggled blue wire recycling event reporting %s\r\n", bReportRecyleEvents ? "ON" : "OFF");
      }
      else if(rxVal == 'o')  {
        bReportOEMresync = !bReportOEMresync;
        DebugPort.printf("Toggled OEM resync event reporting %s\r\n", bReportOEMresync ? "ON" : "OFF");
      }
      else if(rxVal == 's') {
        CommState.toggleReporting();
      }
      else if(rxVal == '+') {
        TxManage.queueOnRequest();
//        HeaterData.setRefTime();
      }
      else if(rxVal == '-') {
        TxManage.queueOffRequest();
//        HeaterData.setRefTime();
      }
      else if(rxVal == 'r') {
        ESP.restart();            // reset the esp
      }
    }
#ifdef PROTOCOL_INVESTIGATION    
    if(bSendVal) {
      switch(mode) {
        case 1:
          DefaultBTCParams.Controller.Prime = val & 0xff;     // always  0x32:Thermostat, 0xCD:Fixed
          break;
        case 2:
          DefaultBTCParams.Controller.GlowDrive = val & 0xff;     // always 0x05
          break;
        case 3:
          DefaultBTCParams.Controller.Unknown2_MSB = (val >> 8) & 0xff;     // always 0x0d
          DefaultBTCParams.Controller.Unknown2_LSB = (val >> 0) & 0xff;     // always 0xac  16bit: "3500" ??  Ignition fan max RPM????
          break;
        case 4:
          DebugPort.printf("Forced controller command = %d\r\n", val&0xff);
          DefaultBTCParams.Controller.Command = val & 0xff;
          break;
      }
    }
#endif
  }
}

// 0x00 - Normal:  BTC, with heater responding
// 0x01 - Error:   BTC, heater not responding
// 0x02 - Special: OEM controller & heater responding
// 0x03 - Error:   OEM controller, heater not responding
int getBlueWireStat()
{
  int stat = 0;
  if(!bHasHtrData) {
    stat |= 0x01;
  }
  if(bHasOEMController) {
    stat |= 0x02;
  }
  return stat;
}

const char* getBlueWireStatStr()
{
  static const char* BlueWireStates[] = { "BTC,Htr", "BTC", "OEM,Htr", "OEM" };

  return BlueWireStates[getBlueWireStat()];
}

bool hasOEMcontroller()
{
  return bHasOEMController;
}

bool hasOEMLCDcontroller()
{
  return bHasOEMLCDController;
}

int getSmartError()
{
  return SmartError.getError();
}

bool isCyclicActive()
{
  return bUserON && NVstore.getCyclicMode().isEnabled();
}

void setupGPIO()
{
  if(BoardRevision) {
    // some special considerations for GPIO inputs, depending upon PCB hardware
    // V1.0 PCBs only expose bare inputs, which are pulled high. Active state into ESP32 is LOW. 
    // V2.0+ PCBs use an input transistor buffer. Active state into ESP32 is HIGH (inverted).
    int activePinState = (BoardRevision == 10) ? LOW : HIGH;  
    int Input1 = BoardRevision == 20 ? GPIOin1_pinV20 : GPIOin1_pinV21V10;
    GPIOin.begin(Input1, GPIOin2_pin, NVstore.getGPIOparams().inMode, activePinState);

    // GPIO out is always active high from ESP32
    // V1.0 PCBs only expose the bare pins
    // V2.0+ PCBs provide an open collector output that conducts when active
    GPIOout.begin(GPIOout1_pin, GPIOout2_pin, NVstore.getGPIOparams().outMode);

    // ### MAJOR ISSUE WITH ADC INPUTS ###
    //
    // V2.0 PCBs that have not been modified connect the analogue input to GPIO26.
    // This is ADC2 channel (#9). 
    // Unfortunately it was subsequently discovered that any ADC2 input cannot be 
    // used if Wifi is enabled. 
    // THIS ISSUE IS NOT RESOLBVABLE IN SOFTWARE.
    // *** It is not possible to use ANY of the 10 ADC2 channels if Wifi is enabled :-( ***
    //
    // Fix is to cut traces to GPIO33 & GPIO26 and swap the connections.
    // This directs GPIO input1 into GPIO26 and the analogue input into GPIO33 (ADC1_CHANNEL_5)
    // This will be properly fixed in V2.1 PCBs
    //
    // As V1.0 PCBS expose the bare pins, the correct GPIO33 input can be readily chosen.
    GPIOalgModes algMode = NVstore.getGPIOparams().algMode;
    if(BoardRevision == 20)  
      algMode = GPIOalgNone;      // force off analogue support in V2.0 PCBs
    GPIOalg.begin(GPIOalg_pin, algMode);
  }
  else {
    // unknown board - deny all GPIO operation (unlikely)
    GPIOin.begin(0, 0, GPIOinNone, LOW);
    GPIOout.begin(0, 0, GPIOoutNone);
    GPIOalg.begin(ADC1_CHANNEL_5, GPIOalgNone);
  }
}

bool toggleGPIOout(int channel) 
{
  if(NVstore.getGPIOparams().outMode == GPIOoutUser) {
    setGPIOout(channel, !getGPIOout(channel));  // toggle selected GPIO output 
    return true;
  }
  return false;
}

void setGPIOout(int channel, bool state)
{
  DebugPort.printf("setGPIO: Output #%d = %d\r\n", channel+1, state);
  GPIOout.setState(channel, state);
}

bool getGPIOout(int channel)
{
  bool retval = GPIOout.getState(channel);
  DebugPort.printf("getGPIO: Output #%d = %d\r\n", channel+1, retval);
  return retval;
}

float getVersion()
{
  return float(FirmwareRevision) * 0.1f + float(FirmwareSubRevision) * .001f;
}

const char* getVersionStr() {
  static char vStr[32];
  sprintf(vStr, "V%.1f.%d", float(FirmwareRevision) * 0.1f, FirmwareSubRevision);
  return vStr;
}

const char* getVersionDate()
{
  return FirmwareDate;
}

int getBoardRevision()
{
  return BoardRevision;
}

void ShowOTAScreen(int percent, eOTAmodes updateType)
{
  ScreenManager.showOTAMessage(percent, updateType);
  feedWatchdog();
}

void feedWatchdog()
{
  uint64_t timeRem = timerRead(watchdogTimer);
  if(timeRem > 500000)  // 500ms
    DebugPort.printf("\007WD time = %lld\r\n", timeRem);  // print longer WD intervals

  timerWrite(watchdogTimer, 0); //reset timer (feed watchdog)  
}


void doStreaming() 
{
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

  GPIOin.manage();
  GPIOout.manage();
  GPIOalg.manage();

  // manage changes in Bluetooth connection status
  if(Bluetooth.isConnected()) {
    if(!bBTconnected) {
      resetJSONmoderator();  // force full send upon BT client connect
    }
    bBTconnected = true;
  }
  else {
    bBTconnected = false;
  }
  // manage changes in number of wifi clients
  if(isWebServerClientChange()) {
    resetJSONmoderator();  // force full send upon number of Wifi clients change
  }

  DebugPort.handle();    // keep telnet spy alive

}

void getGPIOinfo(sGPIO& info)
{
  info.inState[0] = GPIOin.getState(0);
  info.inState[1] = GPIOin.getState(1);
  info.outState[0] = GPIOout.getState(0);
  info.outState[1] = GPIOout.getState(1);
  info.algVal = GPIOalg.getValue();
  info.inMode = GPIOin.getMode();
  info.outMode = GPIOout.getMode();
  info.algMode = GPIOalg.getMode();
}

// hook for JSON input, simulating a GPIO key press
void simulateGPIOin(uint8_t newKey)   
{
  GPIOin.simulateKey(newKey);
}