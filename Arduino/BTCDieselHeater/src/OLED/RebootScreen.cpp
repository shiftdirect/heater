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

#include "RebootScreen.h"
#include "../Utility/NVstorage.h"

///////////////////////////////////////////////////////////////////////////
//
// CRebootScreen
//
// This screen presents information prior to a forced reboot
//
///////////////////////////////////////////////////////////////////////////


CRebootScreen::CRebootScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreen(display, mgr) 
{
}


bool 
CRebootScreen::show()
{
  int yPos = 0;

  _display.clearDisplay();
   
  long tDelta =  _restartTime - millis();
  if(tDelta < 0) {
    ESP.restart();
  }
  tDelta /= 1000;
  tDelta++;

  char msg[20];
  char fillmsg[20];
  memset(fillmsg, ' ', 20);
  sprintf(msg, " REBOOT %d ", tDelta);
  fillmsg[strlen(msg)] = 0;

  _printInverted(_display.xCentre(), yPos, fillmsg, true, eCentreJustify); 
  yPos += _display.textHeight();
  _printInverted(_display.xCentre(), yPos, msg, true, eCentreJustify); 
  yPos += _display.textHeight();
  _printInverted(_display.xCentre(), yPos, fillmsg, true, eCentreJustify); 

  yPos += _display.textHeight() + 10;

  for(int i = 0; i < 2; i++) {
    _printMenuText(_display.xCentre(), yPos, _rebootMessage[i].c_str(), false, eCentreJustify); 
    yPos += _display.textHeight() + 10;
  }

  return true;
}


bool 
CRebootScreen::keyHandler(uint8_t event)
{
  return true;
}

void
CRebootScreen::setMessage(const char* content[2], long delayTime)
{
  for(int i = 0; i<2; i++)
    _rebootMessage[i] = content[i];

  _restartTime = millis() + delayTime;
}
