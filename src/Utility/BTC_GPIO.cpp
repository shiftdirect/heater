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

#include "BTC_GPIO.h"
#include "helpers.h"
#include <driver/adc.h>
#include "DebugPort.h"
#include "../Protocol/Protocol.h"

const int BREATHINTERVAL = 45;
const int FADEAMOUNT = 3;
const int FLASHPERIOD = 2000;
const int ONFLASHINTERVAL = 50;

// const char* GPIOinNames[] = {
//   "Disabled",
//   "On1Off2",
//   "Hold1",
//   "On1Off1",
//   "ExtThermostat"
// };

const char* GPIOin1Names[] = {
  "Disabled",
  "Mom On",
  "Hold On",
  "Mom On/Off"
};
const char* GPIOin2Names[] = {
  "Disabled",
  "Mom Off",
  "Ext \352T"
};


// const char* GPIOoutNames[] = {
//   "Disabled",
//   "Status",
//   "User"
// };
const char* GPIOout1Names[] = {
  "Disabled",
  "Status",
  "User"
};
const char* GPIOout2Names[] = {
  "Disabled",
  "User"
};

const char* GPIOalgNames[] = {
  "Disabled",
  "HeatDemand"
};


CGPIOin::CGPIOin()
{
  _Mode1 = GPIOin1None;
  _Mode2 = GPIOin2None;
  _lastKey = 0;
}

void 
CGPIOin::begin(int pin1, int pin2, GPIOin1Modes mode1, GPIOin2Modes mode2, int activeState)
{
  _Debounce.addPin(pin1);
  _Debounce.addPin(pin2);
  _Debounce.setActiveState(activeState);

  setMode(mode1, mode2);
}

uint8_t 
CGPIOin::getState(int channel) 
{ 
  uint8_t retval = 0;

  if((channel & ~0x01) == 0) {
    // index is in bounds 0 or 1

    // check for transient events
    if(_eventList[channel].empty()) {
      // read last actual state
      int mask = 0x01 << (channel & 0x01);
      retval = (_Debounce.getState() & mask) != 0; 
    }
    else {
      // emit transient events if they occured
      retval = _eventList[channel].front() != 0;
      _eventList[channel].pop_front();
    }
  }
  return retval;
}

GPIOin1Modes CGPIOin::getMode1() const
{
  return _Mode1;
};

GPIOin2Modes CGPIOin::getMode2() const
{
  return _Mode2;
};

void 
CGPIOin::manage() 
{
  uint8_t newKey = _Debounce.manage();
  // determine edge events
  uint8_t keyChange = newKey ^ _lastKey;
  _lastKey = newKey;

  if(keyChange) {
    simulateKey(newKey);
    
    // record possible sub sample transients - JSON usage especially
    if(keyChange & 0x01)
      _eventList[0].push_back(newKey & 0x01);  // mask the channel bit
    if(keyChange & 0x02)
      _eventList[1].push_back(newKey & 0x02);  // mask the channel bit
  }
}

void 
CGPIOin::simulateKey(uint8_t newKey)
{
  switch (_Mode1) {
    case GPIOin1None:
      break;
    case GPIOin1On:
      _doOn1(newKey);
      break;
    case GPIOin1Hold:
      _doOnHold1(newKey);
      break;
    case GPIOin1OnOff:
      _doOn1Off1(newKey);
      break;
  }
  switch (_Mode2) {
    case GPIOin2None:
      break;
    case GPIOin2Off:
      _doOff2(newKey);
      break;
    case GPIOin2ExtThermostat:
      break;  // handling actually performed at Tx Manage for setting the fuel rate
  }
}

void 
CGPIOin::_doOn1(uint8_t newKey)
{
  if(newKey & 0x01) {
    requestOn();
  }
}

void 
CGPIOin::_doOff2(uint8_t newKey)
{
  if(newKey & 0x02) {
    requestOff();
  }
}

