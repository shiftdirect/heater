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
#include "../Utility/NVStorage.h"

const int BREATHINTERVAL = 45;
const int FADEAMOUNT = 3;
const int FLASHPERIOD = 2000;
const int ONFLASHINTERVAL = 50;

const char* GPIOin1Names[] = {
  "Disabled",
  "Mom On",
  "Hold On",
  "Mom On/Off"
};
const char* GPIOin2Names[] = {
  "Disabled",
  "Mom Off",
  "Ext Thermo"
};


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

CGPIOin1::CGPIOin1()
{
  _Mode = Disabled;
  _prevActive = false;
}

void 
CGPIOin1::begin(CGPIOin1::Modes mode)
{
  setMode(mode);
}

CGPIOin1::Modes CGPIOin1::getMode() const
{
  return _Mode;
};

void 
CGPIOin1::manage(bool active)
{
  if(_prevActive ^ active) {
    switch (_Mode) {
      case Disabled:  break;
      case Start:     _doStart(active); break;
      case Run:       _doRun(active); break;
      case StartStop: _doStartStop(active); break;
    }
    _prevActive = active;
  }
}

void 
CGPIOin1::_doStart(bool active)
{
  if(active) {
    requestOn();
  }
}

// mode where heater runs if input 1 is shorted
// stops when open
void 
CGPIOin1::_doRun(bool active)
{
  if(active) {
    requestOn();
  }
  else {
    requestOff();
  }
}

// mode where heater runs if input 1 is shorted
// stops when open
void 
CGPIOin1::_doStartStop(bool active)
{
  if(active) {
    if(getHeaterInfo().getRunStateEx())
      requestOff();
    else 
      requestOn();
  }
}

CGPIOin2::CGPIOin2()
{
  _Mode = Disabled;
  _prevActive = false;
  _OffHoldoff = 0;
}

void 
CGPIOin2::begin(CGPIOin2::Modes mode)
{
  setMode(mode);
}

CGPIOin2::Modes CGPIOin2::getMode() const
{
  return _Mode;
};

void 
CGPIOin2::manage(bool active)
{
  switch (_Mode) {
    case Disabled:   break;
    case Stop:       _doStop(active); break;
    case Thermostat: _doThermostat(active); break;
  }
}

void 
CGPIOin2::_doStop(bool active)
{
  if(_prevActive ^ active) {
    if(active) {
      requestOff();
    }
    _prevActive = active;
  }
}

// mode where heater runs if input 1 is shorted
// stops when open
void 
CGPIOin2::_doThermostat(bool active)
{
    // only if actually using thermostat input, and a timeout is defined do we perform heater start / stop functions
  if((NVstore.getUserSettings().ThermostatMethod == 3) && NVstore.getUserSettings().ExtThermoTimeout) {
    if(active && !_prevActive)  {  // initial switch on of thermostat input
      DebugPort.println("starting heater due to thermostat contact closure");
      requestOn();   // request heater to start upon closure of thermostat input
    }
    if(!active && _prevActive)  {  // initial switch off of thermostat input
      _OffHoldoff = (millis() + NVstore.getUserSettings().ExtThermoTimeout) | 1;
      DebugPort.printf("thermostat contact opened - will stop in %ldms\r\n", NVstore.getUserSettings().ExtThermoTimeout);
    }
    if(!active) {
      if(_OffHoldoff) {
        long tDelta = millis() - _OffHoldoff;
        if(tDelta >= 0) {
          DebugPort.println("stopping heater due to thermostat contact being open for required dwell");
          requestOff();  // request heater to stop after thermostat input has stayed open for interval
          _OffHoldoff = 0;
        }
      }
    }
    _prevActive = active;
  }
  // handling actually performed at Tx Manage for setting the fuel rate
}



CGPIOin::CGPIOin()
{
  _Input1.setMode(CGPIOin1::Disabled);
  _Input2.setMode(CGPIOin2::Disabled);
  _lastKey = 0;
}

void 
CGPIOin::begin(int pin1, int pin2, CGPIOin1::Modes mode1, CGPIOin2::Modes mode2, int activeState)
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

CGPIOin1::Modes CGPIOin::getMode1() const
{
  return _Input1.getMode();
};

