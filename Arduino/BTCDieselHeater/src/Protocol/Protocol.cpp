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


unsigned short 
CProtocol::CalcCRC(int len) const
{
  // calculate a CRC-16/MODBUS checksum using the first 22 bytes of the data array
  unsigned short  wCRCWord = 0xFFFF;

  int wLength = len;
  const unsigned char* pData = Data;
   while (wLength--)
   {
      unsigned char nTemp = *pData++ ^ wCRCWord;
      wCRCWord >>= 8;
      wCRCWord ^= wCRCTable[nTemp];
   }

   return wCRCWord;
}

void 
CProtocol::setCRC()
{
  setCRC(CalcCRC(22));
}

void 
CProtocol::setCRC(unsigned short CRC)
{
  Data[22] = (CRC >> 8) & 0xff;   // MSB of CRC in Data[22]
  Data[23] = (CRC >> 0) & 0xff;   // LSB of CRC in Data[23]
}


unsigned short
CProtocol::getCRC() const
{
  unsigned short CRC;
  CRC = Data[22];   // MSB of CRC in Data[22]
  CRC <<= 8;
  CRC |= Data[23];  // LSB of CRC in Data[23]
  return CRC;
}

// return true for CRC match
bool
CProtocol::verifyCRC(bool bSilent) const
{
  unsigned short CRC = CalcCRC(22);  // calculate CRC based on first 22 bytes
  unsigned short FrameCRC = getCRC();
  bool bOK = (FrameCRC == CRC);
  if(!bOK && !bSilent) {
    DebugPort.printf("verifyCRC FAILED: calc: %04X data: %04X\r\n", CRC, FrameCRC);
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
CProtocol::setFan_Min(short Speed) 
{
  // Minimum speed set
  Controller.MinFanRPM_MSB = (Speed >> 8) & 0xff;
  Controller.MinFanRPM_LSB = (Speed >> 0) & 0xff;
}

void 
CProtocol::setFan_Max(short Speed) 
{
  // Minimum speed set
  Controller.MaxFanRPM_MSB = (Speed >> 8) & 0xff;
  Controller.MaxFanRPM_LSB = (Speed >> 0) & 0xff;
}

short 
CProtocol::getFan_Min() const
{
  short retval;
  // Minimum speed get
  retval = Controller.MinFanRPM_MSB;
  retval <<= 8;
  retval |= Controller.MinFanRPM_LSB;
  return retval;
}

short 
CProtocol::getFan_Max() const
{
  short retval;
  // Maximum speed get
  retval = Controller.MaxFanRPM_MSB;
  retval <<= 8;
  retval |= Controller.MaxFanRPM_LSB;
  return retval;
}

short
CProtocol::getFan_Actual() const
{  
  // Rx side, actual
  short retval;
  retval = Heater.FanRPM_MSB;
  retval <<= 8;
  retval |= Heater.FanRPM_LSB;
  return retval;
}

void 
CProtocol::setFan_Actual(short Speed)  // Heater side, actual
{
  // actual speed set
  Heater.FanRPM_MSB = (Speed >> 8) & 0xff;
  Heater.FanRPM_LSB = (Speed >> 0) & 0xff;
}

float 
CProtocol::getGlowPlug_Current() const   // glow plug current
{
  short val;
  val = Heater.GlowPlugCurrent_MSB;
  val <<= 8;
  val |= Heater.GlowPlugCurrent_LSB;
  return float(val) * 0.01f;             // 10mA / digit
}

void 
CProtocol::setGlowPlug_Current(short ampsx100)    // glow plug current
{
  Heater.GlowPlugCurrent_MSB = (ampsx100 >> 8) & 0xff;
  Heater.GlowPlugCurrent_LSB = (ampsx100 >> 0) & 0xff;
}

float 
CProtocol::getGlowPlug_Voltage() const   // glow plug voltage
{
  short val;
  val = Heater.GlowPlugVoltage_MSB;
  val <<= 8;
  val |= Heater.GlowPlugVoltage_LSB;
  return float(val) * 0.1f;   // 0.1V / digit
}


void 
CProtocol::setGlowPlug_Voltage(short voltsx10)    // glow plug voltage
{
  Heater.GlowPlugVoltage_MSB = (voltsx10 >> 8) & 0xff;
  Heater.GlowPlugVoltage_LSB = (voltsx10 >> 0) & 0xff;
}

short 
CProtocol::getTemperature_HeatExchg() const // temperature of heat exchanger
{
  short retval;
  retval = Heater.HeatExchgTemp_MSB;
  retval <<= 8;
  retval |= Heater.HeatExchgTemp_LSB;
  return retval;
}

void
CProtocol::setTemperature_HeatExchg(short degC) // temperature of heat exchanger
{
  Heater.HeatExchgTemp_MSB = (degC >> 8) & 0xff;
  Heater.HeatExchgTemp_LSB = (degC >> 0) & 0xff;
}

float 
CProtocol::getFan_Voltage() const    // fan voltage
{
  if(getRunState()) {                // fan volatge sensing goes stupid when main heater relay turns off!
    short val;
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
  short val = short(volts * 10);
  Heater.FanVoltage_MSB = (val >> 8) & 0xff;
  Heater.FanVoltage_LSB = (val >> 0) & 0xff;
}

void
CProtocol::setVoltage_Supply(float volts)
{
  short val = short(volts * 10);
  Heater.SupplyV_MSB = (val >> 8) & 0xff;
  Heater.SupplyV_LSB = (val >> 0) & 0xff;
}

float
CProtocol::getVoltage_SupplyRaw() const
{
  short val = 0;
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
    Controller.Unknown2_MSB = 0x0d;   // always 0x0d
    Controller.Unknown2_LSB = 0xac;   // always 0xac  16bit: "3500" ??  Ignition fan max RPM????
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
  if(isCyclicActive()) {
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

const char* Runstates [] PROGMEM = {
  " Stopped/Ready ",      // 0
  "Starting...",          // 1
  "Igniting...",          // 2
  "Ignition retry pause", // 3
  "Ignited",              // 4
  "Running",              // 5
  "Stopping",             // 6 
  "Shutting down",        // 7
  "Cooling",              // 8
  "Heating glow plug",    // 9  - interpreted state - actually runstate 2 with no pump action!
  "Suspended",            // 10 - interpreted state - cyclic mode has suspended heater
  "Suspending...",        // 11 - interpreted state - cyclic mode is suspending heater
  "Suspend cooling",      // 12 - interpreted state - cyclic mode is suspending heater
  "Unknown run state"     
};

 

const char* 
CProtocolPackage::getRunStateStr() const 
{ 
  uint8_t runstate = getRunStateEx();
  UPPERLIMIT(runstate, 13);
  if(runstate == 2 && getPump_Actual() == 0) {  // split runstate 2 - glow, then fuel
    runstate = 9;
  }
  return Runstates[runstate]; 
}


const char* Errstates [] PROGMEM = {
  "",
  "",
  "Low voltage",     // E-01
  "High voltage",    // E-02
  "Glow plug fault", // E-03
  "Pump fault",      // E-04
  "Overheat",        // E-05
  "Motor fault",     // E-06
  "Comms fault",     // E-07
  "Flame out",       // E-08
  "Temp sense",      // E-09
  "Ignition fail",   // E-10          SmartError manufactured state - sensing runstate 2 -> >5
  "Failed 1st ignition attempt",  // E-11  SmartError manufactured state - sensing runstate 2 -> 3
  "Unknown error?"   // mystery code!
};

const char* ErrstatesEx [] PROGMEM = {
  "E-00: OK",
  "E-00: OK",
  "E-01: Low voltage",     // E-01
  "E-02: High voltage",    // E-02
  "E-03: Glow plug fault", // E-03
  "E-04: Pump fault",      // E-04
  "E-05: Overheat",        // E-05
  "E-06: Motor fault",     // E-06
  "E-07: No heater comms", // E-07
  "E-08: Flame out",       // E-08
  "E-09: Temp sense",      // E-09
  "E-10: Ignition fail",   // E-10  SmartError manufactured state - sensing runstate 2 -> >5
  "E-11: Failed 1st ignition attempt",  // E-11  SmartError manufactured state - sensing runstate 2 -> 3
  "Unknown error?"   // mystery code!
};

const char* 
CProtocolPackage::getErrStateStr() const 
{ 
  uint8_t errstate = getErrState();
  UPPERLIMIT(errstate, 13);
  return Errstates[errstate]; 
}

const char* 
CProtocolPackage::getErrStateStrEx() const 
{ 
  uint8_t errstate = getErrState();
  UPPERLIMIT(errstate, 13);
  return ErrstatesEx[errstate]; 
}

/*void  
CProtocolPackage::setRefTime()
{
  _timeStamp.setRefTime();
}*/

void  
CProtocolPackage::reportFrames(bool isOEM)
{
  _timeStamp.report();   // absolute time
  if(isOEM) {
    DebugReportFrame("OEM:", Controller, TERMINATE_OEM_LINE ? "\r\n" : "   ");
  }
  else {
    DebugReportFrame("BTC:", Controller, TERMINATE_BTC_LINE ? "\r\n" : "   ");
  }
  DebugReportFrame("HTR:", Heater, "\r\n");
}

int   
CProtocolPackage::getErrState() const 
{ 
  if(getBlueWireStat() & 0x01) {
    return 8; // force E-07 - we're not seeing heater data
  }
  else {
    int smartErr = getSmartError();
    if(smartErr)
      return smartErr;
    else
      return Heater.getErrState(); 
  }
}

