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

#include "128x64OLED.h"
#include "GPIOInfoScreen.h"
#include "KeyPad.h"
#include "../Utility/NVStorage.h"
#include "../Utility/BTC_GPIO.h"
#include "fonts/Icons.h"
#include "../Utility/BoardDetect.h"

#if USE_JTAG == 0
extern CGPIOout GPIOout;
extern CGPIOin GPIOin;
extern CGPIOalg GPIOalg;
#endif

static const int Line3 = 14;
static const int Line2 = 27;
static const int Line1 = 40;
static const int Column1 = 19;
static const int Column2 = 83;

CGPIOInfoScreen::CGPIOInfoScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreen(display, mgr) 
{
  _keyRepeatCount = -1;
}

void
CGPIOInfoScreen::_initUI()
{
}

bool 
CGPIOInfoScreen::animate()
{
  char msg[16];

  _display.clearDisplay();
  _showTitle("GPIO status");

  _drawBitmap(0, 14, InputIconInfo); 
  _drawBitmap(11, 14, _1IconInfo); 
  _drawBitmap(0, 27, InputIconInfo); 
  _drawBitmap(11, 27, _2IconInfo); 
  _drawBitmap(75, 14, OutputIconInfo); 
  _drawBitmap(86, 14, _1IconInfo); 
  _drawBitmap(75, 27, OutputIconInfo); 
  _drawBitmap(86, 27, _2IconInfo); 

  if(getBoardRevision() == BRD_V2_FULLGPIO || getBoardRevision() == BRD_V1_FULLGPIO)
    _printMenuText(0, Line1, "Analogue:", false, eRightJustify);

  switch(NVstore.getUserSettings().GPIO.in1Mode) {
    case CGPIOin1::Disabled:   
      _drawBitmap(23, 14, CrossLgIconInfo); 
      break;
    case CGPIOin1::Start:      
      _drawBitmap(23, 14, StartIconInfo); 
      break;
    case CGPIOin1::Run:        
      _drawBitmap(23, 14, RunIconInfo); 
      break;  
    case CGPIOin1::StartStop:  
      _drawBitmap(23, 14, StartIconInfo);  
      _drawBitmap(30, 14, StopIconInfo); 
      break;
    case CGPIOin1::Stop:      
      _drawBitmap(23, 14, StopIconInfo); 
      break;
  }
#if USE_JTAG == 0
  _drawBitmap(40, 16, GPIOin.getState(0) ? CloseIconInfo : OpenIconInfo);
#endif

  switch(NVstore.getUserSettings().GPIO.in2Mode) {
    case CGPIOin2::Disabled:   
      _drawBitmap(23, 27, CrossLgIconInfo); 
      break;
    case CGPIOin2::Stop:       
      _drawBitmap(23, 27, StopIconInfo); 
      break;
    case CGPIOin2::Thermostat: 
      _printMenuText(23, 27, "\352T"); 
      break;
  }
#if USE_JTAG == 0
  _drawBitmap(40, 28, GPIOin.getState(1) ? CloseIconInfo : OpenIconInfo);
#endif

  int bulbmode = 0;
#if USE_JTAG == 0
  bulbmode = GPIOout.getState(0);
#endif
  static bool iconstate = false;
  switch(NVstore.getUserSettings().GPIO.out1Mode) {
    case CGPIOout1::Disabled: 
      _drawBitmap(99, 14, CrossLgIconInfo); 
      break;
    case CGPIOout1::Status:   
      _drawBitmap(99, 14, InfoIconInfo);  
      if(iconstate && bulbmode == 2)     // animate bulb icon when status is PWM mode
        _drawBitmap(110, 13, BulbOn2IconInfo); 
      else
        _drawBitmap(110, 13, bulbmode ? BulbOnIconInfo : BulbOffIconInfo); 
      iconstate = !iconstate;
      break;
    case CGPIOout1::User:     
      _drawBitmap(99, 15, UserIconInfo);  
      _drawBitmap(110, 13, bulbmode ? BulbOnIconInfo : BulbOffIconInfo); 
      break;
    case CGPIOout1::Thresh:     
      _drawBitmap(99, 15, threshIconInfo);  
      _drawBitmap(110, 13, bulbmode ? BulbOnIconInfo : BulbOffIconInfo); 
      break;
  }

#if USE_JTAG == 0
  bulbmode = GPIOout.getState(1);
#endif
  switch(NVstore.getUserSettings().GPIO.out2Mode) {
    case CGPIOout2::Disabled: _drawBitmap(99, 27, CrossLgIconInfo); break;
    case CGPIOout2::User:     
      _drawBitmap(99, 27, UserIconInfo);  
      _drawBitmap(110, 26, bulbmode ? BulbOnIconInfo : BulbOffIconInfo); 
      break;
    case CGPIOout2::Thresh:     
      _drawBitmap(99, 27, threshIconInfo);  
      _drawBitmap(110, 26, bulbmode ? BulbOnIconInfo : BulbOffIconInfo); 
      break;
  }


  if(getBoardRevision() == BRD_V2_FULLGPIO || getBoardRevision() == BRD_V1_FULLGPIO) {
    _drawBitmap(0, Line1-1, algIconInfo);  
    if(NVstore.getUserSettings().GPIO.algMode == CGPIOalg::Disabled) {
      _drawBitmap(23, Line1, CrossLgIconInfo);
    }
    else {
#if USE_JTAG == 0
      sprintf(msg, "%d%%", GPIOalg.getValue() * 100 / 4096);
      _printMenuText(23, Line1, msg);
#endif      
    }
  }

  _printMenuText(_display.xCentre(), 53, " \021                \020 ", true, eCentreJustify);
  return true;
}

bool 
CGPIOInfoScreen::show()
{
  return false;//  CScreenHeader::show(false);
}


bool 
CGPIOInfoScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    _keyRepeatCount = 0;     // unlock tracking of repeat events
    // UP press
    if(event & key_Up) {
    }
    // CENTRE press
    if(event & key_Centre) {
    }
    if(event & key_Down) {
      _ScreenManager.selectMenu(CScreenManager::UserSettingsLoop, CScreenManager::GPIOUI);
    }
  }
  if(event & keyRepeat) {
    if(_keyRepeatCount >= 0) {
      _keyRepeatCount++;
      // hold LEFT to toggle GPIO output #1
      if(event & key_Left) {
        if(_keyRepeatCount > 2) {
          _keyRepeatCount = -1;     // prevent double handling
          toggleGPIOout(0);     // toggle GPIO output #1
        }
      }
      // hold RIGHT to toggle GPIO output #2
      if(event & key_Right) {
        if(_keyRepeatCount > 2) {
          _keyRepeatCount = -1;     // prevent double handling
          toggleGPIOout(1);     // toggle GPIO output #2
        }
      }
    }
  }

  // release event
  if(event & keyReleased) {
    if(_keyRepeatCount == 0) {  // short Up press - lower target
      // press LEFT to select previous screen
      if(event & key_Left) {
        _ScreenManager.prevMenu();
      }
      // press RIGHT to select next screen
      if(event & key_Right) {
        _ScreenManager.nextMenu();
      }
    }
    _keyRepeatCount = -1;
  }
  _ScreenManager.reqUpdate();

  return true;
}

