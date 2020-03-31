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
#include "../RTC/RTCStore.h"

const int BREATHINTERVAL = 45;
const int FADEAMOUNT = 3;
const int FLASHPERIOD = 2000;
const int ONFLASHINTERVAL = 50;

const char* GPIOin1Names[] = {
  "Disabled",
  "Mom On",
  "Hold On",
  "Mom On/Off",
  "Mom Off"
};
const char* GPIOin2Names[] = {
  "Disabled",
  "Mom Off",
  "Ext Thermo",
  "Fuel Reset"
};


const char* GPIOout1Names[] = {
  "Disabled",
  "Status",
  "User",
  "Thresh",
  "HeaterOn"
};
const char* GPIOout2Names[] = {
  "Disabled",
  "User",
  "Thresh",
  "HeaterOn"
};

const char* GPIOalgNames[] = {
  "Disabled",
  "Enabled",
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
      case Stop:      _doStop(active); break;
    }
    _prevActive = active;
  }
}


// mode where you can start the heater with a short press
// stop the heater with a long press
void 
CGPIOin1::_doStart(bool active)
{
  if(active && !_prevActive) {
    _holdoff = millis();
  }
  if(!active && _prevActive) {
    unsigned long tDelta = millis() - _holdoff;
    if(tDelta > 50) {
      if(tDelta < 1500)  // longer or shorter than 1.5 seconds?
        requestOn();    // short press is start
      else 
        requestOff();   // long press is stop
    }
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
    if(getHeaterInfo().getRunStateEx() && !RTC_Store.getFrostOn())
      requestOff();
    else 
      requestOn();
  }
}

void 
CGPIOin1::_doStop(bool active)
{
  if(active) {
    requestOff();
  }
}

CGPIOin2::CGPIOin2()
{
  _Mode = Disabled;
  _prevActive = false;
  _holdoff = 0;
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
    case FuelReset:  _doFuelReset(active); break;
  }
  _prevActive = active;
}

void 
CGPIOin2::_doStop(bool active)
{
  if(_prevActive ^ active) {
    if(active) {
      requestOff();
    }
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
      _holdoff = (millis() + NVstore.getUserSettings().ExtThermoTimeout) | 1;
      DebugPort.printf("thermostat contact opened - will stop in %ldms\r\n", NVstore.getUserSettings().ExtThermoTimeout);
    }
    if(active) {
      _holdoff = 0;
      int runstate = getHeaterInfo().getRunStateEx();
      int errstate = getHeaterInfo().getErrState(); 
      if(runstate == 0 && errstate == 0) {
        requestOn();   // request heater to start upon closure of thermostat input (may have shutdown before contact closed again)
      }
    }
    else {
      if(_holdoff) {
        long tDelta = millis() - _holdoff;
        if(tDelta >= 0) {
          DebugPort.println("stopping heater due to thermostat contact being open for required dwell");
          requestOff();  // request heater to stop after thermostat input has stayed open for interval
          _holdoff = 0;
        }
      }
    }
  }
  // handling actually performed at Tx Manage for setting the fuel rate
}


void
CGPIOin2::_doFuelReset(bool active) 
{
  if(active) {
    if(!_prevActive) {
      _holdoff = millis() + 1000;  // require 1 second hold to reset fuel gauge
    }
    if(_holdoff) {
      long tDelta = millis() - _holdoff;
      if(tDelta > 0) {            // 1 second has expired
        resetFuelGauge();
        _holdoff = 0;
      } 
    }
  }
  else {
    _holdoff = 0;                // ensure fresh
  }
}

