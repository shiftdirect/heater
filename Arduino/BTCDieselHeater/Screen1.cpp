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
#include "MiniFont.h"
#include "Icons.h"
#include "BluetoothAbstract.h" 
#include "Screen1.h"
#include "BTCWifi.h"
#include "KeyPad.h"
#include "helpers.h"
#include "Protocol.h"


#define MINIFONT miniFontInfo

#define X_FAN_ICON     55 
#define Y_FAN_ICON     39
#define X_FUEL_ICON    81 
#define Y_FUEL_ICON    39
#define X_TARGET_ICON  31
#define Y_TARGET_ICON  39
#define Y_BASELINE     58
#define X_GLOW_ICON    97
#define Y_GLOW_ICON    38
#define X_BODY_BULB   119
#define X_BULB          1  // >= 1
#define Y_BULB          4

#define MINI_TEMPLABEL
#define MINI_TARGETLABEL
#define MINI_FANLABEL
#define MINI_GLOWLABEL
#define MINI_FUELLABEL
#define MINI_BODYLABEL

///////////////////////////////////////////////////////////////////////////
//
// CScreen1
//
// This screen provides a detailed control/status function
//
///////////////////////////////////////////////////////////////////////////

CScreen1::CScreen1(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
{
  _animatePump = false;
  _animateRPM = false;
  _animateGlow = false;
  _fanAnimationState = 0;
  _dripAnimationState = 0;
  _heatAnimationState = 0;
  _keyRepeatCount = -1;
  _showTarget = 0;
}


void 
CScreen1::show()
{
  CScreenHeader::show();

  const char* c = String(getActualTemperature()).c_str();
  
  int runstate = getHeaterInfo().getRunState();//HtrFrame.getRunState(); 
  int errstate = getHeaterInfo().getErrState(); //HtrFrame.getErrState(); 
  if(errstate) errstate--;  // correct for +1 biased return value
  
  long tDelta = millis() - _showTarget;
  if(_showTarget && (tDelta > 0)) {
    _showTarget = 0;
  }

  float desiredT = 0;
  if((runstate && (runstate <= 5)) || _showTarget) {
    if(getHeaterInfo().isThermostat())
      desiredT = getHeaterInfo().getTemperature_Desired();
    else
      desiredT = -getHeaterInfo().getPump_Fixed();
  }

  showThermometer(desiredT,    // read values from most recently sent [BTC] frame
                  getActualTemperature());

  _animateRPM = false;
  _animatePump = false;
  _animateGlow = false;

  if(runstate) {
    float power = getHeaterInfo().getGlowPlug_Power();
    if(power > 1) {
      showGlowPlug(power);
    }

    showFan(getHeaterInfo().getFan_Actual());

    showFuel(getHeaterInfo().getPump_Actual());

    showBodyThermometer(getHeaterInfo().getTemperature_HeatExchg());
  }

  showRunState(runstate, errstate);
}


bool 
CScreen1::animate()
{
  bool retval = CScreenHeader::animate();

  if(_animatePump || _animateRPM || _animateGlow) {

    if(_animatePump) {
      // erase region of fuel icon
      _display.fillRect(X_FUEL_ICON, Y_FUEL_ICON, W_FUEL_ICON, H_FUEL_ICON + 4, BLACK);
      _display.drawBitmap(X_FUEL_ICON, Y_FUEL_ICON+(_dripAnimationState/2), FuelIcon, W_FUEL_ICON, H_FUEL_ICON, WHITE);
      _dripAnimationState++;
      _dripAnimationState &= 0x07;
    }

    if(_animateRPM) {
      // erase region of fuel icon
      _display.fillRect(X_FAN_ICON, Y_FAN_ICON, W_FAN_ICON, H_FAN_ICON, BLACK);
      switch(_fanAnimationState) {
        case 0: _display.drawBitmap(X_FAN_ICON, Y_FAN_ICON, FanIcon1, W_FAN_ICON, H_FAN_ICON, WHITE); break;
        case 1: _display.drawBitmap(X_FAN_ICON, Y_FAN_ICON, FanIcon2, W_FAN_ICON, H_FAN_ICON, WHITE); break;
        case 2: _display.drawBitmap(X_FAN_ICON, Y_FAN_ICON, FanIcon3, W_FAN_ICON, H_FAN_ICON, WHITE); break;
        case 3: _display.drawBitmap(X_FAN_ICON, Y_FAN_ICON, FanIcon4, W_FAN_ICON, H_FAN_ICON, WHITE); break;
      }
      _fanAnimationState++;
      _fanAnimationState &= 0x03;
    }
    
    if(_animateGlow) {
      _display.fillRect(X_GLOW_ICON, Y_GLOW_ICON, 17, 10, BLACK);
      _display.drawBitmap(X_GLOW_ICON, Y_GLOW_ICON, GlowPlugIcon, 16, 9, WHITE);
      _display.drawBitmap(X_GLOW_ICON, Y_GLOW_ICON + 2 + _heatAnimationState, GlowHeatIcon, 17, 2, WHITE);
      _heatAnimationState -= 2;
      _heatAnimationState &= 0x07;
    }
    return retval |= true;
  }
  return retval;
}


void 
CScreen1::keyHandler(uint8_t event)
{
  
  if(event & keyPressed) {
    _keyRepeatCount = 0;     // unlock tracking of repeat events
    if(event & key_Left) {
      _ScreenManager.prevScreen();
    }
    if(event & key_Right) {
      _ScreenManager.nextScreen();
    }
    if(event & key_Up) {
      reqTempChange(+1);
      _showTarget = millis() + 3500;
    }
    if(event & key_Down) {
      reqTempChange(-1);
      _showTarget = millis() + 3500;
    }
  }
  // require hold to turn ON or OFF
  if(event & keyRepeat) {
    if(_keyRepeatCount >= 0) {
      _keyRepeatCount++;
      if(event & key_Centre) {
        if(getHeaterInfo().getRunState()) {
          if(_keyRepeatCount > 5) {
            _keyRepeatCount = -1;        // prevent double handling
            requestOff();
          }
        }
        else {
          if(_keyRepeatCount > 3) {
            _keyRepeatCount = -1;
            requestOn();
          }
        }
      }
    }
  }
  // release event
  if(event & keyReleased) {
    _keyRepeatCount = -1;
  }
}


#define TEMP_YPOS(A) ((20 - int(A)) + 27)  // 26 is location of 20deg tick
void 
CScreen1::showThermometer(float desired, float actual) 
{
  char msg[16];
  // draw bulb design
  _display.drawBitmap(X_BULB, Y_BULB, ambientThermometerIcon, W_BULB_ICON, H_BULB_ICON, WHITE);
  // draw mercury
  int yPos = Y_BULB + TEMP_YPOS(actual);
  _display.drawLine(X_BULB + 3, yPos, X_BULB + 3, Y_BULB + 42, WHITE);
  _display.drawLine(X_BULB + 4, yPos, X_BULB + 4, Y_BULB + 42, WHITE);  
  // print actual temperature
  {
#ifdef MINI_TEMPLABEL  
    CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
    sprintf(msg, "%.1f`C", actual);
#else
    sprintf(msg, "%.1f", actual);
#endif
    _printMenuText(0, Y_BASELINE, msg);
  }

  // draw target setting
  if(desired) {
    _display.drawBitmap(X_TARGET_ICON, Y_TARGET_ICON, TargetIcon, W_TARGET_ICON, H_TARGET_ICON, WHITE);   // set indicator against bulb
    char msg[16];
    if(desired > 0) {
      int yPos = Y_BULB + TEMP_YPOS(desired) - 2;
      _display.drawBitmap(X_BULB-1, yPos, thermoPtr, 3, 5, WHITE);   // set indicator against bulb
      sprintf(msg, "%.0f`C", desired);
    }
    else {
      sprintf(msg, "%.1fHz", -desired);
    }
#ifdef MINI_TARGETLABEL
    CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
#endif
    _printMenuText(X_TARGET_ICON + (W_TARGET_ICON/2), Y_BASELINE, msg, false, eCentreJustify);
  }
}

#define BODY_YPOS(A) ((((100 - A) * 3) / 16) + 22)   // 100degC centre - ticks +- 80C
void 
CScreen1::showBodyThermometer(int actual) 
{
  // draw bulb design
  _display.drawBitmap(X_BODY_BULB, Y_BULB, bodyThermometerIcon, 8, 50, WHITE);
  // draw mercury
  int yPos = Y_BULB + BODY_YPOS(actual);
  _display.drawLine(X_BODY_BULB + 3, yPos, X_BODY_BULB + 3, Y_BULB + 42, WHITE);
  _display.drawLine(X_BODY_BULB + 4, yPos, X_BODY_BULB + 4, Y_BULB + 42, WHITE);
  // print actual temperature
  _display.setTextColor(WHITE);
  char label[16];
  // determine width and position right justified
#ifdef MINI_BODYLABEL
  CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
  sprintf(label, "%d`C", actual);
#else
  sprintf(label, "%d", actual);
#endif
  _printMenuText(_display.width(), Y_BASELINE, label, false, eRightJustify);
}


void 
CScreen1::showGlowPlug(float power)
{
  _display.drawBitmap(X_GLOW_ICON, Y_GLOW_ICON, GlowPlugIcon, W_GLOW_ICON, H_GLOW_ICON, WHITE);
//  _animateGlow = true;
  char msg[16];
  sprintf(msg, "%.0fW", power);
#ifdef MINI_GLOWLABEL  
  CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
#endif
  _printMenuText(X_GLOW_ICON + (W_GLOW_ICON/2), 
                Y_GLOW_ICON + H_GLOW_ICON + 3,
                msg, false, eCentreJustify);
}

void 
CScreen1::showFan(int RPM)
{
  // NOTE: fan rotation animation performed in animateOLED
  _animateRPM = RPM != 0;   // used by animation routine

  _display.setTextColor(WHITE);
  char msg[16];
  sprintf(msg, "%d", RPM);
#ifdef MINI_FANLABEL  
  CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
#endif
  _printMenuText(X_FAN_ICON + (W_FAN_ICON/2), Y_BASELINE, msg, false, eCentreJustify);
}

void 
CScreen1::showFuel(float rate)
{
  // NOTE: fuel drop animation performed in animateOLED
  _animatePump = rate != 0;    // used by animation routine
  if(rate) {
    char msg[16];
    sprintf(msg, "%.1f", rate);
#ifdef MINI_FUELLABEL
    CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
#endif
    _printMenuText(X_FUEL_ICON + (W_FUEL_ICON/2), Y_BASELINE, msg, false, eCentreJustify);
  }
}

void 
CScreen1::showRunState(int runstate, int errstate) 
{
  static bool toggle = false;
  const char* toPrint = NULL;
  int yPos = 25;
  _display.setTextColor(WHITE, BLACK);
  if(runstate >= 0 && runstate <= 8) {
    if(((runstate == 0) || (runstate > 5)) && errstate) {
      // create an "E-XX" message to display
      char msg[16];
      sprintf(msg, "E-%02d", errstate);
      if(runstate > 5)
        yPos -= _display.textHeight();
      _display.setCursor(_display.xCentre(), yPos);
      // flash error code
      toggle = !toggle;
      if(toggle)
        _display.printCentreJustified(msg);
      else {
        _display.printCentreJustified("          ");
      }
      yPos += _display.textHeight();
      toPrint = getHeaterInfo().getErrStateStr();
    }
    else {
      toPrint = getHeaterInfo().getRunStateStr();
    }
  }
  if(toPrint) {
    _printMenuText(_display.xCentre(), yPos, toPrint, false, eCentreJustify);
  }
}



