#include "128x64OLED.h"
#include "MiniFont.h"
#include "display.h"
#include "tahoma16.h"
#include "OLEDconsts.h"
#include "BluetoothAbstract.h" 
#include "Screen1.h"
#include "BTCWifi.h"
#include "KeyPad.h"
#include "helpers.h"
#include "Protocol.h"

bool animatePump = false;
bool animateRPM = false;
bool animateGlow = false;
unsigned long showTarget = 0;

void showThermometer(C128x64_OLED& display, float desired, float actual);
void showBodyThermometer(C128x64_OLED& display, int actual);
void showGlowPlug(C128x64_OLED& display, int power);
void showFan(C128x64_OLED& display, int RPM);
void showFuel(C128x64_OLED& display, float rate);
void showRunState(C128x64_OLED& display, int state, int errstate);

#define MAXIFONT tahoma_16ptFontInfo
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

#define MINI_BATTLABEL
#define MINI_TEMPLABEL
#define MINI_TARGETLABEL
#define MINI_FANLABEL
#define MINI_GLOWLABEL
#define MINI_FUELLABEL
#define MINI_BODYLABEL


void showScreen1(C128x64_OLED& display, const CProtocol& CtlFrame, const CProtocol& HtrFrame)
{
  int runstate = HtrFrame.getRunState(); 
  int errstate = HtrFrame.getErrState(); 
  if(errstate) errstate--;  // correct for +1 biased return value
  
  long tDelta = millis() - showTarget;
  if(showTarget && (tDelta > 0)) {
    showTarget = 0;
  }

  float desiredT = 0;
  if((runstate && (runstate <= 5)) || showTarget) {
    if(CtlFrame.isThermostat())
      desiredT = CtlFrame.getTemperature_Desired();
    else
      desiredT = -HtrFrame.getPump_Fixed() * 0.1f;
  }

  showThermometer(display,
                  desiredT,    // read values from most recently sent [BTC] frame
                  fFilteredTemperature);

  animateRPM = false;
  animatePump = false;
  animateGlow = false;

  if(runstate) {
    float power = HtrFrame.getGlowPlug_Current() * 0.01 * HtrFrame.getGlowPlug_Voltage() * 0.1;
    if(power > 1) {
      showGlowPlug(display, int(power));
    }

    showFan(display, HtrFrame.getFan_Actual());

    showFuel(display, HtrFrame.getPump_Actual() * 0.1f);

    showBodyThermometer(display, HtrFrame.getTemperature_HeatExchg());
  }
/*  else {
    if(isWifiConnected()) {
      display.setCursor(display.width(), 57);   // xPos irrelevant here - setting Y
      display.printRightJustified(getWifiAddrStr());
    }
  }*/

  showRunState(display, runstate, errstate);

}


void animateScreen1(C128x64_OLED& display)
{
  static int fan = 0;
  static int drip = 0;
  static int heat = 0;

  if(animatePump || animateRPM || animateGlow) {

    if(animatePump) {
      // erase region of fuel icon
      display.fillRect(X_FUEL_ICON, Y_FUEL_ICON, W_FUEL_ICON, H_FUEL_ICON + 4, BLACK);
      display.drawBitmap(X_FUEL_ICON, Y_FUEL_ICON+(drip/2), FuelIcon, W_FUEL_ICON, H_FUEL_ICON, WHITE);
      drip++;
      drip &= 0x07;
    }

    if(animateRPM) {
      // erase region of fuel icon
      display.fillRect(X_FAN_ICON, Y_FAN_ICON, W_FAN_ICON, H_FAN_ICON, BLACK);
      switch(fan) {
        case 0: display.drawBitmap(X_FAN_ICON, Y_FAN_ICON, FanIcon1, W_FAN_ICON, H_FAN_ICON, WHITE); break;
        case 1: display.drawBitmap(X_FAN_ICON, Y_FAN_ICON, FanIcon2, W_FAN_ICON, H_FAN_ICON, WHITE); break;
        case 2: display.drawBitmap(X_FAN_ICON, Y_FAN_ICON, FanIcon3, W_FAN_ICON, H_FAN_ICON, WHITE); break;
        case 3: display.drawBitmap(X_FAN_ICON, Y_FAN_ICON, FanIcon4, W_FAN_ICON, H_FAN_ICON, WHITE); break;
      }
      fan++;
      fan &= 0x03;
    }
    
    if(animateGlow) {
      display.fillRect(X_GLOW_ICON, Y_GLOW_ICON, 17, 10, BLACK);
      display.drawBitmap(X_GLOW_ICON, Y_GLOW_ICON, GlowPlugIcon, 16, 9, WHITE);
      display.drawBitmap(X_GLOW_ICON, Y_GLOW_ICON + 2 + heat, GlowHeatIcon, 17, 2, WHITE);
      heat -= 2;
      heat &= 0x07;
    }
  }
  display.display();
}


