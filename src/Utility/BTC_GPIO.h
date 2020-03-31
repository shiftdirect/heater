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



class CGPIOin1 {
public:
  enum Modes { 
    Disabled, 
    Start,     // input 1 closure starts heater
    Run,       // hold input 1 closed, heater runs; open, heater stops
    StartStop, // alternate input 1 closures start or stop the heater 
    Stop       // input 1 closure stops heater
  };
  CGPIOin1();
  void setMode(Modes mode) { _Mode = mode; };
  void begin(Modes mode);
  void manage(bool active);
  Modes getMode() const;
private:
  Modes _Mode;
  bool _prevActive;
  unsigned long _holdoff;
  void _doStart(bool active);
  void _doRun(bool active);
  void _doStartStop(bool active);
  void _doStop(bool active);
};

class CGPIOin2 {
public:
  enum Modes { 
    Disabled, 
    Stop,       // input 2 closure stops heater
    Thermostat,  // input 2 used to max/min heater if closed/open
    FuelReset
  };
  CGPIOin2();
  void setMode(Modes mode) { _Mode = mode; };
  void begin(Modes mode);
  void manage(bool active);
  Modes getMode() const;
  const char* getExtThermTime();
private:
  Modes _Mode;
  bool _prevActive;
  unsigned long _holdoff;
  void _doStop(bool active);
  void _doThermostat(bool active);
  void _doFuelReset(bool active);
};

class CGPIOin {
  CGPIOin1 _Input1;
  CGPIOin2 _Input2;
  CDebounce _Debounce;
  uint8_t _lastKey;
  std::list<uint8_t> _eventList[2];
public:
  CGPIOin();
  void setMode(CGPIOin1::Modes mode1, CGPIOin2::Modes mode2) { _Input1.setMode(mode1); _Input2.setMode(mode2); };
  void begin(int pin1, int pin2, CGPIOin1::Modes mode1, CGPIOin2::Modes mode2, int activeState);
  void manage();
  uint8_t getState(int channel);
  CGPIOin1::Modes  getMode1() const;
  CGPIOin2::Modes getMode2() const;
  void simulateKey(uint8_t newKey);
  bool usesExternalThermostat() const { 
    return (_Input2.getMode() == CGPIOin2::Thermostat); 
  };
  const char* getExtThermHoldTime() {
    return _Input2.getExtThermTime();
  }
};

class CGPIOoutBase {
  int  _thresh;
  bool _userState;
  int  _pin;
protected:
  void _doActive();
  void _doThresh();
  void _doUser();
  bool _getUserState();
  int _getPin() { return _pin; };
  void _setPinState(int state);
  int _getPinState();
public:
  CGPIOoutBase();
  void setState(bool state);
  void setThresh(int val);
  void begin(int pin);
};

class CGPIOout1 : public CGPIOoutBase {
public:
  enum Modes { 
    Disabled, 
    Status,
    User,
    Thresh,
    HtrActive
  };
  CGPIOout1();
  void begin(int pin, Modes mode);
  void setMode(Modes mode);
  void manage();
  uint8_t getState();
  Modes getMode() const;
private:
  Modes _Mode;
  void _doStatus();
  int _prevState;
  int _statusState;
  int _statusDelay;
  unsigned long _breatheDelay;
  uint8_t _ledState;
  void _doStartMode();
  void _doStopMode();
  void _doSuspendMode();
};

class CGPIOout2 : public CGPIOoutBase {
public:
  enum Modes { 
    Disabled, 
    User,
    Thresh,
    HtrActive,
  };
  CGPIOout2();
  void begin(int pin, Modes mode);
  void setMode(Modes mode);
  void manage();
  uint8_t getState();
  Modes getMode() const;
private:
  Modes _Mode;
};

class CGPIOout {
  CGPIOout1 _Out1;
  CGPIOout2 _Out2;
public:
  CGPIOout();
  void setMode(CGPIOout1::Modes mode1, CGPIOout2::Modes mode2);
  void begin(int pin1, int pin2, CGPIOout1::Modes mode1, CGPIOout2::Modes mode2);
  void manage();
  void setState(int channel, bool state);
  void setThresh(int thr1, int thr2);
  uint8_t getState(int channel);
  CGPIOout1::Modes getMode1() const;
  CGPIOout2::Modes getMode2() const;
};

class CGPIOalg {
public:
  enum Modes {
    Disabled,   // Unmodified V2.0 PCBs must use this - ADC2 / Wifi unresolvable conflict
    HeatDemand,
  };
  CGPIOalg();
  void begin(adc1_channel_t pin, Modes mode);
  void manage();
  int getValue();
  Modes getMode() const;
private:
  Modes _Mode;
  float _expMean;
  adc1_channel_t _pin;

};

struct sGPIOparams {
  CGPIOin1::Modes in1Mode;
  CGPIOin2::Modes in2Mode;
  CGPIOout1::Modes out1Mode;
  CGPIOout2::Modes out2Mode;
  CGPIOalg::Modes algMode;
  int8_t thresh[2];
};

struct sGPIO {
  uint8_t outState[2];
  uint8_t inState[2];
  int  algVal;
  sGPIO() {
    outState[0] = outState[1] = false;
    inState[0] = inState[1] = false;
    algVal = 0;
    out1Mode = CGPIOout1::Disabled;
    out2Mode = CGPIOout2::Disabled;
    in1Mode = CGPIOin1::Disabled;
    in2Mode = CGPIOin2::Disabled;
    algMode = CGPIOalg::Disabled;
  };
  CGPIOout1::Modes out1Mode;
  CGPIOout2::Modes out2Mode;
  CGPIOin1::Modes in1Mode;
  CGPIOin2::Modes in2Mode;
  CGPIOalg::Modes algMode;
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

extern CGPIOin GPIOin;
extern CGPIOout GPIOout;

#endif // __BTCGPIO_H__
