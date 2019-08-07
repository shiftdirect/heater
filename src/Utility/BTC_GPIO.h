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

#ifndef __BTCGPIO_H__
#define __BTCGPIO_H__

#include <stdint.h>
#include <driver/adc.h>
#include "Debounce.h"
#include <list>

extern const char* GPIOin1Names[];
extern const char* GPIOin2Names[];
extern const char* GPIOout1Names[];
extern const char* GPIOout2Names[];
extern const char* GPIOalgNames[];

enum GPIOin1Modes { 
  GPIOin1None, 
  GPIOin1On,      // input 1 closure, heater starts; input2 closure, heater stops
  GPIOin1Hold,    // hold input 1 closure, heater runs; input 1 open, heater stops
  GPIOin1OnOff    // alternate input 1 closures start or stop the heater 
};
enum GPIOin2Modes { 
  GPIOin2None, 
  GPIOin2Off,          // input 2 closure stops heater
  GPIOin2ExtThermostat // input 2 used to max/min heater if closed/open
};


enum GPIOout1Modes { 
  GPIOout1None, 
  GPIOout1Status,
  GPIOout1User
};
enum GPIOout2Modes { 
  GPIOout2None, 
  GPIOout2User
};


enum GPIOalgModes {
  GPIOalgNone,   // Unmodified V2.0 PCBs must use this - ADC2 / Wifi unresolvable conflict
  GPIOalgHeatDemand,
};


struct sGPIOparams {
  GPIOin1Modes in1Mode;
  GPIOin2Modes in2Mode;
  GPIOout1Modes out1Mode;
  GPIOout2Modes out2Mode;
  GPIOalgModes algMode;
};


class CGPIOin {
  GPIOin1Modes _Mode1;
  GPIOin2Modes _Mode2;
  CDebounce _Debounce;
  uint8_t _lastKey;
  std::list<uint8_t> _eventList[2];
  void _doOn1(uint8_t newKey);
  void _doOff2(uint8_t newKey);
  void _doOnHold1(uint8_t newKey);
  void _doOn1Off1(uint8_t newKey);
public:
  CGPIOin();
  // void setMode(GPIOinModes mode) { _Mode = mode; };
  // void begin(int pin1, int pin2, GPIOinModes mode, int activeState);
  void setMode(GPIOin1Modes mode1, GPIOin2Modes mode2) { _Mode1 = mode1; _Mode2 = mode2; };
  void begin(int pin1, int pin2, GPIOin1Modes mode1, GPIOin2Modes mode2, int activeState);
  void manage();
  uint8_t getState(int channel);
  GPIOin1Modes getMode1() const;
  GPIOin2Modes getMode2() const;
  void simulateKey(uint8_t newKey);
  bool usesExternalThermostat() const { 
//    return (_Mode == GPIOinOnHold1) || (_Mode == GPIOinExtThermostat2); 
    return (_Mode2 == GPIOin2ExtThermostat); 
  };
};

class CGPIOout {
//  GPIOoutModes _Mode;
  GPIOout1Modes _Mode1;
  GPIOout2Modes _Mode2;
  void _doStatus();
  void _doUser1();
  void _doUser2();
  int _pins[2];
  int _prevState;
  int _statusState;
  int _statusDelay;
  unsigned long _breatheDelay;
  uint8_t _userState;
  void _doStartMode();
  void _doStopMode();
  void _doSuspendMode();
public:
  CGPIOout();
//  void setMode(GPIOoutModes mode);
//  void begin(int pin1, int pin2, GPIOoutModes mode);
  void setMode(GPIOout1Modes mode1, GPIOout2Modes mode2);
  void begin(int pin1, int pin2, GPIOout1Modes mode1, GPIOout2Modes mode2);
  void manage();
  void setState(int channel, bool state);
  bool getState(int channel);
//  GPIOoutModes getMode() const;
  GPIOout1Modes getMode1() const;
  GPIOout2Modes getMode2() const;
};

class CGPIOalg {
  GPIOalgModes _Mode;
  float _expMean;
  adc1_channel_t _pin;
public:
  CGPIOalg();
  void begin(adc1_channel_t pin, GPIOalgModes mode);
  void manage();
  int getValue();
  GPIOalgModes getMode() const;
};

struct sGPIO {
  bool outState[2];
  bool inState[2];
  int  algVal;
//  GPIOoutModes outMode;
//  GPIOinModes inMode;
  GPIOout1Modes out1Mode;
  GPIOout2Modes out2Mode;
  GPIOin1Modes in1Mode;
  GPIOin2Modes in2Mode;
  GPIOalgModes algMode;
  sGPIO& operator=(const sGPIO& rhs) {
    outState[0] = rhs.outState[0];
    outState[1] = rhs.outState[1];
    inState[0] = rhs.inState[0];
    inState[1] = rhs.inState[1];
    algVal = rhs.algVal;
    out1Mode = rhs.out1Mode;
    out2Mode = rhs.out2Mode;
    in1Mode = rhs.in1Mode;
    in2Mode = rhs.in2Mode;
    algMode = rhs.algMode;
    return *this;
  }
};

#endif // __BTCGPIO_H__
