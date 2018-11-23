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


#define MAXIFONT tahoma_16ptFontInfo
#define MINIFONT miniFontInfo

unsigned long showSetTemp = 0;
unsigned long showMode = 0;
unsigned char nModeSel;

void showScreen2(C128x64_OLED& display, const CProtocol& CtlFrame, const CProtocol& HtrFrame)
{
  char msg[20];

  sprintf(msg, "%.1f`", fFilteredTemperature);
  display.setFontInfo(&MAXIFONT);  // Dot Factory Font
  uint16_t width, height;
  display.getTextExtents(msg, width, height);
  int xPos = 64 - (width/2);
  display.fillRect(xPos, 25, width, height, BLACK);
  display.setCursor(xPos, 25);
  display.print(msg);
  display.setFontInfo(NULL);  // standard 5x7 font

  if(showMode) {
    if(millis() < showMode) {
      strcpy(msg, "Fixed Hz");
      display.getTextExtents(msg, width, height);
      int yPos = 63 - height - 2;
      display.setCursor(2, yPos);
      display.print(msg);
      if(nModeSel == 0) {
        display.drawRoundRect(0, yPos-2, width+4, height+4, 3, WHITE);
      }
      strcpy(msg, "Thermostat");
      display.getTextExtents(msg, width, height);
      int xPos = 127 - width - 2;
      display.setCursor(xPos, yPos);
      display.print(msg);
      if(nModeSel == 1) {
        display.drawRoundRect(xPos - 2, yPos-2, width+4, height+4, 3, WHITE);
      }
    }
    else {
      showMode = 0;
      setThermostatMode(nModeSel);
    }
  }
  else if(millis() < showSetTemp) {
    if(getThermostatMode()) {
      sprintf(msg, "Setpoint = %d`C", getSetTemp());
    }
    else {
      sprintf(msg, "Setpoint = %.1fHz", getFixedHz());
    }
    display.getTextExtents(msg, width, height);
    xPos = 64 - (width/2);     // centre across 
    int yPos = 63 - height;    // at bottom of screen
    display.setCursor(xPos, yPos);
    display.print(msg);
  }
  else {
    int runState = getRunState();
    if(runState) {
      const char* pMsg;
      if(runState < 5) {
        pMsg = "Starting";
      }
      else if(runState == 5) {
        pMsg = "Running";
      }
      else {
        pMsg = "Shutting down";
      }

      display.getTextExtents(pMsg, width, height);
      int xPos = 64 - (width/2);   // centre across
      int yPos = 63 - height;      // at bottom of screen
      display.setCursor(xPos, yPos);
      display.print(pMsg);
    }
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
    uint8_t doubleKey = key_Down | key_Up;
    if((event & doubleKey) == doubleKey) {
      reqThermoToggle();
      showSetTemp = millis() + 2000;
    }
    // press CENTRE to accept new mode, and/or show current setting
    if(event & key_Centre) {
      if(showMode) {
        setThermostatMode(nModeSel);
      }
      showMode = 0;
      showSetTemp = millis() + 2000;
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
          nModeSel = getThermostatMode();
        }
      }
      // hold CENTRE to turn ON or OFF
      if(event & key_Centre) {
        if(getRunState()) {
          // running, request OFF
          if(repeatCount > 5) {
            repeatCount = -1;        // prevent double handling
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
        showSetTemp = millis() + 2000;
      }
      // release UP key to increase set demand, provided we are not in mode select
      if(event & key_Up) {
        reqTempChange(+1);
        showSetTemp = millis() + 2000;
      }
    }
    repeatCount = -1;
  }
}