CGPIOin2::Modes CGPIOin::getMode2() const
{
  return _Input2.getMode();
};

void 
CGPIOin::manage() 
{
  uint8_t newKey = _Debounce.manage();
  // determine edge events
  uint8_t keyChange = newKey ^ _lastKey;
  _lastKey = newKey;

  if(keyChange) {
    
    // record possible sub sample transients - JSON usage especially
    if(keyChange & 0x01)
      _eventList[0].push_back(newKey & 0x01);  // mask the channel bit
    if(keyChange & 0x02)
      _eventList[1].push_back(newKey & 0x02);  // mask the channel bit
  }
  simulateKey(newKey);
}

void 
CGPIOin::simulateKey(uint8_t newKey)
{
  _Input1.manage((newKey & 0x01) != 0);
  _Input2.manage((newKey & 0x02) != 0);
}

/*********************************************************************************************************
 ** GPIO out root
 *********************************************************************************************************/


CGPIOout::CGPIOout()
{
}

void 
CGPIOout::begin(int pin1, int pin2, CGPIOout1::Modes mode1, CGPIOout2::Modes mode2)
{
  _Out1.begin(pin1, mode1);
  _Out2.begin(pin2, mode2);
}

void 
CGPIOout::setMode(CGPIOout1::Modes mode1, CGPIOout2::Modes mode2) 
{ 
  _Out1.setMode(mode1);
  _Out2.setMode(mode2);
};

CGPIOout1::Modes 
CGPIOout::getMode1() const
{
  return _Out1.getMode();
};

CGPIOout2::Modes 
CGPIOout::getMode2() const
{
  return _Out2.getMode();
};

void
CGPIOout::manage()
{
  _Out1.manage();
  _Out2.manage();
}


void 
CGPIOout::setState(int channel, bool state)
{
  if(channel)
    _Out2.setState(state);
  else
    _Out1.setState(state);
}

uint8_t
CGPIOout::getState(int channel)
{
  if(channel)
    return _Out2.getState();
  else
    return _Out1.getState();
}


/*********************************************************************************************************
 ** GPIO out #1
 *********************************************************************************************************/


CGPIOout1::CGPIOout1()
{
  _Mode = Disabled;
  _pin = 0;
  _breatheDelay = 0;
  _statusState = 0;
  _statusDelay = 0;
  _userState = 0;
  _prevState = -1;
}

void 
CGPIOout1::begin(int pin, CGPIOout1::Modes mode)
{
  _pin = pin;
  if(pin) {
    pinMode(pin, OUTPUT);   // GPIO output pin #1
    digitalWrite(pin, LOW);
    ledcSetup(0, 500, 8);   // create PWM channel for GPIO1: 500Hz, 8 bits
  }

  setMode(mode);
}

void 
CGPIOout1::setMode(Modes mode) 
{ 
  _Mode = mode; 
  _prevState = -1;
  ledcDetachPin(_pin);     // ensure PWM detached from IO line
};

CGPIOout1::Modes CGPIOout1::getMode() const
{
  return _Mode;
};

void
CGPIOout1::manage()
{
  switch (_Mode) {
    case Disabled:   break;
    case Status: _doStatus(); break;
    case User:   _doUser(); break;
  }
}

