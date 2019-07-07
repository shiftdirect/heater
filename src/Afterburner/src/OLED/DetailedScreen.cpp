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
#include "fonts/MiniFont.h"
#include "fonts/Icons.h"
#include "DetailedScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "../Protocol/Protocol.h"
#include "../Utility/NVStorage.h"


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
// CDetailedScreen
//
// This screen provides a detailed control/status function
//
///////////////////////////////////////////////////////////////////////////

CDetailedScreen::CDetailedScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreenHeader(display, mgr) 
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


bool 
CDetailedScreen::show()
{
  CScreenHeader::show();

  int runstate = getHeaterInfo().getRunStateEx();
  int errstate = getHeaterInfo().getErrState(); 
  if(errstate) errstate--;  // correct for +1 biased return value
  
  long tDelta = millis() - _showTarget;
  if(_showTarget && (tDelta > 0)) {
    _showTarget = 0;
  }

  float desiredT = 0;
  float fPump = 0;
  if((runstate && (runstate <= 5)) || (runstate == 9) || _showTarget) {  // state 9 = manufactured "heating glow plug"
    if(getThermostatModeActive())
      desiredT = getTemperatureDesired();
    else {
      fPump = getHeaterInfo().getPump_Fixed();
      if(NVstore.getUserSettings().cyclic.isEnabled())
        desiredT = getDemandDegC();
    }
  }

  float fTemp = getTemperatureSensor();
  showThermometer(desiredT,    // read values from most recently sent [BTC] frame
                  fTemp,
                  fPump);

  _animateRPM = false;
  _animatePump = false;
  _animateGlow = false;

  if(runstate != 0 && runstate != 10) {  // not idle modes
    float power = getHeaterInfo().getGlowPlug_Power();
    if(power > 1) {
      showGlowPlug(power);
    }

    if(_showTarget)
      showFanV(getHeaterInfo().getFan_Voltage());
    else
      showFan(getHeaterInfo().getFan_Actual());

    showFuel(getHeaterInfo().getPump_Actual());

    showBodyThermometer(getHeaterInfo().getTemperature_HeatExchg());
  }

  showRunState(runstate, errstate);
  return true;
}


bool 
CDetailedScreen::animate()
{
  bool retval = CScreenHeader::animate();

  if(_animatePump || _animateRPM || _animateGlow) {

    if(_animatePump) {
      // erase region of fuel icon
      _display.fillRect(X_FUEL_ICON, Y_FUEL_ICON, FuelIconInfo.width, FuelIconInfo.height + 4, BLACK);
      _drawBitmap(X_FUEL_ICON, Y_FUEL_ICON+(_dripAnimationState/2), FuelIconInfo);
      _dripAnimationState++;
      _dripAnimationState &= 0x07;
    }

    if(_animateRPM) {
      // erase region of fuel icon
      _display.fillRect(X_FAN_ICON, Y_FAN_ICON, FanIcon1Info.width, FanIcon1Info.height, BLACK);
      switch(_fanAnimationState) {
        case 0: _drawBitmap(X_FAN_ICON, Y_FAN_ICON, FanIcon1Info); break;
        case 1: _drawBitmap(X_FAN_ICON, Y_FAN_ICON, FanIcon2Info); break;
        case 2: _drawBitmap(X_FAN_ICON, Y_FAN_ICON, FanIcon3Info); break;
        case 3: _drawBitmap(X_FAN_ICON, Y_FAN_ICON, FanIcon4Info); break;
      }
      _fanAnimationState++;
      _fanAnimationState &= 0x03;
    }
    
    if(_animateGlow) {
      _display.fillRect(X_GLOW_ICON, Y_GLOW_ICON, 17, 10, BLACK);
      _drawBitmap(X_GLOW_ICON, Y_GLOW_ICON, GlowPlugIconInfo);
      _drawBitmap(X_GLOW_ICON, Y_GLOW_ICON + 2 + _heatAnimationState, GlowHeatIconInfo);
      _heatAnimationState -= 2;
      _heatAnimationState &= 0x07;
    }

    retval = true;
  }
  retval |= CScreen::animate();
  return retval;
}


