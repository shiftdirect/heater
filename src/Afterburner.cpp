/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2019  Ray Jones <ray@mrjones.id.au>
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

#include "WiFi/ABMQTT.h"
#include "cfg/BTCConfig.h"
#include "cfg/pins.h"
#include "RTC/Timers.h"
#include "RTC/Clock.h"
#include "RTC/RTCStore.h"
#include "WiFi/BTCWifi.h"
#include "WiFi/BTCWebServer.h"
#include "WiFi/BTCota.h"
#include "Protocol/Protocol.h"
#include "Protocol/TxManage.h"
#include "Protocol/SmartError.h"
#include "Utility/helpers.h" 
#include "Utility/NVStorage.h"
#include "Utility/DebugPort.h"
#include "Utility/macros.h"
#include "Utility/UtilClasses.h"
#include "Utility/BTC_JSON.h"
#include "Utility/BTC_GPIO.h"
#include "Utility/BoardDetect.h"
#include "Utility/FuelGauge.h"
#include "OLED/ScreenManager.h"
#include "OLED/KeyPad.h"
#include "Utility/TempSense.h"
#include "Utility/DataFilter.h"
#include "Utility/HourMeter.h"
#include <rom/rtc.h>
#include <esp_spiffs.h>
#include <SPIFFS.h>
#include <nvs.h>
#include "Utility/MQTTsetup.h"
#include <FreeRTOS.h>
#include "RTC/TimerManager.h"
#include "Utility/GetLine.h"

// SSID & password now stored in NV storage - these are still the default values.
//#define AP_SSID "Afterburner"
//#define AP_PASSWORD "thereisnospoon"

#define RX_DATA_TIMOUT 50

const int FirmwareRevision = 32;
const int FirmwareSubRevision = 0;
const int FirmwareMinorRevision = 3;
const char* FirmwareDate = "9 Apr 2020";


#ifdef ESP32
#include "Bluetooth/BluetoothESP32.h"
#else
#include "Bluetooth/BluetoothHC05.h"
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
void manageFrostMode();
void manageHumidity();
bool preemptCyclicMode();
void doStreaming();
void heaterOn();
void heaterOff();
void updateFilteredData();
bool HandleMQTTsetup(char rxVal);
void showMainmenu();

// DS18B20 temperature sensor support
// Uses the RMT timeslot driver to operate as a one-wire bus
//CBME280Sensor BMESensor;
CTempSense TempSensor;
long lastTemperatureTime;            // used to moderate DS18B20 access
int DS18B20holdoff = 2;

int BoardRevision = 0;
bool bTestBTModule = false;
bool bSetupMQTT = false;

unsigned long lastAnimationTime;     // used to sequence updates to LCD for animation

sFilteredData FilteredSamples;
CommStates CommState;
CTxManage TxManage(TxEnbPin, BlueWireSerial);
CModeratedFrame OEMCtrlFrame;        // data packet received from heater in response to OEM controller packet
CModeratedFrame HeaterFrame1;        // data packet received from heater in response to OEM controller packet
CProtocol HeaterFrame2;              // data packet received from heater in response to our packet 
CProtocol DefaultBTCParams(CProtocol::CtrlMode);  // defines the default parameters, used in case of no OEM controller
CSmartError SmartError;
CKeyPad KeyPad;
CScreenManager ScreenManager;
ABTelnetSpy DebugPort;
#if USE_JTAG == 0
//CANNOT USE GPIO WITH JTAG DEBUG
CGPIOin GPIOin;
CGPIOout GPIOout;
CGPIOalg GPIOalg;
#endif

CMQTTsetup MQTTmenu;



long lastRxTime;                     // used to observe inter character delays
bool bHasOEMController = false;
bool bHasOEMLCDController = false;
bool bHasHtrData = false;

// these variables will persist over a soft reboot.
__NOINIT_ATTR float persistentRunTime;
__NOINIT_ATTR float persistentGlowTime;

CFuelGauge FuelGauge; 
CRTC_Store RTC_Store;
CHourMeter* pHourMeter = NULL;

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
long BootTime;

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

unsigned long WatchdogTick = -1;
unsigned long JSONWatchdogTick = -1;

void WatchdogTask(void * param)
{
  for(;;) {
    if(WatchdogTick >= 0) {
      if(WatchdogTick == 0) {
        interruptReboot();
      }
      else {
        WatchdogTick--;
      }
    }
    if(JSONWatchdogTick >= 0) {
      if(JSONWatchdogTick == 0) {
        interruptReboot();
      }
      else {
        JSONWatchdogTick--;
      }
    }
    vTaskDelay(10);
  }
}


//**************************************************************************************************
//**                                                                                              **
//**         WORKAROUND for crap ESP32 millis() standard function                                 **
//**                                                                                              **
//**************************************************************************************************
//
// Substitute shitfull ESP32 millis() with a true and proper ms counter
// The standard millis() on ESP32 is actually micros()/1000.
// This wraps every 71.5 minutes in a **very non linear fashion**.
//
// The FreeRTOS Tick Counter however does increment each ms, and rolls naturally past 0 every 49days.
// With this proper linear behaviour you can use valid timeout calcualtions even through wrap around.
// This elegance breaks using the standard library function, leading to many weird and obtuse issues.
//
// *** IMPORTANT ***
//
// You **MUST** use --wrap millis in the linker command, or -Wl,--wrap,millis in the GCC command.
// platformio.ini file for this project defines the latter as a build_flags entry.
//
// The linker will now link to __wrap_millis() instead of millis() for *any* usage of millis().
// Best of all this includes any library usages of millis() :-D
// If you really must call the shitty ESP32 Arduino millis(), you must call __real_millis() 
// from your dubious code ;-) - basically DON'T do this.

