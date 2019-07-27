/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2019  Ray Jones <ray@mrjones.id.au>
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
#include "HourMeterScreen.h"
#include "KeyPad.h"
#include "../Utility/helpers.h"
#include "../Utility/HourMeter.h"
#include "fonts/Arial.h"


CHourMeterScreen::CHourMeterScreen(C128x64_OLED& display, CScreenManager& mgr) : CScreen(display, mgr) 
{
}

void makeHourMeter(char* hrs, char* str, unsigned long seconds)
{
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  
  unsigned long printMins = minutes - (hours * 60);

  sprintf(hrs, "%02ld:", hours);

  if(hours > 48)
    sprintf(str, "%02ld (%ldd)", printMins, days);
  else
    sprintf(str, "%02ld", printMins);
}

bool 
CHourMeterScreen::show()
{
  char msg[32];
  char hrs[16];

  int colon = 75;

  _display.clearDisplay();

  // standard version information screens,
  // animation of update available via animate() if firmware update is available on web server
  _showTitle("Hour Meters");
  
  makeHourMeter(hrs, msg, pHourMeter->getRunTime());
  _printMenuText(38, 14, "Run", false, eRightJustify);
  _printMenuText(colon, 14, hrs, false, eRightJustify);
  _printMenuText(colon, 14, msg);
  makeHourMeter(hrs, msg, pHourMeter->getGlowTime());
  _printMenuText(38, 26, "Glow", false, eRightJustify);
  _printMenuText(colon, 26, hrs, false, eRightJustify);
  _printMenuText(colon, 26, msg);
  makeHourMeter(hrs, msg, sysUptime());
  _printMenuText(38, 38, "UpTime", false, eRightJustify);
  _printMenuText(colon, 38, hrs, false, eRightJustify);
  _printMenuText(colon, 38, msg);

  _printMenuText(_display.xCentre(), 53, " \021                \020 ", true, eCentreJustify);     // " <              > "
  _printMenuText(_display.xCentre(), 53, "Exit", false, eCentreJustify);     // " <     Exit     > "
  return true;
}

bool 
CHourMeterScreen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
    // UP press
    if(event & key_Up) {
    }
    // DOWN press
    if(event & key_Down) {
    }
    // LEFT press
    if(event & key_Left) {
      _ScreenManager.prevMenu();
    }
    // RIGHT press
    if(event & key_Right) {
      _ScreenManager.nextMenu();
    }
    // CENTRE press
    if(event & key_Centre) {
      _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
    }
  }

  _ScreenManager.reqUpdate();

  return true;
}