void keyhandlerScreen1(uint8_t event)
{
  static int repeatCount = -1;
  if(event & keyPressed) {
    repeatCount = 0;     // unlock tracking of repeat events
    if(event & key_Left) {
      prevScreen();
    }
    if(event & key_Right) {
      nextScreen();
    }
    if(event & key_Up) {
      reqTempChange(+1);
      showTarget = millis() + 3500;
    }
    if(event & key_Down) {
      reqTempChange(-1);
      showTarget = millis() + 3500;
    }
  }
  // require hold to turn ON or OFF
  if(event & keyRepeat) {
    if(repeatCount >= 0) {
      repeatCount++;
      if(event & key_Centre) {
        if(getRunState()) {
          if(repeatCount > 5) {
            repeatCount = -1;        // prevent double handling
            requestOff();
          }
        }
        else {
          if(repeatCount > 3) {
            repeatCount = -1;
            requestOn();
          }
        }
      }
    }
  }
  if(event & keyReleased) {
    repeatCount = -1;
  }
}


#define TEMP_YPOS(A) ((((20 - A) * 3) / 2) + 22)
void showThermometer(C128x64_OLED& display, float desired, float actual) 
{
  char msg[16];
  // draw bulb design
  display.drawBitmap(X_BULB, Y_BULB, thermometerBitmap, W_BULB_ICON, H_BULB_ICON, WHITE);
  // draw mercury
  int yPos = Y_BULB + TEMP_YPOS(actual);
  display.drawLine(X_BULB + 3, yPos, X_BULB + 3, Y_BULB + 42, WHITE);
  display.drawLine(X_BULB + 4, yPos, X_BULB + 4, Y_BULB + 42, WHITE);  
  // print actual temperature
#ifdef MINI_TEMPLABEL  
  sprintf(msg, "%.1f`C", actual);
  display.setFontInfo(&MINIFONT);  // select Mini Font
#else
  sprintf(msg, "%.1f", actual);
#endif
  display.setCursor(0, Y_BASELINE);
  display.print(msg);
  display.setFontInfo(NULL);  

  // draw target setting
  if(desired) {
    display.drawBitmap(X_TARGET_ICON, Y_TARGET_ICON, TargetIcon, W_TARGET_ICON, H_TARGET_ICON, WHITE);   // set indicator against bulb
    char msg[16];
    if(desired > 0) {
      int yPos = Y_BULB + TEMP_YPOS(desired) - 2;
      display.drawBitmap(X_BULB-1, yPos, thermoPtr, 3, 5, WHITE);   // set indicator against bulb
      sprintf(msg, "%.0f`C", desired);
    }
    else {
      sprintf(msg, "%.1fHz", -desired);
    }
#ifdef MINI_TARGETLABEL
    display.setFontInfo(&MINIFONT);  // select Mini Font
#endif
    display.setCursor(X_TARGET_ICON + (W_TARGET_ICON/2), 
                      Y_BASELINE);   // set centre position of text
    display.printCentreJustified(msg);
    display.setFontInfo(NULL);  
  }
}

