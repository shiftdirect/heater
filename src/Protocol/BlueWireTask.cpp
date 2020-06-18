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

#include "BlueWireTask.h"
#include "../cfg/BTCConfig.h"
#include "../cfg/pins.h"
#include "Protocol.h"
#include "TxManage.h"
// #include "SmartError.h"
#include "../Utility/UtilClasses.h"
#include "../Utility/DataFilter.h"
#include "../Utility/FuelGauge.h"
#include "../Utility/HourMeter.h"
#include "../Utility/macros.h"

// Setup Serial Port Definitions
// for ESP32, Mega
// HardwareSerial is it for these boards
static HardwareSerial& BlueWireSerial(Serial1);  

CBlueWireCommsTask BlueWireCommsTask;  // AltCommsTaskInfo;

#define RX_DATA_TIMOUT 50

CommStates CommState;
CTxManage TxManage(TxEnbPin, BlueWireSerial);
CProtocol DefaultBTCParams(CProtocol::CtrlMode);  // defines the default parameters, used in case of no OEM controller
CModeratedFrame OEMCtrlFrame;        // data packet received from heater in response to OEM controller packet
CModeratedFrame HeaterFrame1;        // data packet received from heater in response to OEM controller packet
CProtocol HeaterFrame2;              // data packet received from heater in response to our packet 
// CSmartError SmartError;
CProtocolPackage reportHeaterData;
CProtocolPackage primaryHeaterData;
char dbgMsg[COMMS_MSGQUEUESIZE];

static bool bHasOEMController = false;
static bool bHasOEMLCDController = false;
static bool bHasHtrData = false;

extern bool bReportRecyleEvents;
extern bool bReportOEMresync;
extern bool bReportBlueWireData;
extern sFilteredData FilteredSamples;

// QueueHandle_t BlueWireMsgQueue = NULL;    // cannot use general Serial.print etc from this task without causing conflicts
// QueueHandle_t BlueWireRxQueue = NULL;   // queue to pass down heater receive data
// QueueHandle_t BlueWireTxQueue = NULL;   // queue to pass down heater transmit data
// SemaphoreHandle_t BlueWireSemaphore = NULL;  // flag to indicate completion of heater data exchange

bool validateFrame(const CProtocol& frame, const char* name);
void DebugReportFrame(const char* hdr, const CProtocol& Frame, const char* ftr, char* msg);
// void updateFilteredData();
void initBlueWireSerial();

/*void pushDebugMsg(const char* msg) {
  if(BlueWireMsgQueue)
    xQueueSend(BlueWireMsgQueue, msg, 0);
}*/
void pushDebugMsg(const char* msg) {
  BlueWireCommsTask.putMsgQueue(msg);
}


CBlueWireCommsTask::CBlueWireCommsTask()
{

}

void 
CBlueWireCommsTask::commsTask(void* arg) 
{
  //////////////////////////////////////////////////////////////////////////////////////
  // Blue wire data reception
  //  Reads data from the "blue wire" Serial port, (to/from heater)
  //  If an OEM controller exists we will also see it's data frames
  //  Note that the data is read now, then held for later use in the state machine
  //

  CBlueWireCommsTask* pThis = (CBlueWireCommsTask*)arg;

  pThis->_task();  // loops here until terminated

  vTaskDelete(NULL);   // NEVER fall out from a task!
  for(;;);

}

