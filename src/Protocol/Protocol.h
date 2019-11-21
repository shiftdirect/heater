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

#ifndef _CPROTOCOL_H_
#define _CPROTOCOL_H_

#include "../Utility/UtilClasses.h"
#include "../Utility/MODBUS-CRC16.h"

class CProtocol {
  // CModBusCRC16 _CRCengine;
public:
  union {
    uint8_t Data[24];
    struct {
      uint8_t Byte0;              //  [0] always 0x76
      uint8_t Len;                //  [1] always 0x16 == 22
      uint8_t Command;            //  [2] transient commands: 00: NOP, 0xa0 START, 0x05: STOP
      int8_t  ActualTemperature;  //  [3] 1deg6C / digit
      int8_t  DesiredDemand;      //  [4] typ. 1degC / digit, but also gets used for Fixed Hx demand too!
      uint8_t MinPumpFreq;        //  [5] 0.1Hz/digit
      uint8_t MaxPumpFreq;        //  [6] 0.1Hz/digit
      uint8_t MinFanRPM_MSB;      //  [7] 16 bit - big endian MSB
      uint8_t MinFanRPM_LSB;      //  [8] 16 bit - big endian LSB : 1 RPM / digit
      uint8_t MaxFanRPM_MSB;      //  [9] 16 bit - big endian MSB
      uint8_t MaxFanRPM_LSB;      // [10] 16 bit - big endian LSB : 1 RPM / digit
      uint8_t OperatingVoltage;   // [11] 120, 240 : 0.1V/digit
      uint8_t FanSensor;          // [12] SN-1 or SN-2
      uint8_t OperatingMode;      // [13] 0x32:Thermostat, 0xCD:Fixed
      int8_t  MinTemperature;     // [14] Minimum settable temperature
      int8_t  MaxTemperature;     // [15] Maximum settable temperature
      uint8_t GlowDrive;          // [16] power to supply to glow plug
      uint8_t Prime;              // [17] 00: normal, 0x5A: fuel prime
      uint8_t Unknown1_MSB;       // [18] always 0x01
      uint8_t Unknown1_LSB;       // [19] always 0x2c  "300 secs = max run without burn detected"?
      uint8_t Unknown2_MSB;       // [20] always 0x0d
      uint8_t Unknown2_LSB;       // [21] always 0xac  "3500 ?"
      uint8_t CRC_MSB;            // [22]
      uint8_t CRC_LSB;            // [23]
    } Controller;
    struct {
      uint8_t Byte0;              // always 0x76
      uint8_t Len;                // always 0x16 == 22 bytes
      uint8_t RunState;           // operating state
      uint8_t ErrState;           // 0: OFF, 1: ON, 2+ (E-0n + 1)
      uint8_t SupplyV_MSB;        // 16 bit - big endian MSB
      uint8_t SupplyV_LSB;        // 16 bit - big endian MSB : 0.1V / digit
      uint8_t FanRPM_MSB;         // 16 bit - big endian MSB
      uint8_t FanRPM_LSB;         // 16 bit - big endian LSB : 1 RPM / digit
      uint8_t FanVoltage_MSB;     // 16 bit - big endian MSB  
      uint8_t FanVoltage_LSB;     // 16 bit - big endian LSB : 0.1V / digit
      uint8_t HeatExchgTemp_MSB;  // 16 bit - big endian MSB  
      uint8_t HeatExchgTemp_LSB;  // 16 bit - big endian LSB : 1 degC / digit
      uint8_t GlowPlugVoltage_MSB;   // 16 bit - big endian MSB  
      uint8_t GlowPlugVoltage_LSB;   // 16 bit - big endian LSB : 0.1V / digit
      uint8_t GlowPlugCurrent_MSB; // 16 bit - big endian MSB  
      uint8_t GlowPlugCurrent_LSB; // 16 bit - big endian LSB : 10mA / digit
      uint8_t ActualPumpFreq;     // fuel pump freq.: 0.1Hz / digit
      uint8_t StoredErrorCode;    // 
      uint8_t Unknown1;           // always 0x00
      uint8_t FixedPumpFreq;      // fixed mode frequency set point: 0.1Hz / digit
      uint8_t Unknown2;           // always 0x64  "100 ?"
      uint8_t Unknown3;           // always 0x00  
      uint8_t CRC_MSB;
      uint8_t CRC_LSB;
    } Heater;
  };
  static const int CtrlMode = 1;
  static const int HeatMode = 2;

public:
  CProtocol() { Init(0); };
  CProtocol(int TxMode) { Init(TxMode); };