void
CGPIOout1::_doStatus()
{
  if(_pin == 0) 
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
        ledcDetachPin(_pin);     // detach PWM from IO line
        digitalWrite(_pin, LOW);
        _ledState = 0;
        break;
      case 1:
        ledcAttachPin(_pin, 0);  // attach PWM to GPIO line
        ledcWrite(0, _statusState);
        _breatheDelay = millis() + BREATHINTERVAL; 
        break;
      case 2:
        ledcDetachPin(_pin);     // detach PWM from IO line
        digitalWrite(_pin, HIGH);
        _ledState = 1;
        break;
      case 3:
        ledcAttachPin(_pin, 0);  // attach PWM to GPIO line
        _statusState = 255;
        ledcWrite(0, _statusState);
        _breatheDelay = millis() + BREATHINTERVAL; 
        break;
      case 4:
        ledcDetachPin(_pin);     // detach PWM from IO line
        _breatheDelay += (FLASHPERIOD - ONFLASHINTERVAL);  // extended off
        digitalWrite(_pin, LOW);
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
CGPIOout1::_doUser()
{
//  DebugPort.println("GPIOout::_doUser2()");
  if(_pin) {
    digitalWrite(_pin, _userState ? HIGH : LOW);
  }
}

void 
CGPIOout1::_doStartMode()   // breath up PWM
{
  long tDelta = millis() - _breatheDelay;
  if(tDelta >= 0) {
    _breatheDelay += BREATHINTERVAL;
    int expo = ((_statusState >> 5) + 1);
    _statusState += expo;
    _statusState &= 0xff;
    ledcWrite(0, _statusState);
  }
  _ledState = 2;
}

void 
CGPIOout1::_doStopMode()   // breath down PWM
{
  long tDelta = millis() - _breatheDelay;
  if(tDelta >= 0) {
    _breatheDelay += BREATHINTERVAL;
    int expo = ((_statusState >> 5) + 1);
    _statusState -= expo;
    _statusState &= 0xff;
    ledcWrite(0, _statusState);
  }
  _ledState = 2;
}

void 
CGPIOout1::_doSuspendMode()  // brief flash
{
  static unsigned long stretch = 0;

  long tDelta = millis() - _breatheDelay;
  if(tDelta >= 0) {
    _statusState++;
    if(_statusState & 0x01) {
      _breatheDelay += ONFLASHINTERVAL;  // brief flash on
      digitalWrite(_pin, HIGH);
      stretch = (millis() + 250) | 1;   // pulse extend for UI purposes, ensure non zero
    }
    else {
      _breatheDelay += (FLASHPERIOD - ONFLASHINTERVAL);  // extended off
      digitalWrite(_pin, LOW);
    }
  }
  if(stretch) {
    tDelta = millis() - stretch;
    if(tDelta >= 0)
      stretch = 0;
  }
  _ledState = stretch ? 1 : 0;
}

void 
CGPIOout1::setState(bool state)
{
  _userState = state;
}

uint8_t
CGPIOout1::getState()
{
  switch(_Mode) {
    case User:   return _userState;
    case Status: return _ledState;   // special pulse extender for suspend mode
    default:     return 0;
   } 
}

/*********************************************************************************************************
 ** GPIO2
 *********************************************************************************************************/
CGPIOout2::CGPIOout2()
{
  _Mode = Disabled;
  _pin = 0;
  _userState = 0;
}

void 
CGPIOout2::begin(int pin, Modes mode)
{
  _pin = pin;
  if(pin) {
    pinMode(pin, OUTPUT);   // GPIO output pin #2
    digitalWrite(pin, LOW);
    ledcSetup(1, 500, 8);   // create PWM channel for GPIO2: 500Hz, 8 bits 
  }

  setMode(mode);
}

void 
CGPIOout2::setMode(CGPIOout2::Modes mode) 
{ 
  _Mode = mode; 
  if(_pin)
    ledcDetachPin(_pin);     // ensure PWM detached from IO line
};

CGPIOout2::Modes CGPIOout2::getMode() const
{
  return _Mode;
};

void
CGPIOout2::manage()
{
  switch (_Mode) {
    case CGPIOout2::Disabled: break;
    case CGPIOout2::User: _doUser(); break;
  }
}

void
CGPIOout2::_doUser()
{
  if(_pin) {
    digitalWrite(_pin, _userState ? HIGH : LOW);
  }
}

void 
CGPIOout2::setState(bool state)
{
  _userState = state;
}

uint8_t
CGPIOout2::getState()
{
  return _userState;
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
CGPIOalg::begin(adc1_channel_t pin, CGPIOalg::Modes mode)
{
  _pin = pin;
  _Mode = mode;

  if(_Mode != CGPIOalg::Disabled) {
    adc_gpio_init(ADC_UNIT_1, ADC_CHANNEL_5);
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_11db);
  }
}

CGPIOalg::Modes CGPIOalg::getMode() const
{
  return _Mode;
};


void CGPIOalg::manage()
{
  const float fAlpha = 0.95;           // exponential mean alpha

  if(_Mode != CGPIOalg::Disabled) {
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
