/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2018  Ray Jones <ray@mrjones.id.au>
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

#include <Arduino.h>
#include "Protocol.h"
#include "../Utility/DebugPort.h"
#include "../Utility/helpers.h"
#include "../cfg/BTCConfig.h"
#include "../Utility/macros.h"
#include "BlueWireTask.h"
#include "SmartError.h"
#include "../Utility/FuelGauge.h"
#include "../Utility/DataFilter.h"
#include "../Utility/HourMeter.h"
#include "../RTC/RTCStore.h"

CProtocol BlueWireRxData;
CProtocol BlueWireTxData;
CProtocolPackage BlueWireData;

void updateFilteredData(CProtocol& HeaterInfo);

const CProtocolPackage& getHeaterInfo()
{
  return BlueWireData;
}

void 
CProtocol::setCRC()
{
  CModBusCRC16 CRCengine;
  setCRC(CRCengine.process(22, Data));
}

void 
CProtocol::setCRC(uint16_t CRC)
{
  Data[22] = (CRC >> 8) & 0xff;   // MSB of CRC in Data[22]
  Data[23] = (CRC >> 0) & 0xff;   // LSB of CRC in Data[23]
}


uint16_t
CProtocol::getCRC() const
{
  uint16_t CRC;
  CRC = Data[22];   // MSB of CRC in Data[22]
  CRC <<= 8;
  CRC |= Data[23];  // LSB of CRC in Data[23]
  return CRC;
}

// return true for CRC match
bool
CProtocol::verifyCRC(std::function<void(const char*)> pushMsg) const
{
  char errmsg[32];
  CModBusCRC16 CRCengine;
  uint16_t CRC = CRCengine.process(22, Data);  // calculate CRC based on first 22 bytes of our data buffer

  uint16_t FrameCRC = getCRC();
  bool bOK = (FrameCRC == CRC);
  if(!bOK) {
    sprintf(errmsg, "verifyCRC FAILED: calc: %04X data: %04X\r\n", CRC, FrameCRC);
    pushMsg(errmsg);
  }
  return bOK;        // does it match the stored values?
}

CProtocol& 
CProtocol::operator=(const CProtocol& rhs)
{
  memcpy(Data, rhs.Data, 24);
  return *this;
}


void 
CProtocol::setFan_Min(uint16_t Speed) 
{
  // Minimum speed set
  Controller.MinFanRPM_MSB = (Speed >> 8) & 0xff;
  Controller.MinFanRPM_LSB = (Speed >> 0) & 0xff;
}

void 
CProtocol::setFan_Max(uint16_t Speed) 
{
  // Minimum speed set
  Controller.MaxFanRPM_MSB = (Speed >> 8) & 0xff;
  Controller.MaxFanRPM_LSB = (Speed >> 0) & 0xff;
}

uint16_t
CProtocol::getFan_Min() const
{
  uint16_t retval;
  // Minimum speed get
  retval = Controller.MinFanRPM_MSB;
  retval <<= 8;
  retval |= Controller.MinFanRPM_LSB;
  return retval;
}

uint16_t 
CProtocol::getFan_Max() const
{
  uint16_t retval;
  // Maximum speed get
  retval = Controller.MaxFanRPM_MSB;
  retval <<= 8;
  retval |= Controller.MaxFanRPM_LSB;
  return retval;
}

uint16_t
CProtocol::getFan_Actual() const
{  
  // Rx side, actual
  uint16_t retval;
  retval = Heater.FanRPM_MSB;
  retval <<= 8;
  retval |= Heater.FanRPM_LSB;
  return retval;
}

void 
CProtocol::setFan_Actual(uint16_t Speed)  // Heater side, actual
{
  // actual speed set
  Heater.FanRPM_MSB = (Speed >> 8) & 0xff;
  Heater.FanRPM_LSB = (Speed >> 0) & 0xff;
}

float 
CProtocol::getGlowPlug_Current() const   // glow plug current
{
  uint16_t val;
  val = Heater.GlowPlugCurrent_MSB;
  val <<= 8;
  val |= Heater.GlowPlugCurrent_LSB;
  return float(val) * 0.01f;             // 10mA / digit
}

void 
CProtocol::setGlowPlug_Current(uint16_t ampsx100)    // glow plug current
{
  Heater.GlowPlugCurrent_MSB = (ampsx100 >> 8) & 0xff;
  Heater.GlowPlugCurrent_LSB = (ampsx100 >> 0) & 0xff;
}

float 
CProtocol::getGlowPlug_Voltage() const   // glow plug voltage
{
  uint16_t val;
  val = Heater.GlowPlugVoltage_MSB;
  val <<= 8;
  val |= Heater.GlowPlugVoltage_LSB;
  return float(val) * 0.1f;   // 0.1V / digit
}


