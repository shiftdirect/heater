#include "128x64OLED.h"
#include "display.h"
#include "KeyPad.h"
#include "helpers.h"
#include "Screen3.h"

int rowSel = 0;
int colSel = 0;
const int border = 3;
const int radius = 4;
const int Row[] = { 52, 40, 28, 16 } ;
const int Col[] = { border, 70, 100};
const char* Label0 = "Prev/Next Screen"; 
const char* Label1[] = { "Thermostat",
                         "Fixed Hz" }; 
const char* Label2[] = { "Prime pump",
                          "OFF",
                          "ON" }; 

void deltaCol(int dir);
void stopPump();

unsigned long PrimeStop = 0;
unsigned long PrimeCheck = 0;

void showScreen3(C128x64_OLED& display)
{
  CRect extents;
  display.setCursor(display.xCentre(), Row[0]);
  display.printCentreJustified(Label0);
  if(rowSel == 0) {
    display.getTextExtents(Label0, extents);
    extents.xPos = Col[0];
    extents.yPos = Row[0];
    extents.Expand(border);
    display.drawRoundRect(display.xCentre() - extents.width/2, extents.yPos, extents.width, extents.height, radius, WHITE);
  }

  // thermostat / fixed mode selection menu
  // highlight active state
  int col = getThermostatMode() ? 0 : 1;              // follow actual heater settings
  display.getTextExtents(Label1[col], extents);
  extents.xPos = (col == 0) ? border : display.width() - extents.width - border;
  extents.yPos = Row[1];
  if(rowSel == 1) {
    // draw selection box
    extents.Expand(border);
    display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
  }
  else {
    // draw white background
    extents.Expand(1);
    display.fillRect(extents.xPos, extents.yPos, extents.width, extents.height, WHITE);
  }
  
  if(col == 0 && rowSel != 1)
    display.setTextColor(BLACK);
  display.setCursor(border, Row[1]);
  display.print(Label1[0]);
  display.setTextColor(WHITE);

  if(col == 1 && rowSel != 1)
    display.setTextColor(BLACK);
  display.setCursor(display.width() - border, Row[1]);
  display.printRightJustified(Label1[1]);
  display.setTextColor(WHITE);

  // fuel pump priming menu
  display.setCursor(Col[0], Row[2]);
  display.print(Label2[0]);
  if(rowSel == 2) {
    for(int col = 1; col < 3; col++) {
      display.setCursor(Col[col], Row[2]);
      display.print(Label2[col]);
    }
    display.getTextExtents(Label2[colSel], extents);
    extents.xPos = Col[colSel];
    extents.yPos = Row[2];
    extents.Expand(border);
    display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
  }


  if(rowSel == 2 && colSel == 2) {
    float pumpHz = getPumpHz();
    long tDelta = millis() - PrimeCheck;
    if(PrimeCheck && tDelta > 0) {
      if(pumpHz < 0.1) {
        stopPump();
      }
    }
    tDelta = millis() - PrimeStop;
    if(PrimeStop && tDelta > 0) {
      stopPump();
    }

    if(PrimeStop) {
      char msg[16];
      sprintf(msg, "%.1fHz", pumpHz);
      display.getTextExtents(msg, extents);
      extents.xPos = display.width() - extents.width - border;
      extents.yPos = Row[2];
      extents.Expand(border);
      display.fillRect(extents.xPos, extents.yPos, extents.width, extents.height, BLACK);
      display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
      extents.Expand(-border);
      display.setCursor(extents.xPos, extents.yPos);
      display.print(msg);
    }
  }

}

void animateScreen3(C128x64_OLED& display)
{
  display.display();
}


void keyhandlerScreen3(uint8_t event)
{
  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      return;
    }
    // press LEFT 
    if(event & key_Left) {
      switch(rowSel) {
        case 0: 
          prevScreen(); 
          break;
        case 1: 
          colSel = 0; 
          setThermostatMode(1);
          break;
        case 2: 
          colSel = 1; 
          break;
        case 3: break;
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      switch(rowSel) {
        case 0: 
          nextScreen(); 
          break;
        case 1: 
          colSel = 1; 
          setThermostatMode(0);
          break;
        case 2: 
          if(!getRunState()) 
            colSel = 2; 
          break;
        case 3: break;
      }
    }
    // press UP
    if(event & key_Up) {
      rowSel++;
      UPPERLIMIT(rowSel, 2);
      if(rowSel == 2)
        colSel = 1;       // select OFF upon entry to priming menu
    }
    // press DOWN
    if(event & key_Down) {
      rowSel--;
      LOWERLIMIT(rowSel, 0);
      colSel = 0;
    }

    // check if fuel priming was selected
    if(rowSel == 2 && colSel == 2) {
      reqPumpPrime(true);
      PrimeStop = millis() + 150000;   // allow 2.5 minutes - much the same as the heater itself cuts out at
      PrimeCheck = millis() + 3000;    // holdoff upon start before testing for heater shutting off pump
    }
    else {
      stopPump();
    }

    reqDisplayUpdate();
  }
}

void stopPump()
{
  reqPumpPrime(false);
  PrimeCheck = 0;
  PrimeStop = 0;
  if(colSel == 2)
    colSel = 1;
}
