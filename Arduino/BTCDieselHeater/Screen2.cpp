#include "128x64OLED.h"
#include "display.h"
#include "MiniFont.h"
#include "tahoma16.h"
#include "OLEDconsts.h"
#include "BluetoothAbstract.h" 
#include "Screen2.h"
#include "BTCWifi.h"
#include "KeyPad.h"
#include "helpers.h"
#include "Protocol.h"
#include "UtilClasses.h"


#define MAXIFONT tahoma_16ptFontInfo
#define MINIFONT miniFontInfo

unsigned long showSetMode = 0;
unsigned long showMode = 0;
unsigned char nModeSel;

void showRunState(C128x64_OLED& display);

void showScreen2(C128x64_OLED& display, const CProtocol& CtlFrame, const CProtocol& HtrFrame)
{
  char msg[20];
  CRect textRect;

  sprintf(msg, "%.1f`", fFilteredTemperature);
  display.setFontInfo(&MAXIFONT);  // Dot Factory Font
  display.getTextExtents(msg, textRect);
  textRect.xPos = (display.width()- textRect.width) / 2;
  textRect.yPos = 25;
  display.fillRect(textRect.xPos, textRect.yPos, textRect.width, textRect.height, BLACK);
  display.setCursor(textRect.xPos, textRect.yPos);
  display.print(msg);
  display.setFontInfo(NULL);  // standard 5x7 font


  // at bottom of screen show either:
  //   Selection between Fixed or Thermostat mode
  //   Current heat demand setting
  //   Run state of heater
  
  if(showMode) {
    const int border = 3;
    const int radius = 4;
    // Show selection between Fixed or Thermostat mode
    long tDelta = millis() - showMode;
    if(tDelta < 0) {
      // display "Fixed Hz" at lower left, allowing space for a selection surrounding box
      strcpy(msg, "Fixed Hz");
      display.getTextExtents(msg, textRect);  // size of text to print
      textRect.xPos = display.width()- textRect.width - border;     // set X position to finish short of RHS
      textRect.yPos = display.height() - textRect.height - border;  // bottom of screen, with room for box
      display.setCursor(textRect.xPos,            // centre text in potential box
                        textRect.yPos);     
      display.print(msg);                         // show the text
      if(nModeSel == 1) {                         // add selection box if current selection
        textRect.Expand(border);                  // expand about text position
        display.drawRoundRect(textRect.xPos, textRect.yPos, textRect.width, textRect.height, radius, WHITE);
      }
      // display "Thermostat" at lower right, allowing space for a selection surrounding box
      strcpy(msg, "Thermostat");
      display.getTextExtents(msg, textRect);
      textRect.xPos = border;
      textRect.yPos = display.height() - textRect.height - border;  // bottom of screen, with room for box
      display.setCursor(textRect.xPos,            // centre text in potential box
                        textRect.yPos);
      display.print(msg);                         // show the text
      if(nModeSel == 0) {                         // add selection box if current selection
        textRect.Expand(border);                  // expand about text position
        display.drawRoundRect(textRect.xPos, textRect.yPos, textRect.width, textRect.height, radius, WHITE);
      }
      setThermostatMode(nModeSel == 0 ? 1 : 0);    // set the new mode
    }
    else {
      // cancel selection mode, apply whatever is boxed
      showMode = 0;
      showSetMode = millis() + 5000;  // then make the new mode setting be shown
    }
  }
  if((showMode == 0) && showSetMode) {
    long tDelta = millis() - showSetMode;  
    if(tDelta < 0) {
      // Show current heat demand setting
      if(getThermostatMode()) {
        sprintf(msg, "Setpoint = %d`C", getSetTemp());
      }
      else {
        sprintf(msg, "Setpoint = %.1fHz", getFixedHz());
      }
      // centre message at bottom of screen
      display.getTextExtents(msg, textRect);
      display.setCursor(display.xCentre(), 
                        display.height() - textRect.height);
      display.printCentreJustified(msg);
    }
    else {
      showSetMode = 0;
    }
  }
  if((showMode == 0) && (showSetMode == 0)) {
    showRunState(display);
  }
}

void animateScreen2(C128x64_OLED& display)
{
  display.display();
}