// mode where heater runs if input 1 is shorted
// stops when open
void 
CGPIOin::_doOnHold1(uint8_t newKey)
{
  if(newKey & 0x01) {
    requestOn();
  }
  else {
    requestOff();
  }
}

// mode where heater runs if input 1 is shorted
// stops when open
void 
CGPIOin::_doOn1Off1(uint8_t newKey)
{
  if(newKey & 0x01) {
    if(getHeaterInfo().getRunStateEx())
      requestOff();
    else 
      requestOn();
  }
}



CGPIOout::CGPIOout()
{
  _Mode1 = GPIOout1None;
  _Mode2 = GPIOout2None;
  _pins[0] = 0;
  _pins[1] = 0;
  _breatheDelay = 0;
  _statusState = 0;
  _statusDelay = 0;
  _userState = 0;
  _prevState = -1;
}

void 
CGPIOout::begin(int pin1, int pin2, GPIOout1Modes mode1, GPIOout2Modes mode2)
{
  _pins[0] = pin1;
  _pins[1] = pin2;
  if(pin1) {
    pinMode(pin1, OUTPUT);   // GPIO output pin #1
    digitalWrite(pin1, LOW);
    ledcSetup(0, 500, 8);   // create PWM channel for GPIO1: 500Hz, 8 bits
  }
  if(pin2) {
    pinMode(pin2, OUTPUT);   // GPIO output pin #2
    digitalWrite(pin2, LOW);
    ledcSetup(1, 500, 8);   // create PWM channel for GPIO2: 500Hz, 8 bits 
  }

  setMode(mode1, mode2);
}

void 
CGPIOout::setMode(GPIOout1Modes mode1, GPIOout2Modes mode2) 
{ 
  _Mode1 = mode1; 
  _Mode2 = mode2; 
  _prevState = -1;
  ledcDetachPin(_pins[0]);     // ensure PWM detached from IO line
  ledcDetachPin(_pins[1]);     // ensure PWM detached from IO line
};

GPIOout1Modes CGPIOout::getMode1() const
{
  return _Mode1;
};

GPIOout2Modes CGPIOout::getMode2() const
{
  return _Mode2;
};

void
CGPIOout::manage()
{
  switch (_Mode1) {
    case GPIOout1None: break;
    case GPIOout1Status: _doStatus(); break;
    case GPIOout1User: _doUser1(); break;
  }
  switch (_Mode2) {
    case GPIOout2None: break;
    case GPIOout2User: _doUser2(); break;
  }
}

void
CGPIOout::_doStatus()
{
  if(_pins[0] == 0) 
    return;

//  DebugPort.println("GPIOout::_doStatus()");
  int runstate = getHeaterInfo().getRunStateEx();
  int statusMode = 0;
  switch(runstate) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 9:
      // starting modes
      statusMode = 1; 
      break;
    case 5:
      // run mode
      statusMode = 2; 
      break;    
    case 6:
    case 7:
    case 8:
    case 11:
    case 12:
      // cooldown modes
      statusMode = 3; 
      break;
    case 10:
      // suspend mode
      statusMode = 4;
      break;
  }

  // change of mode typically requires changing from simple digital out 
  // to PWM or vice versa
  if(_prevState != statusMode) {
    _prevState = statusMode;
    _statusState = 0;
    _statusDelay = millis() + BREATHINTERVAL;
    switch(statusMode) {
      case 0:
        ledcDetachPin(_pins[0]);     // detach PWM from IO line
        digitalWrite(_pins[0], LOW);
        break;
      case 1:
        ledcAttachPin(_pins[0], 0);  // attach PWM to GPIO line
        ledcWrite(0, _statusState);
        _breatheDelay = millis() + BREATHINTERVAL; 
        break;
      case 2:
        ledcDetachPin(_pins[0]);     // detach PWM from IO line
        digitalWrite(_pins[0], HIGH);
        break;
      case 3:
        ledcAttachPin(_pins[0], 0);  // attach PWM to GPIO line
        _statusState = 255;
        ledcWrite(0, _statusState);
        _breatheDelay = millis() + BREATHINTERVAL; 
        break;
      case 4:
        ledcDetachPin(_pins[0]);     // detach PWM from IO line
        _breatheDelay += (FLASHPERIOD - ONFLASHINTERVAL);  // extended off
        digitalWrite(_pins[0], LOW);
        break;
    }  
  }
  switch(statusMode) {
    case 1: _doStartMode(); break;
    case 3: _doStopMode(); break;
    case 4: _doSuspendMode(); break;
  }
}

