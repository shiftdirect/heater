#include <Arduino.h>
#include "CFrame.h"

unsigned short 
CFrame::CalcCRC() 
{
  // calculate a CRC-16/MODBUS checksum using the first 22 bytes of the data array
  unsigned short  wCRCWord = 0xFFFF;

  int wLength = 22;
  unsigned char* pData = Data;
   while (wLength--)
   {
      unsigned char nTemp = *pData++ ^ wCRCWord;
      wCRCWord >>= 8;
      wCRCWord ^= wCRCTable[nTemp];
   }

   return wCRCWord;
}

void 
CFrame::setCRC()
{
  setCRC(CalcCRC());
}

void 
CFrame::setCRC(unsigned short CRC)
{
  Data[22] = (CRC >> 8) & 0xff;   // MSB of CRC in Data[22]
  Data[23] = (CRC >> 0) & 0xff;   // LSB of CRC in Data[23]
}

unsigned short
CFrame::getCRC()
{
  unsigned short CRC;
  CRC = Data[22];   // MSB of CRC in Data[22]
  CRC <<= 8;
  CRC |= Data[23];  // LSB of CRC in Data[23]
}

// return true for CRC match
bool
CFrame::verifyCRC()
{
  unsigned short CRC = CalcCRC();  // calculate CRC based on first 22 bytes
  return (getCRC() == CRC);        // does it match the stored values?
}

void 
CFrame::setFan_Min(short Speed) 
{
  // Minimum speed set
  Tx.MinFanRPM_MSB = (Speed >> 8) & 0xff;
  Tx.MinFanRPM_LSB = (Speed >> 0) & 0xff;
}

void 
CFrame::setFan_Max(short Speed) 
{
  // Minimum speed set
  Tx.MaxFanRPM_MSB = (Speed >> 8) & 0xff;
  Tx.MaxFanRPM_LSB = (Speed >> 0) & 0xff;
}

short 
CFrame::getFan_Min() 
{
  short retval;
  // Minimum speed get
  retval = Tx.MinFanRPM_MSB;
  retval <<= 8;
  retval |= Tx.MinFanRPM_LSB;
  return retval;
}

short 
CFrame::getFan_Max() 
{
  short retval;
  // Maximum speed get
  retval = Tx.MaxFanRPM_MSB;
  retval <<= 8;
  retval |= Tx.MaxFanRPM_LSB;
  return retval;
}

short
CFrame::getFan_Actual() 
{  
  // Rx side, actual
  short retval;
  retval = Rx.FanRPM_MSB;
  retval <<= 8;
  retval |= Rx.FanRPM_LSB;
  return retval;
}

short 
CFrame::getTemperature_GlowPin()    // temperature of glow pin
{
  short retval;
  retval = Rx.GlowPinTemp_MSB;
  retval <<= 8;
  retval |= Rx.GlowPinPWMDuty_LSB;
  return retval;
}

short 
CFrame::getTemperature_HeatExchg() // temperature of heat exchanger
{
  short retval;
  retval = Rx.HeatExchgTemp_MSB;
  retval <<= 8;
  retval |= Rx.HeatExchgTemp_LSB;
  return retval;
}

short 
CFrame::getTemperature_Inlet()     // temperature near inlet
{
  short retval;
  retval = Rx.InletTemp_MSB;
  retval <<= 8;
  retval |= Rx.InletTemp_LSB;
  return retval;
}

void 
CFrame::Init(int Txmode)
{
  if(Txmode) { 
    Tx.Byte0 = 0x76;
    Tx.Len = 22;
    Tx.Command = 0;            // NOP
    setTemperature_Actual(18);  // 1degC / digit
    setTemperature_Desired(20); // 1degC / digit
    setPump_Min(14);            // 0.1Hz/digit
    setPump_Max(43);            // 0.1Hz/digit
    setFan_Min(1450);           // 1RPM / digit
    setFan_Max(4500);           // 1RPM / digit
    Tx.OperatingVoltage = 120;  // 0.1V/digit
    Tx.FanSensor = 1;           // SN-1 or SN-2
    Tx.OperatingMode = 0x32;    // 0x32:Thermostat, 0xCD:Fixed
    setTemperature_Min(8);      // Minimum settable temperature
    setTemperature_Max(35);     // Maximum settable temperature
    Tx.MinTempRise = 5;         // temp rise to sense fuel ignition
    Tx.Prime = 0;               // 00: normal, 0x5A: fuel prime
    Tx.Unknown1_MSB = 0x01;     // always 0x01
    Tx.Unknown1_LSB = 0x2c;     // always 0x2c  16bit: "300 secs = max run without burn detected" ??
    Tx.Unknown2_MSB = 0x0d;     // always 0x0d
    Tx.Unknown2_LSB = 0xac;     // always 0xac  16bit: "3500" ??
    setCRC();
  }
  else {
    memset(Data, 0, 24);
  }
}

