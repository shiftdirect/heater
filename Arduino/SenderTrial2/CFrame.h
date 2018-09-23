#ifndef _CFRAME_H_
#define _CFRAME_H_

class CFrame {
public:
  union {
    unsigned char Data[24];
    struct {
      unsigned char Byte0;              //  [0] always 0x76
      unsigned char Len;                //  [1] always 0x16 == 22
      unsigned char Command;            //  [2] transient commands: 00: NOP, 0xa0 START, 0x05: STOP
      unsigned char ActualTemperature;  //  [3] 1degC / digit
      unsigned char DesiredTemperature; //  [4] 1degC / digit
      unsigned char MinPumpFreq;        //  [5] 0.1Hz/digit
      unsigned char MaxPumpFreq;        //  [6] 0.1Hz/digit
      unsigned char MinFanRPM_MSB;      //  [7] 16 bit - big endian MSB
      unsigned char MinFanRPM_LSB;      //  [8] 16 bit - big endian LSB : 1 RPM / digit
      unsigned char MaxFanRPM_MSB;      //  [9] 16 bit - big endian MSB
      unsigned char MaxFanRPM_LSB;      // [10] 16 bit - big endian LSB : 1 RPM / digit
      unsigned char OperatingVoltage;   // [11] 120, 240 : 0.1V/digit
      unsigned char FanSensor;          // [12] SN-1 or SN-2
      unsigned char OperatingMode;      // [13] 0x32:Thermostat, 0xCD:Fixed
      unsigned char MinTemperature;     // [14] Minimum settable temperature
      unsigned char MaxTemperature;     // [15] Maximum settable temperature
      unsigned char MinTempRise;        // [16] temp rise to sense running OK
      unsigned char Prime;              // [17] 00: normal, 0x5A: fuel prime
      unsigned char Unknown1_MSB;       // [18] always 0x01
      unsigned char Unknown1_LSB;       // [19] always 0x2c  "300 secs = max run without burn detected"?
      unsigned char Unknown2_MSB;       // [20] always 0x0d
      unsigned char Unknown2_LSB;       // [21] always 0xac  "3500 ?"
      unsigned char CRC_MSB;            // [22]
      unsigned char CRC_LSB;            // [23]
    } Tx;
    struct {
      unsigned char Byte0;              // always 0x76
      unsigned char Len;                // always 0x16 == 22
      unsigned char RunState;           // operating state
      unsigned char OnOff;              // 0: OFF, 1: ON
      unsigned char SupplyV_MSB;        // 16 bit - big endian MSB
      unsigned char SupplyV_LSB;        // 16 bit - big endian MSB : 0.1V / digit
      unsigned char FanRPM_MSB;         // 16 bit - big endian MSB
      unsigned char FanRPM_LSB;         // 16 bit - big endian LSB : 1 RPM / digit
      unsigned char InletTemp_MSB;      // 16 bit - big endian MSB  
      unsigned char InletTemp_LSB;      // 16 bit - big endian LSB : 1 degC / digit
      unsigned char HeatExchgTemp_MSB;  // 16 bit - big endian MSB  
      unsigned char HeatExchgTemp_LSB;  // 16 bit - big endian LSB : 1 degC / digit
      unsigned char GlowPinPWMDuty_MSB; // 16 bit - big endian MSB  
      unsigned char GlowPinPWMDuty_LSB; // 16 bit - big endian LSB : 1% / digit
      unsigned char GlowPinTemp_MSB;    // 16 bit - big endian MSB  
      unsigned char GlowPinTemp_LSB;    // 16 bit - big endian LSB : 1 degC / digit
      unsigned char ActualPumpFreq;     // fuel pump freq.: 0.1Hz / digit
      unsigned char ErrorCode;          // 
      unsigned char Unknown1;           // always 0x00
      unsigned char FixedPumpFreq;      // fixed mode frequency set point: 0.1Hz / digit
      unsigned char Unknown2;           // always 0x64  "100 ?"
      unsigned char Unknown3;           // always 0x00  
      unsigned char CRC_MSB;
      unsigned char CRC_LSB;
    } Rx;
  };
  static const int TxMode = 1;
  const unsigned short wCRCTable[256] = {
    0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
    0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
    0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
    0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
    0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
    0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
    0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
    0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
    0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
    0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
    0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
    0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
    0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
    0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
    0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
    0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
    0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
    0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
    0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
    0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
    0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
    0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
    0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
    0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
    0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
    0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
    0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
    0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
    0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
    0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
    0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
    0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040 
  };

public:
  CFrame() { Init(0); };
  CFrame(int TxMode) { Init(TxMode); };
  void Init(int Txmode);
  // CRC handlers
  unsigned short CalcCRC(int len);  // calculate and set the CRC upon first 22 bytes
  void setCRC();             // calculate and set the CRC in the buffer
  void setCRC(unsigned short CRC);  // set  the CRC in the buffer
  unsigned short getCRC();   // extract CRC value from buffer
  bool verifyCRC();          // return true for CRC match
  // fan set/get
  short getFan_Actual();  // Rx side, actual
  short getFan_Min();  // Tx side, define min fan speed
  short getFan_Max();  // Tx side, define max fan speed
  void setFan_Min(short speed); // Tx side, define min fan speed
  void setFan_Max(short speed); // Tx side, define max fan speed
  // pump set/get
  void setPump_Min(unsigned short Freq) {   Tx.MinPumpFreq = Freq; };
  void setPump_Max(unsigned short Freq) {   Tx.MaxPumpFreq = Freq; };
  unsigned char getPump_Min() { return Tx.MinPumpFreq; };   // Tx side, min pump freq
  unsigned char getPump_Max() { return Tx.MaxPumpFreq; };   // Tx side, max pump freq
  unsigned char getPump_Actual() { return Rx.ActualPumpFreq; };  // Rx style, actual
  unsigned char getPump_Fixed() { return Rx.FixedPumpFreq; };   // Fixed mode pump frequency
  // temperature set/get
  void setTemperature_Desired(unsigned char degC) { Tx.DesiredTemperature = degC; };
  void setTemperature_Min(unsigned char degC) { Tx.MinTemperature = degC; };
  void setTemperature_Max(unsigned char degC) { Tx.MaxTemperature = degC; };
  void setTemperature_Actual(unsigned char degC) { Tx.ActualTemperature = degC; };
  unsigned char getTemperature_Desired() { return Tx.DesiredTemperature; };
  short getTemperature_GlowPin();   // temperature of glow pin
  short getTemperature_HeatExchg(); // temperature of heat exchanger
  short getTemperature_Inlet();     // temperature near inlet

  CFrame& operator=(CFrame& rhs);
};

#endif