void 
CBlueWireCommsTask::_task() 
{
  static unsigned long lastRxTime = 0;                     // used to observe inter character delays
  static unsigned long moderator = 50;
  bool isBTCmaster = false;

  // create FreeRTOS queues etc
  create(BLUEWIRE_DATAQUEUESIZE);
  // BlueWireMsgQueue = xQueueCreate(4, BLUEWIRE_MSGQUEUESIZE);
  // BlueWireRxQueue = xQueueCreate(4, BLUEWIRE_DATAQUEUESIZE);
  // BlueWireTxQueue = xQueueCreate(4, BLUEWIRE_DATAQUEUESIZE);
  // BlueWireSemaphore = xSemaphoreCreateBinary();

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

  _initSerial();
  // initBlueWireSerial();

  CommState.setCallback(pushDebugMsg);
  TxManage.setCallback(pushDebugMsg);

  // for(;;) {
  _runState = 1;
  while(_runState == 1) {

    sRxData BlueWireRxData;
    unsigned long timenow = millis();

    // calc elapsed time since last rxd byte
    // used to detect no OEM controller, or the start of an OEM frame sequence
    unsigned long RxTimeElapsed = timenow - lastRxTime;

    if (BlueWireSerial.available()) {
      // Data is available, read and store it now, use it later
      // Note that if not in a recognised data receive frame state, the data 
      // will be deliberately lost!
      BlueWireRxData.setValue(BlueWireSerial.read());  // read hex byte, store for later use
        
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
            sprintf(dbgMsg, "%ldms - ", RxTimeElapsed);
            pushDebugMsg(dbgMsg);
          }
          if(CommState.is(CommStates::OEMCtrlRx)) {
            bHasOEMController = false;
            bHasOEMLCDController = false;
            if(bReportRecyleEvents) {
              pushDebugMsg("Timeout collecting OEM controller data, returning to Idle State\r\n");
            }
          }
          else if(CommState.is(CommStates::HeaterRx1)) {
            bHasHtrData = false;
            if(bReportRecyleEvents) {
              pushDebugMsg("Timeout collecting OEM heater response data, returning to Idle State\r\n");
            }
          }
          else {
            bHasHtrData = false;
            if(bReportRecyleEvents) {
              pushDebugMsg("Timeout collecting BTC heater response data, returning to Idle State\r\n");
            }
          }
        }

        if(bReportRecyleEvents) {
          pushDebugMsg("Recycling blue wire serial interface\r\n");
        }
  #ifdef REBOOT_BLUEWIRE
        initBlueWireSerial();
  #endif
        CommState.set(CommStates::ExchangeComplete);    // revert to idle mode, after passing thru exchange complete mode
      }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    // do our state machine to track the reception and delivery of blue wire data

    switch(CommState.get()) {

      case CommStates::Idle:

        moderator = 50;  

        digitalWrite(LED_Pin, LOW);
        // Detect the possible start of a new frame sequence from an OEM controller
        // This will be the first activity for considerable period on the blue wire
        // The heater always responds to a controller frame, but otherwise never by itself
        
        if(RxTimeElapsed >= (NVstore.getUserSettings().FrameRate - 60)) {  // compensate for the time spent just doing things in this state machine
          // have not seen any receive data for a second (typ.).
          // OEM controller is probably not connected. 
          // Skip state machine immediately to BTC_Tx, sending our own settings.
          bHasHtrData = false;
          bHasOEMController = false;
          bHasOEMLCDController = false;
          isBTCmaster = true;
          TxManage.PrepareFrame(DefaultBTCParams, isBTCmaster);  // use our parameters, and mix in NV storage values
          TxManage.Start(timenow);
          CommState.set(CommStates::TxStart);
          break;
        } 

        if(BlueWireRxData.available() && (RxTimeElapsed > (RX_DATA_TIMOUT+10))) {  

          if(bReportOEMresync) {
            sprintf(dbgMsg, "Re-sync'd with OEM Controller. %ldms Idle time.\r\n", RxTimeElapsed);
            pushDebugMsg(dbgMsg);
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
          break;  // only break if we fail all Idle state tests
        }


      case CommStates::OEMCtrlRx:

        digitalWrite(LED_Pin, HIGH);

        // collect OEM controller frame
        if(BlueWireRxData.available()) {
        digitalWrite(LED_Pin, LOW);
        digitalWrite(LED_Pin, HIGH);
          if(CommState.collectData(OEMCtrlFrame, BlueWireRxData.getValue()) ) {
            CommState.set(CommStates::OEMCtrlValidate);  // collected 24 bytes, move on!
          }
        }
        break;


      case CommStates::OEMCtrlValidate:

        digitalWrite(LED_Pin, LOW);

        // test for valid CRC, abort and restarts Serial1 if invalid
        if(!validateFrame(OEMCtrlFrame, "OEM")) {
          break;
        }

        // filled OEM controller frame
        OEMCtrlFrame.setTime();
        // LCD controllers use 0x76 as first byte, rotary knobs use 0x78
        bHasOEMLCDController = (OEMCtrlFrame.Controller.Byte0 != 0x78);

        // xQueueSend(BlueWireTxQueue, OEMCtrlFrame.Data, 0);

        CommState.set(CommStates::HeaterRx1);
        break;


      case CommStates::HeaterRx1:

        digitalWrite(LED_Pin, HIGH);

        // collect heater frame, always in response to an OEM controller frame
        if(BlueWireRxData.available()) {
        digitalWrite(LED_Pin, LOW);
        digitalWrite(LED_Pin, HIGH);
          if( CommState.collectData(HeaterFrame1, BlueWireRxData.getValue()) ) {
            CommState.set(CommStates::HeaterValidate1);
          }
        }
        break;


      case CommStates::HeaterValidate1:
        
        digitalWrite(LED_Pin, LOW);

        // test for valid CRC, abort and restarts Serial1 if invalid
        if(!validateFrame(HeaterFrame1, "RX1")) {
          bHasHtrData = false;
          break;
        }
        bHasHtrData = true;

        HeaterFrame1.setTime();

        while(BlueWireSerial.available()) {
          pushDebugMsg("DUMPED ROGUE RX DATA\r\n");
          BlueWireSerial.read();
        }
        BlueWireSerial.flush();

        // received heater frame (after controller message), report
        primaryHeaterData.set(HeaterFrame1, OEMCtrlFrame);  // OEM is always *the* controller
        if(bReportBlueWireData) {
          primaryHeaterData.reportFrames(true, pushDebugMsg);
        }
        isBTCmaster = false;
        TxManage.PrepareFrame(OEMCtrlFrame, isBTCmaster);  // parrot OEM parameters, but block NV modes
        CommState.set(CommStates::TxStart);
        break;


      case CommStates::TxStart:
        xQueueSend(_txQueue, TxManage.getFrame().Data, 0);
        TxManage.Start(timenow);
        CommState.set(CommStates::TxInterval);
        break;
      

      case CommStates::TxInterval:
        // Handle time interval where we send data to the blue wire
        lastRxTime = timenow;                     // *we* are pumping onto blue wire, track this activity!
        if(TxManage.CheckTx(timenow) ) {          // monitor progress of our data delivery
          CommState.set(CommStates::HeaterRx2);   // then await heater repsonse
        }
        break;


      case CommStates::HeaterRx2:

        digitalWrite(LED_Pin, HIGH);

        // collect heater frame, in response to our control frame
        if(BlueWireRxData.available()) {
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
          if( CommState.collectData(HeaterFrame2, BlueWireRxData.getValue()) ) {
            CommState.set(CommStates::HeaterValidate2);
          }
#endif
        } 
        break;


      case CommStates::HeaterValidate2:

        digitalWrite(LED_Pin, LOW);

        // test for valid CRC, abort and restart Serial1 if invalid
        if(!validateFrame(HeaterFrame2, "RX2")) {
          bHasHtrData = false;
          break;
        }
        bHasHtrData = true;

        // received heater frame (after our control message), report

        xQueueSend(_rxQueue, HeaterFrame2.Data, 0);
        _online = true;

        // do some monitoring of the heater state variables
        // if abnormal transitions, introduce a smart error!
        // SmartError.monitor(HeaterFrame2);

        if(!bHasOEMController)              // no OEM controller - BTC is *the* controller
          primaryHeaterData.set(HeaterFrame2, TxManage.getFrame());
        
        if(bReportBlueWireData) {  // debug or investigation purposes
          reportHeaterData.set(HeaterFrame2, TxManage.getFrame());
          reportHeaterData.reportFrames(false, pushDebugMsg);
        }
        CommState.set(CommStates::ExchangeComplete);
        break;


      case CommStates::ExchangeComplete:
        xSemaphoreGive(_semaphore);
        CommState.set(CommStates::Idle);
        break;
    }  // switch(CommState)

#if DBG_FREERTOS == 1
    digitalWrite(GPIOout1_pin, LOW);
#endif
    if (!BlueWireSerial.available()) {
       vTaskDelay(1);
    }
#if DBG_FREERTOS == 1
    digitalWrite(GPIOout1_pin, HIGH);
#endif
  }

  BlueWireSerial.end();

  // return pins to standard GPIO functions
  pinMode(Tx1Pin, OUTPUT);  
  pinMode(Rx1Pin, INPUT_PULLUP);  // required for MUX to work properly
  pinMode(TxEnbPin, OUTPUT);
  digitalWrite(Tx1Pin, HIGH);
  digitalWrite(TxEnbPin, LOW);

  _runState = 0;
 
  DebugPort.println("Blue wire task concluded");
}


