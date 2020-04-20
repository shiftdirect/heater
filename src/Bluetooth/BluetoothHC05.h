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


#include "BluetoothAbstract.h"
#include "../Utility/Moderator.h"

// Define the serial port for access to a HC-05 module.
// This is generally Serial2, but different platforms use 
// a different class for the implementation.
#ifdef __arm__
// for Arduino Due
static UARTClass& HC05_SerialPort(Serial2);      
#else
// for Mega, ESP32
static HardwareSerial& HC05_SerialPort(Serial2); 
#endif

// define a derived class that offers bluetooth messaging over the HC-05

class CBluetoothHC05 : public CBluetoothAbstract {
  bool ATCommand(const char* str);
  bool ATResponse(const char* str, const char* respHdr, char* response, int& len);
  int _sensePin, _keyPin;
  CModerator foldbackModerator;
  char _MAC[32];
  bool _bTest;
  bool _bGotMAC;
  int _BTbaudIdx;
public:
  CBluetoothHC05(int keyPin, int sensePin);
  void begin();
  void send(const char* Str);
  void check();
  virtual bool isConnected();
  const char* getMAC();
  virtual bool test(char);   // returns true whilst test mode is active
protected:
  virtual void _openSerial(int baudrate);
  virtual void _foldbackDesiredTemp();
  void _flush();
  void _decodeMACresponse(char* pResponse, int len);
  void _setCommandMode(bool commandMode);
};