void 
CProtocol::setGlowPlug_Voltage(uint16_t voltsx10)    // glow plug voltage
{
  Heater.GlowPlugVoltage_MSB = (voltsx10 >> 8) & 0xff;
  Heater.GlowPlugVoltage_LSB = (voltsx10 >> 0) & 0xff;
}

int16_t 
CProtocol::getTemperature_HeatExchg() const // temperature of heat exchanger
{
  int16_t retval;
  retval = Heater.HeatExchgTemp_MSB;
  retval <<= 8;
  retval |= Heater.HeatExchgTemp_LSB;
  return retval;
}

void
CProtocol::setTemperature_HeatExchg(uint16_t degC) // temperature of heat exchanger
{
  Heater.HeatExchgTemp_MSB = (degC >> 8) & 0xff;
  Heater.HeatExchgTemp_LSB = (degC >> 0) & 0xff;
}

float 
CProtocol::getFan_Voltage() const    // fan voltage
{
  if(getRunState()) {                // fan volatge sensing goes stupid when main heater relay turns off!
    uint16_t val;
    val = Heater.FanVoltage_MSB;
    val <<= 8;
    val |= Heater.FanVoltage_LSB;
    return float(val) * 0.1;
  }
  return 0;
}

void
CProtocol::setFan_Voltage(float volts)     // fan voltage
{
  uint16_t val = uint16_t(volts * 10);
  Heater.FanVoltage_MSB = (val >> 8) & 0xff;
  Heater.FanVoltage_LSB = (val >> 0) & 0xff;
}

void
CProtocol::setVoltage_Supply(float volts)
{
  uint16_t val = uint16_t(volts * 10);
  Heater.SupplyV_MSB = (val >> 8) & 0xff;
  Heater.SupplyV_LSB = (val >> 0) & 0xff;
}

float
CProtocol::getVoltage_SupplyRaw() const
{
  uint16_t val = 0;
  val = Heater.SupplyV_MSB & 0xff;
  val <<= 8;
  val |= Heater.SupplyV_LSB & 0xff;
  float voltage = float(val) * 0.1f;
  return voltage;
}

float
CProtocol::getVoltage_Supply() const
{
  return getVoltage_SupplyRaw() + 0.6; // compensate for series protection diode
}

void 
CProtocol::setAltitude(float altitude, bool valid)
{
  int16_t alt = (int16_t)altitude;
  Controller.Altitude_MSB = (alt >> 8) & 0xff;
  Controller.Altitude_LSB = (alt >> 0) & 0xff;
  if(valid) {
    Controller.Unknown1_MSB = 0xeb;   
    Controller.Unknown1_LSB = 0x47;   
  }
  else {
    Controller.Unknown1_MSB = 0x01;   // always 0x01
    Controller.Unknown1_LSB = 0x2c;   // always 0x2c  16bit: "300 secs = max run without burn detected" ??
  }
}

int
CProtocol::getAltitude() const 
{
  int16_t alt = (Controller.Altitude_MSB << 8) | Controller.Altitude_LSB;
  return alt;
}


void 
CProtocol::Init(int FrameMode)
{
  if(FrameMode == CtrlMode) { 
    Controller.Byte0 = 0x76;
    Controller.Len = 22;
    Controller.Command = 0;            // NOP
    setTemperature_Actual(18);  // 1degC / digit
    setHeaterDemand(20); // 1degC / digit
    setPump_Min(1.4f);          // Hz
    setPump_Max(4.3f);          // Hz
    setFan_Min(1450);           // 1RPM / digit
    setFan_Max(4500);           // 1RPM / digit
    Controller.OperatingVoltage = 120;  // 0.1V/digit
    Controller.FanSensor = 1;           // SN-1 or SN-2
    Controller.OperatingMode = 0x32;    // 0x32:Thermostat, 0xCD:Fixed
    setTemperature_Min(8);            // Minimum settable temperature
    setTemperature_Max(35);           // Maximum settable temperature
    Controller.GlowDrive = 5;         // GLOW PLUG POWER, 5 => 85W?
    Controller.Prime = 0;             // 00: normal, 0x5A: fuel prime
    Controller.Unknown1_MSB = 0x01;   // always 0x01
    Controller.Unknown1_LSB = 0x2c;   // always 0x2c  16bit: "300 secs = max run without burn detected" ??
    Controller.Altitude_MSB = 0x0d;   // basic controllers always 0x0d
    Controller.Altitude_LSB = 0xac;   // basic controllers always 0xac  16bit: "3500" 
    setCRC();
  }
  else if(FrameMode == HeatMode){
    Heater.Byte0 = 0x76;
    Heater.Len = 22;
    setRunState(0);
    setErrState(0);
    setVoltage_Supply(133);
    setFan_Actual(0);
    setFan_Voltage(0);
    setTemperature_HeatExchg(18);
    setGlowPlug_Voltage(0);
    setGlowPlug_Current(0);
    Heater.ActualPumpFreq = 0;     // fuel pump freq.: 0.1Hz / digit
    Heater.StoredErrorCode = 0;    // 
    Heater.Unknown1 = 0;           // always 0x00
    Heater.FixedPumpFreq = 23;     // fixed mode frequency set point: 0.1Hz / digit
    Heater.Unknown2 = 100;         // always 0x64  "100 ?"
    Heater.Unknown3 = 0;           // always 0x00  
    setCRC();    
  }
  else {
    memset(Data, 0, 24);
  }
}