bool validateFrame(const CProtocol& frame, const char* name)
{
  if(!frame.verifyCRC(pushDebugMsg)) {
    // Bad CRC - restart blue wire Serial port
    sprintf(dbgMsg, "\007Bad CRC detected for %s frame - restarting blue wire's serial port\r\n", name);
    pushDebugMsg(dbgMsg);
    dbgMsg[0] = 0;  // empty string
    DebugReportFrame("BAD CRC:", frame, "\r\n", dbgMsg);
    pushDebugMsg(dbgMsg);
#ifdef REBOOT_BLUEWIRE
    initBlueWireSerial();
#endif
    CommState.set(CommStates::ExchangeComplete);
    return false;
  }
  return true;
}


void DebugReportFrame(const char* hdr, const CProtocol& Frame, const char* ftr, char* msg)
{
  strcat(msg, hdr);                     // header
  for(int i=0; i<24; i++) {
    char str[8];
    sprintf(str, " %02X", Frame.Data[i]);  // build 2 dig hex values
    strcat(msg, str);                   // and print     
  }
  strcat(msg, ftr);                     // footer
}

bool hasOEMcontroller()
{
  return bHasOEMController;
}

bool hasOEMLCDcontroller()
{
  return bHasOEMLCDController;
}

bool hasHtrData()
{
  return bHasHtrData;
}

void 
CBlueWireCommsTask::_initSerial()
{
  // initialize serial port to interact with the "blue wire"
  // 25000 baud, Tx and Rx channels of Chinese heater comms interface:
  // Tx/Rx data to/from heater, 
  // Note special baud rate for Chinese heater controllers
  // ESP32
  BlueWireSerial.begin(25000, SERIAL_8N1, Rx1Pin, Tx1Pin);  // need to explicitly specify pins for pin multiplexer!
  pinMode(Rx1Pin, INPUT_PULLUP);  // required for MUX to work properly
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

void reqPumpPrime(bool on)
{
  TxManage.reqPrime(on);
}

void
CBlueWireCommsTask::taskStart()
{
  CCommsTask::taskStart();
  _runState = 0;           
  xTaskCreate(commsTask,              
              "BlueWireTask",
              1600,
              this,
              TASK_PRIORITY_HEATERCOMMS,
              &_taskHandle);
}