void
CGPIOout::_doUser1()
{
//  DebugPort.println("GPIOout::_doUser1()");
  if(_pins[0]) {
    digitalWrite(_pins[0], (_userState & 0x01) ? HIGH : LOW);
  }
}

void
CGPIOout::_doUser2()
{
//  DebugPort.println("GPIOout::_doUser2()");
  if(_pins[1]) {
    digitalWrite(_pins[1], (_userState & 0x02) ? HIGH : LOW);
  }
}

void 
CGPIOout::_doStartMode()   // breath up PWM
{
  long tDelta = millis() - _breatheDelay;
  if(tDelta >= 0) {
    _breatheDelay += BREATHINTERVAL;
    int expo = ((_statusState >> 5) + 1);
    _statusState += expo;
    _statusState &= 0xff;
    ledcWrite(0, _statusState);
  }
}

void 
CGPIOout::_doStopMode()   // breath down PWM
{
  long tDelta = millis() - _breatheDelay;
  if(tDelta >= 0) {
    _breatheDelay += BREATHINTERVAL;
    int expo = ((_statusState >> 5) + 1);
    _statusState -= expo;
    _statusState &= 0xff;
    ledcWrite(0, _statusState);

  }
}

void 
CGPIOout::_doSuspendMode()  // brief flash
{
  long tDelta = millis() - _breatheDelay;
  if(tDelta >= 0) {
    _statusState++;
    if(_statusState & 0x01) {
      _breatheDelay += ONFLASHINTERVAL;  // brief flash on
      digitalWrite(_pins[0], HIGH);
    }
    else {
      _breatheDelay += (FLASHPERIOD - ONFLASHINTERVAL);  // extended off
      digitalWrite(_pins[0], LOW);
    }
  }
}

void 
CGPIOout::setState(int channel, bool state)
{
  int mask = 0x01 << (channel & 0x01);
  if(state)
    _userState |= mask;
  else
    _userState &= ~mask;
}

bool
CGPIOout::getState(int channel)
{
  int mask = 0x01 << (channel & 0x01);
  return (_userState & mask) != 0;
}


// expected external analogue circuit is a 10k pot.
//   Top end of pot is connected to GPIO Vcc (red wire) via 5k6 fixed resistor. (GPIO Vcc is 5V via schottky diode)
//   Bottom end of pot is connected to GND (black wire) via 1k fixed resistor.
//   Wiper is into Pin 6 of GPIO (white wire) - analogue input

CGPIOalg::CGPIOalg()
{
  _expMean = 0;
}

void
CGPIOalg::begin(adc1_channel_t pin, GPIOalgModes mode)
{
  _pin = pin;
  _Mode = mode;

  if(_Mode != GPIOalgNone) {
    adc_gpio_init(ADC_UNIT_1, ADC_CHANNEL_5);
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_11db);
  }
}

GPIOalgModes CGPIOalg::getMode() const
{
  return _Mode;
};


void CGPIOalg::manage()
{
  const float fAlpha = 0.95;           // exponential mean alpha

  if(_Mode != GPIOalgNone) {
    int read_raw;
    char msg[32];
    read_raw = adc1_get_raw( ADC1_CHANNEL_5);
    sprintf(msg, "ADC: %d", read_raw );
    _expMean = _expMean * fAlpha + (1-fAlpha) * float(read_raw);
//    DebugPort.println(msg);
  }
}

int 
CGPIOalg::getValue()
{
  return _expMean;
}