void
CProtocol::DebugReport(const char* hdr, const char* ftr)
{
  DebugPort.print(hdr);                     // header
  for(int i=0; i<24; i++) {
    char str[16];
    sprintf(str, " %02X", Data[i]);  // build 2 dig hex values
    DebugPort.print(str);                   // and print     
  }
  DebugPort.print(ftr);                     // footer
}

void 
CProtocol::setThermostatModeProtocol(unsigned on) 
{ 
  Controller.OperatingMode = on ? 0x32 : 0xCD; 
};

void 
CProtocol::setSystemVoltage(float fVal) 
{
  int val = int(fVal * 10.);
  if(val == 120 || val == 240)
    Controller.OperatingVoltage = val;
}


int CProtocolPackage::getRunStateEx() const
{
  int runstate = getRunState();
  if(isCyclicStopStartActive()) {
    // special states for cyclic suspended
    switch(runstate) {
      case 0:  runstate = 10; break;   // standby, awaiting temperature drop
      case 7:  runstate = 11; break;   // shutting down due to cyclic trip
      case 8:  runstate = 12; break;   // cooling due to cyclic trip
    }
  }
  if(runstate == 2 && getPump_Actual() == 0) {  // split runstate 2 - glow, then fuel
    runstate = 9;
  }
  return runstate;
}	

// const char* Runstates [] PROGMEM = {
//   " Stopped/Ready ",      // 0
//   "Starting...",          // 1
//   "Igniting...",          // 2
//   "Ignition retry pause", // 3
//   "Ignited",              // 4
//   "Running",              // 5
//   "Stopping",             // 6 
//   "Shutting down",        // 7
//   "Cooling",              // 8
//   "Heating glow plug",    // 9  - interpreted state - actually runstate 2 with no pump action!
//   "Suspended",            // 10 - interpreted state - cyclic mode has suspended heater
//   "Suspending...",        // 11 - interpreted state - cyclic mode is suspending heater
//   "Suspend cooling",      // 12 - interpreted state - cyclic mode is suspending heater
//   "Unknown run state"     
// };

 

// const char* 
// CProtocolPackage::getRunStateStr() const 
// { 
//   uint8_t runstate = getRunStateEx();
//   UPPERLIMIT(runstate, 13);
//   if(runstate == 2 && getPump_Actual() == 0) {  // split runstate 2 - glow, then fuel
//     runstate = 9;
//   }
//   return Runstates[runstate]; 
// }


// const char* Errstates [] PROGMEM = {
//   "",                // [0] 
//   "",                // [1] 
//   "Low voltage",     // [2] E-01
//   "High voltage",    // [3] E-02
//   "Glow plug fault", // [4] E-03
//   "Pump fault",      // [5] E-04
//   "Overheat",        // [6] E-05
//   "Motor fault",     // [7] E-06
//   "Comms fault",     // [8] E-07
//   "Flame out",       // [9] E-08
//   "Temp sense",      // [10] E-09
//   "Ignition fail",   // [11] E-10          SmartError manufactured state - sensing runstate 2 -> >5
//   "Failed 1st ignite", // [12] E-11  SmartError manufactured state - sensing runstate 2 -> 3
//   "Excess fuel usage", // [13] E-12  SmartError manufactured state - excess fuel consumed
//   "Unknown error?"   // mystery code!
// };

// const char* ErrstatesEx [] PROGMEM = {
//   "E-00: OK",              // [0] 
//   "E-00: OK",              // [1]
//   "E-01: Low voltage",     // [2] E-01
//   "E-02: High voltage",    // [3] E-02
//   "E-03: Glow plug fault", // [4] E-03
//   "E-04: Pump fault",      // [5] E-04
//   "E-05: Overheat",        // [6] E-05
//   "E-06: Motor fault",     // [7] E-06
//   "E-07: No heater comms", // [8] E-07
//   "E-08: Flame out",       // [9] E-08
//   "E-09: Temp sense",      // [10] E-09
//   "E-10: Ignition fail",   // [11] E-10  SmartError manufactured state - sensing runstate 2 -> >5
//   "E-11: Failed 1st ignite",  // [12] E-11  SmartError manufactured state - sensing runstate 2 -> 3
//   "E-12: Excess fuel shutdown", // [13] E-12  SmartError manufactured state - excess fuel consumed
//   "Unknown error?"   // mystery code!
// };