bool 
CDetailedScreen::keyHandler(uint8_t event)
{
  
  if(event & keyPressed) {
    _keyRepeatCount = 0;     // unlock tracking of repeat events
  }
  // require hold to turn ON or OFF
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

      if(event & key_Centre) {
        int runstate = getHeaterInfo().getRunStateEx();
        if(runstate) {   // running, including cyclic mode idle
          if(_keyRepeatCount > 5) {
            _keyRepeatCount = -1;        // prevent double handling
            requestOff();
          }
        }
        else {
          if(_keyRepeatCount > 3) {
            _keyRepeatCount = -1;   // prevent double handling
            requestOn();
          }
        }
      }
      if(event & key_Down) {
        if(_keyRepeatCount > 1) {    // held Down - togle thermo/fixed mode
          _keyRepeatCount = -1;      // prevent double handling
          if(reqThermoToggle()) {
            _showTarget = millis() + 3500;
            NVstore.save();
          } 
          else  _reqOEMWarning();
        }
      }
      if(event & key_Up) {
        if(_keyRepeatCount > 1) {    // held Down - togle thermo/fixed mode
          _keyRepeatCount = -1;      // prevent double handling
          sUserSettings settings = NVstore.getUserSettings();
          toggle(settings.degF);
          NVstore.setUserSettings(settings);
          NVstore.save();
        }
      }
    }
  }
  // release event
  if(event & keyReleased) {
    if(_keyRepeatCount == 0) {  // short Up press - lower target
      if(event & key_Up) {
        if(reqTempDelta(+1))  _showTarget = millis() + 3500;
        else  _reqOEMWarning();
      }
      if(event & key_Down) {   // short Down press - lower target
        if(reqTempDelta(-1))  _showTarget = millis() + 3500;
        else  _reqOEMWarning();
      }
      if(event & key_Centre) {  // short Centre press - show target
        _showTarget = millis() + 3500;
      }
      _ScreenManager.reqUpdate();
      if(event & key_Left) {
        _ScreenManager.prevMenu();
      }
      if(event & key_Right) {
        _ScreenManager.nextMenu();
      }
    }
    _keyRepeatCount = -1;
  }

  return true;
}


#define TEMP_YPOS(A) ((20 - int(A)) + 27)  // 26 is location of 20deg tick
void 
CDetailedScreen::showThermometer(float fDesired, float fActual, float fPump) 
{
  char msg[16];
  // draw bulb design
  _drawBitmap(X_BULB, Y_BULB, AmbientThermometerIconInfo, WHITE);

  if(fActual > 0) { 
    // draw mercury
    int yPos = Y_BULB + TEMP_YPOS(fActual);
    _display.drawLine(X_BULB + 3, yPos, X_BULB + 3, Y_BULB + 42, WHITE);
    _display.drawLine(X_BULB + 4, yPos, X_BULB + 4, Y_BULB + 42, WHITE);  
  }
  // print actual temperature
  if(fActual > -80) {
    CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
    if(NVstore.getUserSettings().degF) {
      fActual = fActual * 9 / 5 + 32;
      sprintf(msg, "%.1f`F", fActual);
    }
    else {
      sprintf(msg, "%.1f`C", fActual);
    }
    _printMenuText(0, Y_BASELINE, msg);
  }
  else {
    _printInverted(1, Y_BASELINE-2, "N/A", true);
  }

  // draw cyclic bracket (if enabled)
  if(NVstore.getUserSettings().cyclic.isEnabled() && (fDesired != 0)) {
    int max = fDesired + NVstore.getUserSettings().cyclic.Stop + 1;
    int min = fDesired + NVstore.getUserSettings().cyclic.Start;  // stored as a negative value!

    // convert to screen coordinates
    max = Y_BULB + TEMP_YPOS(max);   
    min = Y_BULB + TEMP_YPOS(min);   

    int xOfs = 8;                                                            
                                                                             //     #
    _drawBitmap(X_BULB + xOfs, max-2, ThermoPtrHighIconInfo);                //    ##
                                                                             //  ####

                                                                             //  ####
    _drawBitmap(X_BULB + xOfs, min, ThermoPtrLowIconInfo);                   //    ##
                                                                             //     #
  }

  // draw target setting
  // may be suppressed if not in normal start or run state
  if((fDesired != 0) || (fPump != 0)) {
    _drawBitmap(X_TARGET_ICON, Y_TARGET_ICON, TargetIconInfo);   // draw target icon
    char msg[16];
    if(fPump == 0) {
      int yPos = Y_BULB + TEMP_YPOS(fDesired) - 2;      // 2 offsets mid height of icon
      _drawBitmap(X_BULB-1, yPos, ThermoPtrIconInfo);   // set closed indicator against bulb
      if(NVstore.getUserSettings().degF) {
        fDesired = fDesired * 9 / 5 + 32;
        sprintf(msg, "%.0f`F", fDesired);
      }
      else {
        sprintf(msg, "%.0f`C", fDesired);
      }
    }
    else {
      if(fDesired) {
        int yPos = Y_BULB + TEMP_YPOS(fDesired) - 2;
        _drawBitmap(X_BULB-1, yPos, ThermoOpenPtrIconInfo);   // set open style indicator against bulb
      }
      sprintf(msg, "%.1fHz", fPump);
    }
#ifdef MINI_TARGETLABEL
    CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
#endif
    _printMenuText(X_TARGET_ICON + (TargetIconInfo.width/2), Y_BASELINE, msg, false, eCentreJustify);
  }
}

