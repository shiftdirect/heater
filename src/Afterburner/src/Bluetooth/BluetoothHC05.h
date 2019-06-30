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
  int _sensePin, _keyPin;
  CModerator foldbackModerator;
public:
  CBluetoothHC05(int keyPin, int sensePin);
  void begin();
  void send(const char* Str);
  void check();
  virtual bool isConnected();
protected:
  virtual void openSerial(int baudrate);
  virtual void foldbackDesiredTemp();
  void flush();
};