  void Init(int Txmode);
  // CRC handlers
  void setCRC();                    // calculate and set the CRC in the buffer
  void setCRC(uint16_t CRC);  // set  the CRC in the buffer
  uint16_t getCRC() const;    // extract CRC value from buffer
  bool verifyCRC(bool silent=false) const;   // return true for CRC match

  void setActiveMode() { Controller.Byte0 = 0x76; };  // this allows heater to save tuning params to EEPROM
  void setPassiveMode() { Controller.Byte0 = 0x78; };  // this prevents heater saving tuning params to EEPROM
  // command helpers
  void resetCommand() { setRawCommand(0x00); };
  void onCommand() { setRawCommand(0xA0); };
  void offCommand() { setRawCommand(0x05); };
  // raw command
  int getRawCommand() const { return Controller.Command; };
  void setRawCommand(int mode) { Controller.Command = mode; };
  // Run state
  uint8_t getRunState() const { return Heater.RunState; };
  void setRunState(uint8_t state) { Heater.RunState = state; };
  uint8_t getErrState() const { return Heater.ErrState; };
  void setErrState(uint8_t state) { Heater.ErrState = state; };
  uint8_t getStoredErrCode() const { return Heater.StoredErrorCode; };
  void setStoredErrCode(uint8_t state) { Heater.StoredErrorCode = state; };
  //
  float getVoltage_Supply() const;
  float getVoltage_SupplyRaw() const;
  void setVoltage_Supply(float volts);
  float getSystemVoltage() const { return float(Controller.OperatingVoltage) * 0.1; };
  void  setSystemVoltage(float val);
  
  // fan set/get
  uint16_t getFan_Actual() const;  // Heater side, actual
  uint16_t getFan_Min() const;  // Controller side, define min fan speed
  uint16_t getFan_Max() const;  // Controller side, define max fan speed
  void setFan_Actual(uint16_t speed);  // Heater side, actual
  void setFan_Min(uint16_t speed); // Controller side, define min fan speed
  void setFan_Max(uint16_t speed); // Controller side, define max fan speed
  float getFan_Voltage() const;      // fan voltage
  void setFan_Voltage(float volts);  // fan voltage
  