const char* 
CGPIOin2::   getExtThermTime()
{
  if((_holdoff == 0) || (NVstore.getUserSettings().ThermostatMethod != 3) || (NVstore.getUserSettings().ExtThermoTimeout == 0)) 
    return NULL;

  long tDelta = _holdoff - millis();
  if(tDelta < 0)
    return NULL;

  long secs = tDelta / 1000;
  long mins = secs / 60;
  secs -= mins * 60;
  static char timeStr[8];
  sprintf(timeStr, "%02ld:%02ld", mins, secs);
  return timeStr;
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
 ** GPIO out base class
 *********************************************************************************************************/
CGPIOoutBase::CGPIOoutBase()
{
  _pin = 0;
  _thresh = 0;
  _userState = 0;
}

void
CGPIOoutBase::begin(int pin) 
{
  _pin = pin;
  if(pin) {
    pinMode(pin, OUTPUT);   // GPIO output pin #1
    digitalWrite(pin, LOW);
  }
}

void
CGPIOoutBase::setThresh(int thresh)
{
  _thresh = thresh;
}

void 
CGPIOoutBase::setState(bool state)
{
  _userState = state;
}

bool 
CGPIOoutBase::_getUserState() 
{ 
  return _userState; 
};

void 
CGPIOoutBase::_setPinState(int state)
{
  digitalWrite(_pin, state);
}

int 
CGPIOoutBase::_getPinState() 
{ 
  return digitalRead(_pin); 
};


void
CGPIOoutBase::_doUser()
{
//  DebugPort.println("GPIOout::_doUser2()");
  if(_pin) {
    digitalWrite(_pin, _userState ? HIGH : LOW);
  }
}


void 
CGPIOoutBase::_doThresh()
{
  if(_thresh) {
    float tAct = getTemperatureSensor(0);
    if(digitalRead(_pin)) {   
      // output is currently active
      if(_thresh > 0)  {               // active when OVER threshold mode
        if((tAct + 0.1) < _thresh) {   // test if under threshold +0.1deg hysteresis
          digitalWrite(_pin, LOW);     // deactivate output when less than threshold
        }
      }
      else {                           // active if UNDER threshold mode
        if(tAct > -_thresh) {          // inactive if over threshold
          digitalWrite(_pin, LOW);     // deactivate output when over threshold
        }
      }
    }
    else {  
      // output is not currently active
      if(_thresh > 0)  {               // active when OVER threshold mode
        if(tAct > _thresh) {           // test if over threshold 
          digitalWrite(_pin, HIGH);    // activate output when over threshold
        }
      }
      else {                           // active if UNDER threshold mode
        if((tAct + 0.1) < -_thresh) {  // test if under threshold +0.1deg hysteresis
          digitalWrite(_pin, HIGH);    // activate output when under threshold
        }
      }
    }
  }
}

void 
CGPIOoutBase::_doActive()
{
  int runstate = getHeaterInfo().getRunState();  // raw state, not suspend mode enhanced
  digitalWrite(_pin, runstate ? HIGH : LOW);    // activates output when heater is not in standby
}

/*********************************************************************************************************
 ** GPIO out manager
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

void 
CGPIOout::setThresh(int op1, int op2)
{
  _Out1.setThresh(op1);
  _Out2.setThresh(op2);
}

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


CGPIOout1::CGPIOout1() : CGPIOoutBase()
{
  _Mode = Disabled;
  _breatheDelay = 0;
  _statusState = 0;
  _statusDelay = 0;
  _prevState = -1;
}

void 
CGPIOout1::begin(int pin, CGPIOout1::Modes mode) 
{
  CGPIOoutBase::begin(pin);

  ledcSetup(0, 500, 8);   // create PWM channel for GPIO1: 500Hz, 8 bits

  setMode(mode);
}

void 
CGPIOout1::setMode(CGPIOout1::Modes mode) 
{
  if(mode >= Disabled && mode <= HtrActive) 
    _Mode = mode; 
  _prevState = -1;
  if(_getPin())
    ledcDetachPin(_getPin());     // ensure PWM detached from IO line
};

CGPIOout1::Modes CGPIOout1::getMode() const
{
  return _Mode;
};

void
CGPIOout1::manage()
{
  switch (_Mode) {
    case CGPIOout1::Disabled:   break;
    case CGPIOout1::Status: _doStatus(); break;
    case CGPIOout1::User:   _doUser(); break;
    case CGPIOout1::Thresh: _doThresh(); break;
    case CGPIOout1::HtrActive: _doActive(); break;
  }
}

void
CGPIOout1::_doStatus()
{
  int pin = _getPin();
  if(pin == 0) 
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
        ledcDetachPin(pin);     // detach PWM from IO line
        _setPinState(LOW);
        _ledState = 0;
        break;
      case 1:
        ledcAttachPin(pin, 0);  // attach PWM to GPIO line
        ledcWrite(0, _statusState);
        _breatheDelay = millis() + BREATHINTERVAL; 
        _ledState = 2;
        break;
      case 2:
        ledcDetachPin(pin);     // detach PWM from IO line
        _setPinState(HIGH);
        _ledState = 1;
        break;
      case 3:
        ledcAttachPin(pin, 0);  // attach PWM to GPIO line
        _statusState = 255;
        ledcWrite(0, _statusState);
        _breatheDelay = millis() + BREATHINTERVAL; 
        _ledState = 3;
        break;
      case 4:
        ledcDetachPin(pin);     // detach PWM from IO line
        _breatheDelay += (FLASHPERIOD - ONFLASHINTERVAL);  // extended off
        _setPinState(LOW);
        _ledState = 4;
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
  _ledState = 3;
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
      _setPinState(HIGH);
      stretch = (millis() + 250) | 1;   // pulse extend for UI purposes, ensure non zero
    }
    else {
      _breatheDelay += (FLASHPERIOD - ONFLASHINTERVAL);  // extended off
      _setPinState(LOW);
    }
  }
  if(stretch) {
    tDelta = millis() - stretch;
    if(tDelta >= 0)
      stretch = 0;
  }
  _ledState = 4;
}

uint8_t
CGPIOout1::getState()
{
  switch(_Mode) {
    case User: 
    case Thresh:
    case HtrActive:
      return _getPinState();
    case Status: 
      return _ledState;   // special pulse extender for suspend mode
    default:
      return 0;
  } 
}

/*********************************************************************************************************
 ** GPIO2
 *********************************************************************************************************/
CGPIOout2::CGPIOout2() : CGPIOoutBase()
{
  _Mode = Disabled;
}

void 
CGPIOout2::begin(int pin, Modes mode)
{
  CGPIOoutBase::begin(pin);

  ledcSetup(1, 500, 8);   // create PWM channel for GPIO2: 500Hz, 8 bits 

  setMode(mode);
}

void 
CGPIOout2::setMode(CGPIOout2::Modes mode) 
{ 
  if(mode >= Disabled && mode <= HtrActive) 
    _Mode = mode; 
  int pin = _getPin();
  if(pin)
    ledcDetachPin(pin);     // ensure PWM detached from IO line
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
    case CGPIOout2::Thresh: _doThresh(); break;
    case CGPIOout2::HtrActive: _doActive(); break;
  }
}


uint8_t
CGPIOout2::getState()
{
  switch (_Mode) {
    case CGPIOout2::User: 
    case CGPIOout2::Thresh: 
    case CGPIOout2::HtrActive: 
      return _getPinState();
    default:
      return 0;
  }
}

// expected external analogue circuit is a 10k pot.
//   Top end of pot is connected to GPIO Vcc (red wire) via 5k6 fixed resistor. (GPIO Vcc is 5V via schottky diode)
//   Bottom end of pot is connected to GND (black wire) via 1k fixed resistor.
//   Wiper is into Pin 6 of GPIO (white wire) - analogue input

CGPIOalg::CGPIOalg()
{
  _expMean = 0;
  _Mode = Disabled;
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