extern "C" unsigned long __wrap_millis() {
  return xTaskGetTickCount();
}


void setup() {

  // ensure cyclic mode is disabled after power on
  bool bESP32PowerUpInit = false;
  if(rtc_get_reset_reason(0) == 1/* || bForceInit*/) {
    bESP32PowerUpInit = true;
//    bForceInit = false;
  }
  
  // initially, ensure the GPIO outputs are not activated during startup
  // (GPIO2 tends to be one with default chip startup)
#if USE_JTAG == 0
  //CANNOT USE GPIO WITH JTAG DEBUG
  pinMode(GPIOout1_pin, OUTPUT);  
  pinMode(GPIOout2_pin, OUTPUT);  
  digitalWrite(GPIOout1_pin, LOW);
  digitalWrite(GPIOout2_pin, LOW);
#endif

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
  
  DebugPort.printf("Getting NVS stats\r\n");

  nvs_stats_t nvs_stats;
  while( nvs_get_stats(NULL, &nvs_stats) == ESP_ERR_NVS_NOT_INITIALIZED);

  DebugPort.printf("Reset reason: core0:%d, core1:%d\r\n", rtc_get_reset_reason(0), rtc_get_reset_reason(0));
//  DebugPort.printf("Previous user ON = %d\r\n", bUserON);   // state flag required for cyclic mode to persist properly after a WD reboot :-)

  // initialise DS18B20 sensor interface
  TempSensor.begin(DS18B20_Pin, 0x76);
  TempSensor.startConvert();  // kick off initial temperature sample


  lastTemperatureTime = millis();
  lastAnimationTime = millis();
  
  BoardRevision = BoardDetect();
  DebugPort.printf("Board revision: V%.1f\r\n", float(BoardRevision) * 0.1);

  DebugPort.printf("ESP32 IDF Version: %s\r\n", esp_get_idf_version());
  DebugPort.printf("NVS:  entries- free=%d used=%d total=%d namespace count=%d\r\n", nvs_stats.free_entries, nvs_stats.used_entries, nvs_stats.total_entries, nvs_stats.namespace_count);

 // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    DebugPort.println("An Error has occurred while mounting SPIFFS");
  }
  else {
    DebugPort.println("Mounted SPIFFS OK");
    DebugPort.printf("SPIFFS usage: %d/%d\r\n", SPIFFS.usedBytes(), SPIFFS.totalBytes());
    DebugPort.println("Listing SPIFFS contents:");
    String report;
    listSPIFFS("/", 2, report);
  }

  NVstore.init();
  NVstore.load();
  
  initJSONMQTTmoderator();   // prevents JSON for MQTT unless requested
  initJSONIPmoderator();   // prevents JSON for IP unless requested
  initJSONTimermoderator();  // prevents JSON for timers unless requested
  initJSONSysModerator();

  
  KeyPad.begin(keyLeft_pin, keyRight_pin, keyCentre_pin, keyUp_pin, keyDown_pin);
  KeyPad.setCallback(parentKeyHandler);

  // Initialize the rtc object
  Clock.begin();

  BootTime = Clock.get().secondstime();
  
  ScreenManager.begin();
  if(Clock.lostPower()) {
    ScreenManager.selectMenu(CScreenManager::BranchMenu, CScreenManager::SetClockUI);
  }

#if USE_WIFI == 1


  if(NVstore.getUserSettings().wifiMode) {
    initWifi();   
#if USE_OTA == 1
    if(NVstore.getUserSettings().enableOTA) {
      initOTA();
    }
#endif // USE_OTA
    initFOTA();
#if USE_WEBSERVER == 1
    initWebServer();
#endif // USE_WEBSERVER
#if USE_MQTT == 1
    mqttInit();
#endif // USE_MQTT
  }

#endif // USE_WIFI

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

//  pinMode(0, OUTPUT);
//  digitalWrite(0, LOW);

#if USE_SW_WATCHDOG == 1 && USE_JTAG == 0
  // create a high priority FreeRTOS task as a watchdog monitor
  TaskHandle_t wdTask;
  xTaskCreate(WatchdogTask,
             "watchdogTask",
             2000,
             NULL,
             configMAX_PRIORITIES-1,
             &wdTask);
#endif
  JSONWatchdogTick = -1;
  WatchdogTick = -1;

  FilteredSamples.ipVolts.setRounding(0.1);
  FilteredSamples.GlowAmps.setRounding(0.01);
  FilteredSamples.GlowVolts.setRounding(0.1);
  FilteredSamples.Fan.setRounding(10);
  FilteredSamples.Fan.setAlpha(0.7);
  FilteredSamples.AmbientTemp.reset(-100.0);
  FilteredSamples.FastipVolts.setRounding(0.1);
  FilteredSamples.FastipVolts.setAlpha(0.7);
  FilteredSamples.FastGlowAmps.setRounding(0.01);
  FilteredSamples.FastGlowAmps.setAlpha(0.7);
  
  RTC_Store.begin();
  FuelGauge.init(RTC_Store.getFuelGauge());