  // pump set/get
  void setPump_Min(float Freq) {   Controller.MinPumpFreq = (uint8_t)(Freq * 10.f + 0.5f); };
  void setPump_Max(float Freq) {   Controller.MaxPumpFreq = (uint8_t)(Freq * 10.f + 0.5f); };
  void setPump_Actual(float Freq) { Heater.ActualPumpFreq = (uint8_t)(Freq * 10.f + 0.5f); }; 
  void setPump_Fixed(float Freq) { Heater.FixedPumpFreq = (uint8_t)(Freq * 10.f + 0.5f); };  
  float getPump_Min() const { return float(Controller.MinPumpFreq) * 0.1f; };   // Tx side, min pump freq
  float getPump_Max() const { return float(Controller.MaxPumpFreq) * 0.1f; };   // Tx side, max pump freq
  float getPump_Actual() const { return float(Heater.ActualPumpFreq) * 0.1f; };  // Rx style, actual
  float getPump_Fixed() const { return float(Heater.FixedPumpFreq) * 0.1f; };   // Fixed mode pump frequency
  void setPump_Prime(bool on) { Controller.Prime = on ? 0x5A : 0; };
  // temperature set/get
  void setHeaterDemand(int8_t degC) { Controller.DesiredDemand = degC; };
  void setTemperature_Min(int8_t degC) { Controller.MinTemperature = degC; };
  void setTemperature_Max(int8_t degC) { Controller.MaxTemperature = degC; };
  void setTemperature_Actual(int8_t degC) { Controller.ActualTemperature = degC; };
  int8_t getHeaterDemand() const { return Controller.DesiredDemand; };
  int8_t getTemperature_Min() const { return Controller.MinTemperature; };
  int8_t getTemperature_Max() const { return Controller.MaxTemperature; };
  int8_t getTemperature_Actual() const { return Controller.ActualTemperature; };
  void setThermostatModeProtocol(unsigned on);
  bool isThermostat() const { return Controller.OperatingMode == 0x32; };
  // glow plug
  float getGlowPlug_Current() const;   // glow plug current
  float getGlowPlug_Voltage() const;   // glow plug voltage
  void setGlowPlug_Current(uint16_t ampsx100);   // glow plug current
  void setGlowPlug_Voltage(uint16_t voltsx10);   // glow plug voltage
  void setGlowDrive(uint8_t val) { Controller.GlowDrive = val; };
  uint8_t getGlowDrive() const { return Controller.GlowDrive; };
  // heat exchanger
  int16_t getTemperature_HeatExchg() const; // temperature of heat exchanger
  void setTemperature_HeatExchg(uint16_t degC); // temperature of heat exchanger

  void DebugReport(const char* hdr, const char* ftr);

  CProtocol& operator=(const CProtocol& rhs);
};

class CModeratedFrame : public CProtocol {
  unsigned long lastTime;
public:
  CModeratedFrame() { lastTime = 0; };
  void setTime() { lastTime = millis(); };
  long elapsedTime() { return millis() - lastTime; };
};


class CProtocolPackage {
  CProtocol Heater;
  CProtocol Controller;
  CContextTimeStamp _timeStamp;
public:
  void  set(const CProtocol& htr, const CProtocol& ctl) { Heater = htr; Controller = ctl; };
  int   getRunState() const { return Heater.getRunState(); };
  int   getRunStateEx() const; // extra support for cyclic thermostat mode
  const char* getRunStateStr() const;
  int   getErrState() const;
  const char* getErrStateStr() const;
  const char* getErrStateStrEx() const;
  float getBattVoltage() const { return Heater.getVoltage_Supply(); };
  bool  isThermostat() const { return Controller.isThermostat(); };
  float getHeaterDemand() const { return float(Controller.getHeaterDemand()); };
  float getTemperature_HeatExchg() const { return float(Heater.getTemperature_HeatExchg()); };
  float getTemperature_Min() const { return float(Controller.getTemperature_Min()); };
  float getTemperature_Max() const { return float(Controller.getTemperature_Max()); };
  float getPump_Fixed() const { return Heater.getPump_Fixed(); };
  float getPump_Actual() const { return Heater.getPump_Actual(); };
  float getPump_Min() const { return Controller.getPump_Min(); };
  float getPump_Max() const { return Controller.getPump_Max(); };
  float getFan_Actual() const { return Heater.getFan_Actual(); };
  uint16_t getFan_Min() const { return Controller.getFan_Min(); };
  uint16_t getFan_Max() const { return Controller.getFan_Max(); };
  float getFan_Voltage() const { return Heater.getFan_Voltage(); };
  int   getFan_Sensor() const { return Controller.Controller.FanSensor; };
  float getGlowPlug_Power() const { return Heater.getGlowPlug_Current() * Heater.getGlowPlug_Voltage(); };
  float getGlow_Voltage() const { return Heater.getGlowPlug_Voltage(); };
  float getGlow_Current() const { return Heater.getGlowPlug_Current(); };
  float getSystemVoltage() const { return Controller.getSystemVoltage(); };
  int   getGlow_Drive() const { return Controller.getGlowDrive(); };

//  void  setRefTime();
  void  reportFrames(bool isOEM);
};

extern const CProtocolPackage& getHeaterInfo();

#endif