#define BODY_YPOS(A) ((((100 - A) * 3) / 16) + 22)   // 100degC centre - ticks +- 80C
void showBodyThermometer(C128x64_OLED& display, int actual) 
{
  // draw bulb design
  display.drawBitmap(X_BODY_BULB, Y_BULB, thermometerBitmap, 8, 50, WHITE);
  // draw mercury
  int yPos = Y_BULB + BODY_YPOS(actual);
  display.drawLine(X_BODY_BULB + 3, yPos, X_BODY_BULB + 3, Y_BULB + 42, WHITE);
  display.drawLine(X_BODY_BULB + 4, yPos, X_BODY_BULB + 4, Y_BULB + 42, WHITE);
  // print actual temperature
  display.setTextColor(WHITE);
  char label[16];
  // determine width and position right justified
#ifdef MINI_BODYLABEL
  sprintf(label, "%d`C", actual);
  display.setFontInfo(&MINIFONT);  // select Mini Font
#else
  sprintf(label, "%d", actual);
#endif
  display.setCursor(display.width(), Y_BASELINE);
  display.printRightJustified(label);
  display.setFontInfo(NULL);  
}


void showGlowPlug(C128x64_OLED& display, int power)
{
  display.drawBitmap(X_GLOW_ICON, Y_GLOW_ICON, GlowPlugIcon, W_GLOW_ICON, H_GLOW_ICON, WHITE);
//  animateGlow = true;
  char msg[16];
  sprintf(msg, "%dW", power);
#ifdef MINI_GLOWLABEL  
  display.setFontInfo(&MINIFONT);  // select Mini Font
#endif
  display.setCursor(X_GLOW_ICON + (W_GLOW_ICON/2), 
                    Y_GLOW_ICON + H_GLOW_ICON + 3); // set centre position of text
  display.printCentreJustified(msg);
  display.setFontInfo(NULL);  
}

void showFan(C128x64_OLED& display, int RPM)
{
  // NOTE: fan rotation animation performed in animateOLED
  animateRPM = RPM != 0;   // used by animation routine

  display.setTextColor(WHITE);
  char msg[16];
  sprintf(msg, "%d", RPM);
#ifdef MINI_FANLABEL  
  display.setFontInfo(&MINIFONT);  // select Mini Font
#endif
  display.setCursor(X_FAN_ICON + (W_FAN_ICON/2), 
                    Y_BASELINE);
  display.printCentreJustified(msg);
  display.setFontInfo(NULL);  
}

void showFuel(C128x64_OLED& display, float rate)
{
  // NOTE: fuel drop animation performed in animateOLED
  animatePump = rate != 0;    // used by animation routine
  if(rate) {
    char msg[16];
    sprintf(msg, "%.1f", rate);
#ifdef MINI_FUELLABEL
    display.setFontInfo(&MINIFONT);  // select Mini Font
#endif
    display.setCursor(X_FUEL_ICON + (W_FUEL_ICON/2),
                      Y_BASELINE);
    display.printCentreJustified(msg);
    display.setFontInfo(NULL);  // select Mini Font
  }
}

void showRunState(C128x64_OLED& display, int runstate, int errstate) 
{
  static bool toggle = false;
  const char* toPrint = NULL;
  int yPos = 25;
  display.setTextColor(WHITE, BLACK);
  if(runstate >= 0 && runstate <= 8) {
    if(((runstate == 0) || (runstate > 5)) && errstate) {
      // create an "E-XX" message to display
      char msg[16];
      sprintf(msg, "E-%02d", errstate);
      if(runstate > 5)
        yPos -= 8;
      display.setCursor(display.xCentre(), yPos);
      yPos += 8;
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
      toPrint = Runstates[runstate];
    }
  }
  if(toPrint) {
      display.setCursor(display.xCentre(), yPos);
      display.printCentreJustified(toPrint);
  }
}


