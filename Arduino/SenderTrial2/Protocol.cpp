#include <Arduino.h>
#include "Protocol.h"
#include "debugport.h"
 
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
CProtocol::verifyCRC() const
{
  unsigned short CRC = CalcCRC(22);  // calculate CRC based on first 22 bytes
  unsigned short FrameCRC = getCRC();
  bool bOK = (FrameCRC == CRC);
  if(!bOK) {
    DebugPort.print("verifyCRC FAILED: calc:");
    DebugPort.print(CRC, HEX);
    DebugPort.print(" data:");
    DebugPort.println(FrameCRC, HEX);
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
CProtocol::getFan_Min() 
{
  short retval;
  // Minimum speed get
  retval = Controller.MinFanRPM_MSB;
  retval <<= 8;
  retval |= Controller.MinFanRPM_LSB;
  return retval;
}

short 
CProtocol::getFan_Max() 
{
  short retval;
  // Maximum speed get
  retval = Controller.MaxFanRPM_MSB;
  retval <<= 8;
  retval |= Controller.MaxFanRPM_LSB;
  return retval;
}

short
CProtocol::getFan_Actual() 
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

short 
CProtocol::getGlowPlug_Current()    // glow plug current
{
  short retval;
  retval = Heater.GlowPlugCurrent_MSB;
  retval <<= 8;
  retval |= Heater.GlowPlugCurrent_LSB;
  return retval;
}

void 
CProtocol::setGlowPlug_Current(short ampsx100)    // glow plug current
{
  Heater.GlowPlugCurrent_MSB = (ampsx100 >> 8) & 0xff;
  Heater.GlowPlugCurrent_LSB = (ampsx100 >> 0) & 0xff;
}

short 
CProtocol::getGlowPlug_Voltage()    // glow plug voltage
{
  short retval;
  retval = Heater.GlowPlugVoltage_MSB;
  retval <<= 8;
  retval |= Heater.GlowPlugVoltage_LSB;
  return retval;
}


void 
CProtocol::setGlowPlug_Voltage(short voltsx10)    // glow plug voltage
{
  Heater.GlowPlugVoltage_MSB = (voltsx10 >> 8) & 0xff;
  Heater.GlowPlugVoltage_LSB = (voltsx10 >> 0) & 0xff;
}

short 
CProtocol::getTemperature_HeatExchg() // temperature of heat exchanger
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

short 
CProtocol::getFan_Voltage()     // temperature near inlet
{
  short retval;
  retval = Heater.FanVoltage_MSB;
  retval <<= 8;
  retval |= Heater.FanVoltage_LSB;
  return retval;
}

void
CProtocol::setFan_Voltage(short voltsx10)     // temperature near inlet
{
  Heater.FanVoltage_MSB = (voltsx10 >> 8) & 0xff;
  Heater.FanVoltage_LSB = (voltsx10 >> 0) & 0xff;
}

void
CProtocol::setVoltage_Supply(short voltsx10)
{
  Heater.SupplyV_MSB = (voltsx10 >> 8) & 0xff;
  Heater.SupplyV_LSB = (voltsx10 >> 0) & 0xff;
}

short
CProtocol::getVoltage_Supply()
{
  short retval = 0;
  retval = Heater.SupplyV_MSB & 0xff;
  retval <<= 8;
  retval |= Heater.SupplyV_LSB & 0xff;
}

void 
CProtocol::Init(int FrameMode)
{
  if(FrameMode == CtrlMode) { 
    Controller.Byte0 = 0x76;
    Controller.Len = 22;
    Controller.Command = 0;            // NOP
    setTemperature_Actual(18);  // 1degC / digit
    setTemperature_Desired(20); // 1degC / digit
    setPump_Min(14);            // 0.1Hz/digit
    setPump_Max(43);            // 0.1Hz/digit
    setFan_Min(1450);           // 1RPM / digit
    setFan_Max(4500);           // 1RPM / digit
    Controller.OperatingVoltage = 120;  // 0.1V/digit
    Controller.FanSensor = 1;           // SN-1 or SN-2
    Controller.OperatingMode = 0x32;    // 0x32:Thermostat, 0xCD:Fixed
    setTemperature_Min(8);      // Minimum settable temperature
    setTemperature_Max(35);     // Maximum settable temperature
    Controller.MinTempRise = 5;         // temp rise to sense fuel ignition
    Controller.Prime = 0;               // 00: normal, 0x5A: fuel prime
    Controller.Unknown1_MSB = 0x01;     // always 0x01
    Controller.Unknown1_LSB = 0x2c;     // always 0x2c  16bit: "300 secs = max run without burn detected" ??
    Controller.Unknown2_MSB = 0x0d;     // always 0x0d
    Controller.Unknown2_LSB = 0xac;     // always 0xac  16bit: "3500" ??
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
    Heater.ErrorCode = 0;          // 
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