// const char* 
// CProtocolPackage::getErrStateStr() const 
// { 
//   uint8_t errstate = getErrState();
//   UPPERLIMIT(errstate, 13);
//   return Errstates[errstate]; 
// }

// const char* 
// CProtocolPackage::getErrStateStrEx() const 
// { 
//   uint8_t errstate = getErrState();
//   UPPERLIMIT(errstate, 13);
//   return ErrstatesEx[errstate]; 
// }

/*void  
CProtocolPackage::setRefTime()
{
  _timeStamp.setRefTime();
}*/

char dbgFrameMsg[192];

void  
CProtocolPackage::reportFrames(bool isOEM, std::function<void(const char*)> pushMsg)
{
  dbgFrameMsg[0] = 0;
  _timeStamp.report(dbgFrameMsg);   // absolute time
  if(isOEM) {
    DebugReportFrame("OEM:", Controller, TERMINATE_OEM_LINE ? "\r\n" : "   ", dbgFrameMsg);
  }
  else {
    DebugReportFrame("BTC:", Controller, TERMINATE_BTC_LINE ? "\r\n" : "   ", dbgFrameMsg);
  }
  DebugReportFrame("HTR:", Heater, "\r\n", dbgFrameMsg);
  pushMsg(dbgFrameMsg);
}

int   
CProtocolPackage::getErrState() const
{ 
  static int CommsErrHoldoff = 5;

  if(getBlueWireStat() & 0x01) {
    if(CommsErrHoldoff)
      CommsErrHoldoff--;    // expire holdoff events, don't be too trigger happy on comms error
    else
      return 8;              // persistently not seeing heater data - force E-07 
  }
  else {
    CommsErrHoldoff = 5;
  }

  int smartErr = getSmartError();
  if(smartErr)
    return smartErr;
  else
    return Heater.getErrState(); 
 
}


void checkBlueWireRxEvents()
{
  // collect and process received heater data from blue wire task
  if(BlueWireCommsTask.getRxQueue( BlueWireRxData.Data) ) {
  // if(BlueWireRxQueue && xQueueReceive(BlueWireRxQueue, BlueWireRxData.Data, 0)) {
    BlueWireData.set(BlueWireRxData, BlueWireTxData);
    SmartError.monitor(BlueWireRxData);

    updateFilteredData(BlueWireRxData);

    FuelGauge.Integrate(BlueWireRxData.getPump_Actual());

    if(INBOUNDS(BlueWireRxData.getRunState(), 1, 5)) {  // check for Low Voltage Cutout
      SmartError.checkVolts(FilteredSamples.FastipVolts.getValue(), FilteredSamples.FastGlowAmps.getValue());
      SmartError.checkfuelUsage();
    }

    // trap being in state 0 with a heater error - cancel user on memory to avoid unexpected cyclic restarts
    if(RTC_Store.getUserStart() && (BlueWireRxData.getRunState() == 0) && (BlueWireRxData.getErrState() > 1)) {
      DebugPort.println("Forcing cyclic cancel due to error induced shutdown");
      // DebugPort.println("Forcing cyclic cancel due to error induced shutdown");
      RTC_Store.setUserStart(false);
    }

    pHourMeter->monitor(BlueWireRxData);
  }
}

void checkBlueWireTxEvents()
{
  // collect transmitted heater data from blue wire task
  if(BlueWireCommsTask.getTxQueue(BlueWireTxData.Data) ) {

  }
  // if(BlueWireTxQueue && xQueueReceive(BlueWireTxQueue, BlueWireTxData.Data, 0)) {
  // }
}

void updateFilteredData(CProtocol& HeaterInfo)
{
  FilteredSamples.ipVolts.update(HeaterInfo.getVoltage_Supply());
  FilteredSamples.GlowVolts.update(HeaterInfo.getGlowPlug_Voltage());
  FilteredSamples.GlowAmps.update(HeaterInfo.getGlowPlug_Current());
  FilteredSamples.Fan.update(HeaterInfo.getFan_Actual());
  FilteredSamples.FastipVolts.update(HeaterInfo.getVoltage_Supply());
  FilteredSamples.FastGlowAmps.update(HeaterInfo.getGlowPlug_Current());
}