//  demandDegC = RTC_Store.getDesiredTemp();
//  demandPump = RTC_Store.getDesiredPump();
//  bCyclicEngaged = RTC_Store.getCyclicEngaged();
  DebugPort.printf("Previous cyclic active = %d\r\n", RTC_Store.getCyclicEngaged());   // state flag required for cyclic mode to persist properly after a WD reboot :-)

  pHourMeter = new CHourMeter(persistentRunTime, persistentGlowTime); // persistent vars passed by reference so they can be valid after SW reboots
  pHourMeter->init(bESP32PowerUpInit || RTC_Store.getBootInit());     // ensure persistent memory variable are reset after powerup, or OTA update
  RTC_Store.setBootInit(false);

  reqDemand(RTC_Store.getDesiredTemp());  // bug fix: was not applying saved set point!

  // Check for solo DS18B20
  // store it's serial number as the primary sensor
  // This allows seamless standard operation, and marks the iniital sensor 
  // as the primary if another is added later
  OneWireBus_ROMCode romCode;
  TempSensor.getDS18B20().getRomCodeIdx(0, romCode);
  if(TempSensor.getDS18B20().getNumSensors() == 1 && 
     memcmp(NVstore.getHeaterTuning().DS18B20probe[0].romCode.bytes, romCode.bytes, 8) != 0) 
  {   
    sHeaterTuning tuning = NVstore.getHeaterTuning();
    tuning.DS18B20probe[0].romCode = romCode;
    tuning.DS18B20probe[1].romCode = {0};
    tuning.DS18B20probe[2].romCode = {0};
    tuning.DS18B20probe[0].offset = 0;
    NVstore.setHeaterTuning(tuning);
    NVstore.save();

    DebugPort.printf("Saved solo DS18B20 %02X:%02X:%02X:%02X:%02X:%02X to NVstore\r\n",
                      romCode.fields.serial_number[5], 
                      romCode.fields.serial_number[4], 
                      romCode.fields.serial_number[3], 
                      romCode.fields.serial_number[2], 
                      romCode.fields.serial_number[1], 
                      romCode.fields.serial_number[0] 
                    );
  }
  TempSensor.getDS18B20().mapSensor(0, NVstore.getHeaterTuning().DS18B20probe[0].romCode);
  TempSensor.getDS18B20().mapSensor(1, NVstore.getHeaterTuning().DS18B20probe[1].romCode);
  TempSensor.getDS18B20().mapSensor(2, NVstore.getHeaterTuning().DS18B20probe[2].romCode);

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

    if(NVstore.getUserSettings().menuMode == 2)
      bReportRecyleEvents = false;

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

      feedWatchdog(); // feed watchdog
      
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
      if(BlueWireData.available() && (RxTimeElapsed > (RX_DATA_TIMOUT+10))) {  

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
        bool isBTCmaster = false;
        TxManage.PrepareFrame(OEMCtrlFrame, isBTCmaster);  // parrot OEM parameters, but block NV modes
        TxManage.Start(timenow);
        CommState.set(CommStates::BTC_Tx);
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
      if(tDelta > MIN_TEMPERATURE_INTERVAL) {  // maintain a minimum holdoff period
        lastTemperatureTime = millis();    // reset time to observe temeprature        

        TempSensor.readSensors();
        if(TempSensor.getTemperature(0, fTemperature)) {  // get Primary sensor temperature
          if(DS18B20holdoff) {
            DS18B20holdoff--; 
            DebugPort.printf("Skipped initial DS18B20 reading: %f\r\n", fTemperature);
          }                           // first value upon sensor connect is bad
          else {
            // exponential mean to stabilse readings
            FilteredSamples.AmbientTemp.update(fTemperature);

            manageCyclicMode();
            manageFrostMode();
            manageHumidity();
          }
        }
        else {
          DS18B20holdoff = 3;
          FilteredSamples.AmbientTemp.reset(-100.0);
        }

        TempSensor.startConvert();  // request a new conversion, will be ready by the time we loop back around

        ScreenManager.reqUpdate();
      }

      if(bHasHtrData) {
        // apply exponential mean to the anlogue readings for some smoothing
        updateFilteredData();

        // integrate fuel pump activity for fuel gauge
        FuelGauge.Integrate(getHeaterInfo().getPump_Actual());

        // test for low volts shutdown during normal run
        if(INBOUNDS(getHeaterInfo().getRunState(), 1, 5)) {  // check for Low Voltage Cutout
          SmartError.checkVolts(FilteredSamples.FastipVolts.getValue(), FilteredSamples.FastGlowAmps.getValue());
          SmartError.checkfuelUsage();
        }

        // trap being in state 0 with a heater error - cancel user on memory to avoid unexpected cyclic restarts
        if(RTC_Store.getCyclicEngaged() && (getHeaterInfo().getRunState() == 0) && (getHeaterInfo().getErrState() > 1)) {
          DebugPort.println("Forcing cyclic cancel due to error induced shutdown");
          RTC_Store.setCyclicEngaged(false);
        }

        pHourMeter->monitor(HeaterFrame2);
      }
      updateJSONclients(bReportJSONData);
      updateMQTT();
      CommState.set(CommStates::Idle);
      NVstore.doSave();   // now is a good time to store to the NV storage, well away from any blue wire activity
      break;
  }  // switch(CommState)

  BlueWireData.reset();   // ensure we flush any used data