#define BODY_YPOS(A) ((((100 - A) * 3) / 16) + 22)   // 100degC centre - ticks +- 80C
void 
CDetailedScreen::showBodyThermometer(int actual) 
{
  // draw bulb design
  _drawBitmap(X_BODY_BULB, Y_BULB, BodyThermometerIconInfo);
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
  if(NVstore.getUserSettings().degF) {
    actual = actual * 9 / 5 + 32;
    sprintf(label, "%d`F", actual);
  }
  else {
    sprintf(label, "%d`C", actual);
  }
#else
  sprintf(label, "%d", actual);
#endif
  _printMenuText(_display.width(), Y_BASELINE, label, false, eRightJustify);
}


void 
CDetailedScreen::showGlowPlug(float power)
{
  _drawBitmap(X_GLOW_ICON, Y_GLOW_ICON, GlowPlugIconInfo);
//  _animateGlow = true;
  char msg[16];
  sprintf(msg, "%.0fW", power);
#ifdef MINI_GLOWLABEL  
  CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
#endif
  _printMenuText(X_GLOW_ICON + (GlowPlugIconInfo.width/2), 
                Y_GLOW_ICON + GlowPlugIconInfo.height + 3,
                msg, false, eCentreJustify);
}

void 
CDetailedScreen::showFan(int RPM)
{
  // NOTE: fan rotation animation performed in animateOLED
  _animateRPM = RPM != 0;   // used by animation routine

  _display.setTextColor(WHITE);
  char msg[16];
  sprintf(msg, "%d", RPM);
#ifdef MINI_FANLABEL  
  CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
#endif
  _printMenuText(X_FAN_ICON + (FanIcon1Info.width/2), Y_BASELINE, msg, false, eCentreJustify);
}

void 
CDetailedScreen::showFanV(float volts)
{
  // NOTE: fan rotation animation performed in animateOLED
  _animateRPM = volts != 0;   // used by animation routine

  _display.setTextColor(WHITE);
  char msg[16];
  sprintf(msg, "%.1fV", volts);
#ifdef MINI_FANLABEL  
  CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
#endif
  _printMenuText(X_FAN_ICON + (FanIcon1Info.width/2), Y_BASELINE, msg, false, eCentreJustify);
}

void 
CDetailedScreen::showFuel(float rate)
{
  // NOTE: fuel drop animation performed in animateOLED
  _animatePump = rate != 0;    // used by animation routine
  if(rate) {
    char msg[16];
    sprintf(msg, "%.1f", rate);
#ifdef MINI_FUELLABEL
    CTransientFont AF(_display, &MINIFONT);  // temporarily use a mini font
#endif
    _printMenuText(X_FUEL_ICON + (FuelIconInfo.width/2), Y_BASELINE, msg, false, eCentreJustify);
  }
}

void 
CDetailedScreen::showRunState(int runstate, int errstate) 
{
  static bool toggle = false;
  const char* toPrint = NULL;
  int yPos = 25;
  _display.setTextColor(WHITE, BLACK);
  if(errstate && ((runstate == 0) || (runstate > 5))) {
    // an error is present in idle or states beyond running, show it
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
    // no errors, heater normal
    toPrint = getHeaterInfo().getRunStateStr();
  }
  if(toPrint) {
    _printMenuText(_display.xCentre(), yPos, toPrint, false, eCentreJustify);
  }
}