void keyhandlerScreen2(uint8_t event)
{
  static int repeatCount = -1;
  if(event & keyPressed) {
    repeatCount = 0;     // unlock tracking of repeat events
    // press LEFT to select previous screen, or Fixed Hz mode when in mode select
    if(event & key_Left) {
      if(!showMode)
        prevScreen();
      else {
        showMode = millis() + 5000;
        nModeSel = 0;
        reqDisplayUpdate();
      }
    }
    // press RIGHT to selecxt next screen, or Thermostat mode when in mode select
    if(event & key_Right) {
      if(!showMode)
        nextScreen();
      else {
        showMode = millis() + 5000;
        nModeSel = 1;
        reqDisplayUpdate();
      }
    }
    // press UP & DOWN to toggle thermostat / fixed Hz mode
    // impossible with 5 way switch!
    uint8_t doubleKey = key_Down | key_Up;
    if((event & doubleKey) == doubleKey) {
      reqThermoToggle();
      showSetMode = millis() + 2000;
    }
  }
  // use repeat function for key hold detection
  if(event & keyRepeat) {
    if(repeatCount >= 0) {
      repeatCount++;
      // hold DOWN to enter thermostat / fixed mode selection
      if(event & key_Down) {
        if(repeatCount > 2) {
          repeatCount = -1;        // prevent double handling
          showMode = millis() + 5000;
          nModeSel = getThermostatMode() ? 0 : 1;
        }
      }
      // hold CENTRE to turn ON or OFF
      if(event & key_Centre) {
        if(getRunState()) {
          // running, request OFF
          if(repeatCount > 5) {
            repeatCount = -2;        // prevent double handling
            requestOff();
          }
        }
        else {
          // standby, request ON
          if(repeatCount > 3) {
            repeatCount = -1;
            requestOn();
          }
        }
      }
    }
  }
  if(event & keyReleased) {
    if(!showMode) {
      // release DOWN key to reduce set demand, provided we are not in mode select
      if(event & key_Down) {
        reqTempChange(-1);
        showSetMode = millis() + 2000;
      }
      // release UP key to increase set demand, provided we are not in mode select
      if(event & key_Up) {
        reqTempChange(+1);
        showSetMode = millis() + 2000;
      }
    }
    // release CENTRE to accept new mode, and/or show current setting
    if(event & key_Centre) {
      if(repeatCount != -2) {  // prevent after off commands
        if(showMode) {
          showMode = millis(); // force immediate cancellation of showmode (via screen update)
        }
        showSetMode = millis() + 2000;
      }
      reqDisplayUpdate();
    }

    repeatCount = -1;
  }
}

void showRunState(C128x64_OLED& display)
{
  int runstate = getRunState(); 
  int errstate = getErrState(); 

  if(errstate) errstate--;  // correct for +1 biased return value

  static bool toggle = false;
  const char* toPrint = NULL;
  display.setTextColor(WHITE, BLACK);
  if(runstate >= 0 && runstate <= 8) {
    if(((runstate == 0) || (runstate > 5)) && errstate) {
      // create an "E-XX" message to display
      char msg[16];
      sprintf(msg, "E-%02d", errstate);
      // determine height of font
      CRect textRect;
      display.getTextExtents(msg, textRect);
      display.setCursor(display.xCentre(),    // locate at bottom centre, 1 line up
                        display.height() - 2*textRect.height);
      // flash error code
      toggle = !toggle;
      if(toggle)
        display.printCentreJustified(msg);
      else {
        display.printCentreJustified("          ");
      }
      // bounds limit error and gather message
      if(errstate > 10) errstate = 11;
      toPrint = Errstates[errstate-1];
    }
    else {
      if(runstate) {
        if(runstate < 5)        toPrint = "Starting";
        else if(runstate == 5)  toPrint = "Running";
        else if(runstate == 8)  toPrint = "Cooling";
        else                    toPrint = "Shutting down";
      }
    }
  }
  if(toPrint) {
    // determine height of font
    CRect textRect;
    display.getTextExtents(toPrint, textRect);
    display.setCursor(display.xCentre(),                   // locate at bottom centre
                      display.height() - textRect.height);
    display.printCentreJustified(toPrint);
  }
}