// 21/11/19 vTaskDelay() causes E-07 errors when OEM controller is attached.
// may look at a specific freertos task to handle the blue wire....
  if(!bHasOEMController) {
    vTaskDelay(1);  // give up for now - allow power lowering...
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

void manageCyclicMode()
{
  const sCyclicThermostat& cyclic = NVstore.getUserSettings().cyclic;
  if(cyclic.Stop && RTC_Store.getCyclicEngaged()) {   // cyclic mode enabled, and user has started heater
    int stopDeltaT = cyclic.Stop + 1;  // bump up by 1 degree - no point invoking at 1 deg over!
    float deltaT = getTemperatureSensor() - getDemandDegC();
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


void manageFrostMode()
{
  uint8_t engage = NVstore.getUserSettings().FrostOn;
  if(engage) {
    float deltaT = getTemperatureSensor() - engage;
    int heaterState = getHeaterInfo().getRunState();
    if(deltaT < 0) {
      if(heaterState == 0) {
        RTC_Store.setFrostOn(true);        
        DebugPort.printf("FROST MODE: Starting heater, < %d`C\r\n", engage);
        if(NVstore.getUserSettings().FrostRise == 0)
          RTC_Store.setCyclicEngaged(true);    // enable cyclic mode if user stop
        heaterOn();
      }
    }
    uint8_t rise = NVstore.getUserSettings().FrostRise;
    if(rise && (deltaT > rise)) {  // if rise is set to 0, user must shut off heater
      if(RTC_Store.getFrostOn()) {
        DebugPort.printf("FROST MODE: Stopping heater, > %d`C\r\n", engage+rise);
        heaterOff();
        RTC_Store.setFrostOn(false);  // cancel active frost mode
        RTC_Store.setCyclicEngaged(false);   // for cyclic mode
      }
    }
  }
}

void manageHumidity()
{
  uint8_t humidity = NVstore.getUserSettings().humidityStart;
  if(humidity) {
    float reading;
    if(getTempSensor().getHumidity(reading)) {
      uint8_t testval = (uint8_t)reading;
      if(testval > humidity) {
        DebugPort.printf("HUMIDITY MODE: Starting heater, > %d%%\r\n", humidity);
        requestOn();
      }
    }
  }
}

bool preemptCyclicMode()
{
  const sCyclicThermostat& cyclic = NVstore.getUserSettings().cyclic;
  if(cyclic.Stop) {   // cyclic mode enabled, and user has started heater
    int stopDeltaT = cyclic.Stop + 1;  // bump up by 1 degree - no point invoking at 1 deg over!
    float deltaT = getTemperatureSensor() - getDemandDegC();

    // check if over temp, skip straight to suspend
    if(deltaT > stopDeltaT) {
      DebugPort.printf("CYCLIC MODE: Skipping directly to suspend, deltaT > +%d\r\n", stopDeltaT);
      heaterOff();    // over temp - request heater stop
      return true;
    }
  }
  return false;
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


int requestOn(bool checkTemp)
{
  DebugPort.println("Start Request!");
  bool fuelOK = 2 != SmartError.checkfuelUsage();
  if(!fuelOK) {
    return -4;
  }
  bool LVCOK = 2 != SmartError.checkVolts(FilteredSamples.FastipVolts.getValue(), FilteredSamples.FastGlowAmps.getValue());
  if(bHasHtrData && LVCOK) {
    RTC_Store.setCyclicEngaged(true);    // for cyclic mode
    RTC_Store.setFrostOn(false);  // cancel frost mode
    if(!preemptCyclicMode()) {    // only start if below cyclic threshold when enabled
      if(!checkTemp || getTemperatureSensor() < getDemandDegC())  { // skip start if warmer than desired
        heaterOn();
        return 0;
      }
      else {
        return -1;   // too warm
      }
    }
    else {
      return -2;   // immediate cyclic suspend
    }
  }
  else {
    return -3;   // LVC
  }
}

void requestOff()
{
  DebugPort.println("Stop Request!");
  heaterOff();
  RTC_Store.setCyclicEngaged(false);   // for cyclic mode
  RTC_Store.setFrostOn(false);  // cancel active frost mode
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


bool reqDemand(uint8_t newDemand, bool save)
{
  if(bHasOEMController)
    return false;

  uint8_t max = DefaultBTCParams.getTemperature_Max();
  uint8_t min = DefaultBTCParams.getTemperature_Min();
  if(newDemand >= max)
    newDemand = max;
  if(newDemand <= min)
    newDemand = min;
  
  // set and save the demand to NV storage
  // note that we now maintain fixed Hz and Thermostat set points seperately
  if(getThermostatModeActive()) {
    CTimerManager::setWorkingTemperature(newDemand);
  }
  else {
    RTC_Store.setDesiredPump(newDemand);
  }

  ScreenManager.reqUpdate();
  return true;
}

bool reqDemandDelta(int delta)
{
  uint8_t newDemand;
  if(getThermostatModeActive()) {
    newDemand = CTimerManager::getWorkingTemperature() + delta;
  }
  else {
    newDemand = RTC_Store.getDesiredPump() + delta;
  }

  return reqDemand(newDemand);
}

bool reqThermoToggle()
{
  return setThermostatMode(getThermostatModeActive() ? 0 : 1);
}

bool setThermostatMode(uint8_t val)
{
  if(bHasOEMController)
    return false;

  sUserSettings settings = NVstore.getUserSettings();
  if(INBOUNDS(val, 0, 1))
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

bool getExternalThermostatModeActive()
{
#if USE_JTAG == 0
  return GPIOin.usesExternalThermostat() && (NVstore.getUserSettings().ThermostatMethod == 3);
#else
  //CANNOT USE GPIO WITH JTAG DEBUG
  return false;
#endif
}

bool getExternalThermostatOn()
{
#if USE_JTAG == 0
  return GPIOin.getState(1);
#else
  //CANNOT USE GPIO WITH JTAG DEBUG
  return false;
#endif
}

const char* getExternalThermostatHoldTime()
{
#if USE_JTAG == 0
  return GPIOin.getExtThermHoldTime();
#else
  //CANNOT USE GPIO WITH JTAG DEBUG
  return "00:00";
#endif
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
  RTC_Store.setBootInit();
}

uint8_t getDemandDegC() 
{
  return CTimerManager::getWorkingTemperature();
}

void  setDemandDegC(uint8_t val) 
{
  uint8_t max = DefaultBTCParams.getTemperature_Max();
  uint8_t min = DefaultBTCParams.getTemperature_Min();
  BOUNDSLIMIT(val, min, max);
  CTimerManager::setWorkingTemperature(val);
}

uint8_t getDemandPump() 
{
  return RTC_Store.getDesiredPump();
}


float getTemperatureDesired()
{
  if(bHasOEMController) {
    return getHeaterInfo().getHeaterDemand();
  }
  else {
    if(getThermostatModeActive()) 
      return CTimerManager::getWorkingTemperature();
    else 
      return RTC_Store.getDesiredPump();
  }
}

float getTemperatureSensor(int source)
{
  float retval;
  TempSensor.getTemperature(source, retval);
  return retval;

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

void  setFanMin(uint16_t cVal)
{
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  if(INBOUNDS(cVal, 500, 5000))
    tuning.Fmin = cVal;
  NVstore.setHeaterTuning(tuning);
}

void  setFanMax(uint16_t cVal)
{
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  if(INBOUNDS(cVal, 500, 5000))
    tuning.Fmax = cVal;
  NVstore.setHeaterTuning(tuning);
}

void setFanSensor(uint8_t cVal)
{
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  if(INBOUNDS(cVal, 1, 2))
    tuning.fanSensor = cVal;
  NVstore.setHeaterTuning(tuning);
}

void setSystemVoltage(float val) {
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  tuning.setSysVoltage(val);
  NVstore.setHeaterTuning(tuning);
}

void setGlowDrive(uint8_t val) {
  sHeaterTuning tuning = NVstore.getHeaterTuning();
  if(INBOUNDS(val, 1, 6))
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
  static uint8_t nGetString = 0;
  static uint8_t nGetConf = 0;
  static String pw1;
  static String pw2;
  static CGetLine line;

  // check for test commands received over Debug serial port or telnet
  char rxVal;
  if(DebugPort.getch(rxVal)) {

#ifdef PROTOCOL_INVESTIGATION    
    static int mode = 0;6
    static int val = 0;
#endif

    if(bTestBTModule) {
      bTestBTModule = Bluetooth.test(rxVal);
      return;
    }
    if(MQTTmenu.Handle(rxVal)) {
      if(rxVal == 0) {
        showMainmenu();
      }
      return;
    }

    if(nGetConf) {
      DebugPort.print(rxVal);
      bool bSave = (rxVal == 'y') || (rxVal == 'Y');
      DebugPort.println("");
      if(!bSave) {
        DebugPort.println(" ABORTED!");
        nGetConf = 0;
        return;
      }
      switch(nGetConf) {
        case 1: 
          setSSID(line.getString());  
          break;
        case 2:
          setAPpassword(pw2.c_str());
          break;
      }
      nGetConf = 0;
      return;
    }
    else if(nGetString) {
      DebugPort.enable(true);

      if(rxVal == 0x1b) {  // ESCAPE
        nGetString = 0;
        DebugPort.println("\r\nABORTED!");
        return;
      }

      if(line.handle(rxVal)) {
        switch(nGetString) {
          case 1:  
            if(line.getLen() <= 31) {
              nGetConf = 1; 
              DebugPort.printf("\r\nSet AP SSID to %s? (y/n) - ", line.getString());
            }
            else {
              DebugPort.println("\r\nNew name is longer than 31 characters - ABORTING");
            }
            nGetString = 0;
            return;
          case 2:
            pw1 = line.getString();
            pw2 = NVstore.getCredentials().APpassword;
            if(pw1 != pw2) {
              DebugPort.println("\r\nPassword does not match existing - ABORTING");
              nGetString = 0;
            }
            else {
              nGetString = 3;
              DebugPort.print("\r\nPlease enter new password - ");
              DebugPort.enable(false);  // block other debug msgs whilst we get the password
            }
            line.reset();
            line.maskEntry();
            return;
          case 3:
            pw1 = line.getString();
            if(line.getLen() < 8) {
              // ABORT - too short
              DebugPort.println("\r\nNew password must be at least 8 characters - ABORTING");
              nGetString = 0;
            }
            else if(line.getLen() > 31) {
              // ABORT - too long!
              DebugPort.println("\r\nNew password is longer than 31 characters - ABORTING");
              nGetString = 0;
            }
            else {
              nGetString = 4;
              DebugPort.print("\r\nPlease confirm new password - ");
              DebugPort.enable(false);  // block other debug msgs whilst we get the password
            }
            line.reset();
            line.maskEntry();
            return;
          case 4:
            pw2 = line.getString();
            line.reset();
            if(pw1 != pw2) {
              DebugPort.println("\r\nNew passwords do not match - ABORTING");
            }
            else {
              nGetConf = 2;
              DebugPort.print("\r\nSet new password (y/n) - ");
            }
            nGetString = 0;
            return;
        }
      }
      DebugPort.enable(false);
      return;

    }

    rxVal = toLowerCase(rxVal);

#ifdef PROTOCOL_INVESTIGATION    
    bool bSendVal = false;
#endif
    if(rxVal == '\n') {    // "End of Line"
#ifdef PROTOCOL_INVESTIGATION    
      String convert(line.getString());
      val = convert.toInt();
      bSendVal = true;
      line.reset();
#endif
    }
    else {
      if(rxVal == ' ') {   // SPACE to bring up menu
        showMainmenu();
      }
#ifdef PROTOCOL_INVESTIGATION    
      else if(isDigit(rxVal)) {
        line.handle(rxVal);
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
        DebugPort.println("Test unknown bytes MSB");
        mode = 3;
      }
      else if(rxVal == 'a') {
        DebugPort.println("Test unknown bytes LSB");
        mode = 5;
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
      else if(rxVal == 'n') {
        DebugPort.print("Please enter new SSID name for Access Point - ");
        line.reset();
        nGetString = 1;
        DebugPort.enable(false);  // block other debug msgs whilst we get strings
      }
      else if(rxVal == 'm') {
        MQTTmenu.setActive();
      }
      else if(rxVal == 'o')  {
        bReportOEMresync = !bReportOEMresync;
        DebugPort.printf("Toggled OEM resync event reporting %s\r\n", bReportOEMresync ? "ON" : "OFF");
      }
      else if(rxVal == 'p') {
        DebugPort.print("Please enter current AP password - ");
        line.reset();
        line.maskEntry();
        nGetString = 2;
        DebugPort.enable(false);  // block other debug msgs whilst we get strings
      }
      else if(rxVal == 's') {
        CommState.toggleReporting();
      }
      else if(rxVal == '+') {
        TxManage.queueOnRequest();
      }
      else if(rxVal == '-') {
        TxManage.queueOffRequest();
      }
      else if(rxVal == 'h') {
        getWebContent(true);
      }
      else if(rxVal == 'r') {
        ESP.restart();            // reset the esp
      }
      else if(rxVal == ('h' & 0x1f)) {   // CTRL-H hourmeter reset
        pHourMeter->resetHard();
      }
      else if(rxVal == ('b' & 0x1f)) {   // CTRL-B Tst Mdoe: bluetooth module route
        bTestBTModule = !bTestBTModule;
        Bluetooth.test(bTestBTModule ? 0xff : 0x00);  // special enter or leave BT test commands
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
          DefaultBTCParams.Controller.Unknown1_MSB = val & 0xff;     
          break;
        case 4:
          DebugPort.printf("Forced controller command = %d\r\n", val&0xff);
          DefaultBTCParams.Controller.Command = val & 0xff;
          break;
        case 5:
          DefaultBTCParams.Controller.Unknown1_LSB = val & 0xff;     
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
  return RTC_Store.getCyclicEngaged() && NVstore.getUserSettings().cyclic.isEnabled();
}

void setupGPIO()
{
#if USE_JTAG == 1
  //CANNOT USE GPIO WITH JTAG DEBUG
  return;
#else
  if(BoardRevision == 10 || BoardRevision == 20 || BoardRevision == 21 || BoardRevision == 30) {
    // some special considerations for GPIO inputs, depending upon PCB hardware
    // V1.0 PCBs only expose bare inputs, which are pulled high. Active state into ESP32 is LOW. 
    // V2.0+ PCBs use an input transistor buffer. Active state into ESP32 is HIGH (inverted).
    int activePinState = (BoardRevision == 10) ? LOW : HIGH;  
    int Input1 = BoardRevision == 20 ? GPIOin1_pinV20 : GPIOin1_pinV21V10;
    GPIOin.begin(Input1, 
                 GPIOin2_pin, 
                 NVstore.getUserSettings().GPIO.in1Mode, 
                 NVstore.getUserSettings().GPIO.in2Mode, 
                 activePinState);

    // GPIO out is always active high from ESP32
    // V1.0 PCBs only expose the bare pins
    // V2.0+ PCBs provide an open collector output that conducts when active
    GPIOout.begin(GPIOout1_pin, 
                  GPIOout2_pin, 
                  NVstore.getUserSettings().GPIO.out1Mode, 
                  NVstore.getUserSettings().GPIO.out2Mode);
    GPIOout.setThresh(NVstore.getUserSettings().GPIO.thresh[0], 
                      NVstore.getUserSettings().GPIO.thresh[1]);

    // ### MAJOR ISSUE WITH ADC INPUTS ###
    //
    // V2.0 PCBs that have not been modified connect the analogue input to GPIO26.
    // This is ADC2 channel (#9). 
    // Unfortunately it was subsequently discovered that any ADC2 input cannot be 
    // used if Wifi is enabled. 
    // THIS ISSUE IS NOT RESOLVABLE IN SOFTWARE.
    // *** It is not possible to use ANY of the 10 ADC2 channels if Wifi is enabled :-( ***
    //
    // Fix is to cut traces to GPIO33 & GPIO26 and swap the connections.
    // This directs GPIO input1 into GPIO26 and the analogue input into GPIO33 (ADC1_CHANNEL_5)
    // This will be properly fixed in V2.1 PCBs
    //
    // As V1.0 PCBS expose the bare pins, the correct GPIO33 input can be readily chosen.
    CGPIOalg::Modes algMode = NVstore.getUserSettings().GPIO.algMode;
    if(BoardRevision == 20)  
      algMode = CGPIOalg::Disabled;      // force off analogue support in V2.0 PCBs
    GPIOalg.begin(GPIOalg_pin, algMode);
  }
  else {
    // unknown board or forced no GPIO by grounding pin26 - deny all GPIO operation 
    // set all pins as inputs with pull ups
    pinMode(GPIOin2_pin, INPUT_PULLUP);
    pinMode(GPIOin1_pinV21V10, INPUT_PULLUP);
    pinMode(GPIOin1_pinV20, INPUT_PULLUP);
    pinMode(GPIOout1_pin, INPUT_PULLUP);
    pinMode(GPIOout2_pin, INPUT_PULLUP);
    GPIOin.begin(0, 0, CGPIOin1::Disabled, CGPIOin2::Disabled, LOW);            // ensure modes disabled (should already be by constructors)
    GPIOout.begin(0, 0, CGPIOout1::Disabled, CGPIOout2::Disabled);
    GPIOalg.begin(ADC1_CHANNEL_5, CGPIOalg::Disabled);
  }
#endif
}

bool toggleGPIOout(int channel) 
{
#if USE_JTAG == 0
  //CANNOT USE GPIO WITH JTAG DEBUG
  if(channel == 0) {
    if(NVstore.getUserSettings().GPIO.out1Mode == CGPIOout1::User) {
      setGPIOout(channel, !getGPIOout(channel));  // toggle selected GPIO output 
      return true;
    }
  }
  else if(channel == 1) {
    if(NVstore.getUserSettings().GPIO.out2Mode == CGPIOout2::User) {
      setGPIOout(channel, !getGPIOout(channel));  // toggle selected GPIO output 
      return true;
    }
  }
#endif
  return false;
}

bool setGPIOout(int channel, bool state)
{
#if USE_JTAG == 0
  //CANNOT USE GPIO WITH JTAG DEBUG
  if(channel == 0) {
    if(GPIOout.getMode1() != CGPIOout1::Disabled) {
      DebugPort.printf("setGPIO: Output #%d = %d\r\n", channel+1, state);
      GPIOout.setState(channel, state);
      return true;
    }
  }
  else if(channel == 1) {
    if(GPIOout.getMode2() != CGPIOout2::Disabled) {
      DebugPort.printf("setGPIO: Output #%d = %d\r\n", channel+1, state);
      GPIOout.setState(channel, state);
      return true;
    }
  }
#endif
  return false;
}

bool getGPIOout(int channel)
{
#if USE_JTAG == 0
  bool retval = GPIOout.getState(channel);
  DebugPort.printf("getGPIO: Output #%d = %d\r\n", channel+1, retval);
  return retval;
#else
  //CANNOT USE GPIO WITH JTAG DEBUG
  return false;
#endif
}

float getVersion()
{
  return float(FirmwareRevision) * 0.1f + float(FirmwareSubRevision) * .001f;
}

const char* getVersionStr(bool beta) {
  static char vStr[32];
  if(beta) {
    if(FirmwareMinorRevision)
      return "BETA";
    else
      return "";
  }
  else {
    if(FirmwareMinorRevision)
      sprintf(vStr, "V%.1f.%d.%d", float(FirmwareRevision) * 0.1f, FirmwareSubRevision, FirmwareMinorRevision);
    else
      sprintf(vStr, "V%.1f.%d", float(FirmwareRevision) * 0.1f, FirmwareSubRevision);
  }
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
}

void feedWatchdog()
{
#if USE_SW_WATCHDOG == 1 && USE_JTAG == 0
    // BEST NOT USE WATCHDOG WITH JTAG DEBUG :-)
  WatchdogTick = 1500;
#else
  WatchdogTick = -1;
#endif
}

void doJSONwatchdog(int topup)
{
 if(topup) {
   JSONWatchdogTick = topup * 100;
 }
 else {
   JSONWatchdogTick = -1;
 }
}


void doStreaming() 
{
#if USE_WIFI == 1

  if(NVstore.getUserSettings().wifiMode) {
    doWiFiManager();
#if USE_OTA == 1
    doOTA();
#endif // USE_OTA 
#if USE_WEBSERVER == 1
    bHaveWebClient = doWebServer();
#endif //USE_WEBSERVER
#if USE_MQTT == 1
    // most MQTT is managed via callbacks, but need some sundry housekeeping
    doMQTT();
#endif
  }

#endif // USE_WIFI

  checkDebugCommands();

  KeyPad.update();      // scan keypad - key presses handler via callback functions!

  Bluetooth.check();    // check for Bluetooth activity

#if USE_JTAG == 0
  //CANNOT USE GPIO WITH JTAG DEBUG
  GPIOin.manage();
  GPIOout.manage(); 
  GPIOalg.manage();
#endif

  // manage changes in Bluetooth connection status
  if(Bluetooth.isConnected()) {
    if(!bBTconnected) {
      resetAllJSONmoderators();  // force full send upon BT client connect
    }
    bBTconnected = true;
  }
  else {
    bBTconnected = false;
  }
  // manage changes in number of wifi clients
  if(isWebSocketClientChange()) {
    resetAllJSONmoderators();  // force full send upon increase of Wifi clients
  }

  DebugPort.handle();    // keep telnet spy alive

}

void getGPIOinfo(sGPIO& info)
{
#if USE_JTAG == 0
  info.inState[0] = GPIOin.getState(0);
  info.inState[1] = GPIOin.getState(1);
  info.outState[0] = GPIOout.getState(0);
  info.outState[1] = GPIOout.getState(1);
  info.algVal = GPIOalg.getValue();
  info.in1Mode = GPIOin.getMode1();
  info.in2Mode = GPIOin.getMode2();
  info.out1Mode = GPIOout.getMode1();
  info.out2Mode = GPIOout.getMode2();
  info.algMode = GPIOalg.getMode();
#endif
}

// hook for JSON input, simulating a GPIO key press
void simulateGPIOin(uint8_t newKey)   
{
#if USE_JTAG == 0
  GPIOin.simulateKey(newKey);
#endif
}

float getBatteryVoltage(bool fast)
{
#ifdef RAW_SAMPLES
  return getHeaterInfo().getBattVoltage();
#else
  if(fast)
    return FilteredSamples.FastipVolts.getValue();
  else
    return FilteredSamples.ipVolts.getValue();
#endif
}

float getGlowVolts()
{
#ifdef RAW_SAMPLES
	return  getHeaterInfo().getGlow_Voltage();
#else
  return FilteredSamples.GlowVolts.getValue();
#endif
}

float getGlowCurrent()
{
#ifdef RAW_SAMPLES
	return getHeaterInfo().getGlow_Current();
#else
  return FilteredSamples.GlowAmps.getValue();
#endif
}

int getFanSpeed()
{
#ifdef RAW_SAMPLES
	return getHeaterInfo().getFan_Actual();
#else
  return (int)FilteredSamples.Fan.getValue();
#endif
}

void updateFilteredData()
{
  FilteredSamples.ipVolts.update(getHeaterInfo().getBattVoltage());
  FilteredSamples.GlowVolts.update(getHeaterInfo().getGlow_Voltage());
  FilteredSamples.GlowAmps.update(getHeaterInfo().getGlow_Current());
  FilteredSamples.Fan.update(getHeaterInfo().getFan_Actual());
  FilteredSamples.FastipVolts.update(getHeaterInfo().getBattVoltage());
  FilteredSamples.FastGlowAmps.update(getHeaterInfo().getGlow_Current());
}

int sysUptime()
{
  return Clock.get().secondstime() - BootTime;
}

void resetFuelGauge()
{
  FuelGauge.reset();
}

void setSSID(const char* name)
{
  sCredentials creds = NVstore.getCredentials();
  strncpy(creds.APSSID, name, 31);
  creds.APSSID[31] = 0;
  NVstore.setCredentials(creds);
  NVstore.save();
  NVstore.doSave();   // ensure NV storage
  DebugPort.println("Restarting ESP to invoke new network credentials");
  DebugPort.handle();
  delay(1000);
  ESP.restart();
}

void setAPpassword(const char* name)
{
  sCredentials creds = NVstore.getCredentials();
  strncpy(creds.APpassword, name, 31);
  creds.APpassword[31] = 0;
  NVstore.setCredentials(creds);
  NVstore.save();
  NVstore.doSave();   // ensure NV storage
  DebugPort.println("Restarting ESP to invoke new network credentials");
  DebugPort.handle();
  delay(1000);
  ESP.restart();
}


void showMainmenu()
{
  DebugPort.print("\014");
  DebugPort.println("MENU options");
  DebugPort.println("");
  DebugPort.printf("  <B> - toggle raw blue wire data reporting, currently %s\r\n", bReportBlueWireData ? "ON" : "OFF");
  DebugPort.printf("  <J> - toggle output JSON reporting, currently %s\r\n", bReportJSONData ? "ON" : "OFF");
  DebugPort.printf("  <W> - toggle reporting of blue wire timeout/recycling event, currently %s\r\n", bReportRecyleEvents ? "ON" : "OFF");
  DebugPort.printf("  <O> - toggle reporting of OEM resync event, currently %s\r\n", bReportOEMresync ? "ON" : "OFF");        
  DebugPort.printf("  <S> - toggle reporting of state machine transits %s\r\n", CommState.isReporting() ? "ON" : "OFF");        
  DebugPort.printf("  <N> - change AP SSID, currently \"%s\"\r\n", NVstore.getCredentials().APSSID);
  DebugPort.println("  <P> - change AP password");
  DebugPort.println("  <M> - configure MQTT");
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

void reloadScreens()
{
  ScreenManager.reqReload();
}

CTempSense& getTempSensor()
{
  return TempSensor;
}

void reqHeaterCalUpdate()
{
  TxManage.queueSysUpdate();
}
