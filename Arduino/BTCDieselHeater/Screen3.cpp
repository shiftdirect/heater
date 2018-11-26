#include "128x64OLED.h"
#include "display.h"
#include "KeyPad.h"
#include "helpers.h"
#include "Screen3.h"

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


CScreen3::CScreen3(C128x64_OLED& display, CScreenManager& mgr) : CScreen(display, mgr) 
{
  _PrimeStop = 0;
  _PrimeCheck = 0;
  _rowSel = 0;
  _colSel = 0;
}


void 
CScreen3::show(const CProtocol& CtlFrame, const CProtocol& HtrFrame)
{
  CScreen::show(CtlFrame, HtrFrame);
  
  CRect extents;
  _display.setCursor(_display.xCentre(), Row[0]);
  _display.printCentreJustified(Label0);
  if(_rowSel == 0) {
    _display.getTextExtents(Label0, extents);
    extents.xPos = Col[0];
    extents.yPos = Row[0];
    extents.Expand(border);
    _display.drawRoundRect(_display.xCentre() - extents.width/2, extents.yPos, extents.width, extents.height, radius, WHITE);
  }

  // thermostat / fixed mode selection menu
  // highlight active state
  int col = getThermostatMode() ? 0 : 1;              // follow actual heater settings
  _display.getTextExtents(Label1[col], extents);
  extents.xPos = (col == 0) ? border : _display.width() - extents.width - border;
  extents.yPos = Row[1];
  if(_rowSel == 1) {
    // draw selection box
    extents.Expand(border);
    _display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
  }
  else {
    // draw white background
    extents.Expand(1);
    _display.fillRect(extents.xPos, extents.yPos, extents.width, extents.height, WHITE);
  }
  
  if(col == 0 && _rowSel != 1)
    _display.setTextColor(BLACK);
  _display.setCursor(border, Row[1]);
  _display.print(Label1[0]);
  _display.setTextColor(WHITE);

  if(col == 1 && _rowSel != 1)
    _display.setTextColor(BLACK);
  _display.setCursor(_display.width() - border, Row[1]);
  _display.printRightJustified(Label1[1]);
  _display.setTextColor(WHITE);

  // fuel pump priming menu
  _display.setCursor(Col[0], Row[2]);
  _display.print(Label2[0]);
  if(_rowSel == 2) {
    for(int col = 1; col < 3; col++) {
      _display.setCursor(Col[col], Row[2]);
      _display.print(Label2[col]);
    }
    _display.getTextExtents(Label2[_colSel], extents);
    extents.xPos = Col[_colSel];
    extents.yPos = Row[2];
    extents.Expand(border);
    _display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
  }


  if(_rowSel == 2 && _colSel == 2) {
    float pumpHz = getPumpHz();
    long tDelta = millis() - _PrimeCheck;
    if(_PrimeCheck && tDelta > 0 && pumpHz < 0.1) {
      stopPump();
    }
    tDelta = millis() - _PrimeStop;
    if(_PrimeStop && tDelta > 0) {
      stopPump();
    }

    if(_PrimeStop) {
      char msg[16];
      sprintf(msg, "%.1fHz", pumpHz);
      _display.getTextExtents(msg, extents);
      extents.xPos = _display.width() - extents.width - border;
      extents.yPos = Row[2];
      extents.Expand(border);
      _display.fillRect(extents.xPos, extents.yPos, extents.width, extents.height, BLACK);
      _display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
      extents.Expand(-border);
      _display.setCursor(extents.xPos, extents.yPos);
      _display.print(msg);
    }
  }

  _display.display();

}


void 
CScreen3::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // press CENTRE
    if(event & key_Centre) {
      return;
    }
    // press LEFT 
    if(event & key_Left) {
      switch(_rowSel) {
        case 0: 
          _Manager.prevScreen(); 
          break;
        case 1: 
          _colSel = 0; 
          setThermostatMode(1);
          break;
        case 2: 
          _colSel = 1; 
          break;
        case 3: break;
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      switch(_rowSel) {
        case 0: 
          _Manager.nextScreen(); 
          break;
        case 1: 
          _colSel = 1; 
          setThermostatMode(0);
          break;
        case 2: 
          if(!getRunState()) 
            _colSel = 2; 
          break;
        case 3: break;
      }
    }
    // press UP
    if(event & key_Up) {
      _rowSel++;
      UPPERLIMIT(_rowSel, 2);
      if(_rowSel == 2)
        _colSel = 1;       // select OFF upon entry to priming menu
    }
    // press DOWN
    if(event & key_Down) {
      _rowSel--;
      LOWERLIMIT(_rowSel, 0);
      _colSel = 0;
    }

    // check if fuel priming was selected
    if(_rowSel == 2 && _colSel == 2) {
      reqPumpPrime(true);
      _PrimeStop = millis() + 150000;   // allow 2.5 minutes - much the same as the heater itself cuts out at
      _PrimeCheck = millis() + 3000;    // holdoff upon start before testing for heater shutting off pump
    }
    else {
      stopPump();
    }

    reqDisplayUpdate();
  }
}

void 
CScreen3::stopPump()
{
  reqPumpPrime(false);
  _PrimeCheck = 0;
  _PrimeStop = 0;
  if(_colSel == 2)
    _colSel = 1;
}
