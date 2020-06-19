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
#include "BME280Screen.h"
#include "KeyPad.h"
#include "fonts/Icons.h"
#include "../Utility/TempSense.h"


CBME280Screen::CBME280Screen(C128x64_OLED& display, CScreenManager& mgr) : CPasswordScreen(display, mgr) 
{
  _initUI();
}

bool 
CBME280Screen::show()
{
  char msg[32];

  _display.clearDisplay();

  if(!CPasswordScreen::show()) {  // for showing "saving settings"

    _showTitle("BME280 status");

    if(getTempSensor().getBME280().getCount()) {
      float temperature;
      float humidity;
      float altitude;

      _printMenuText(76, 16, "Temperature:", false, eRightJustify);
      _printMenuText(76, 26, "Humidity:", false, eRightJustify);
      _printMenuText(76, 36, "Altitude:", false, eRightJustify);

      getTempSensor().getTemperatureBME280(temperature);
      getTempSensor().getHumidity(humidity);
      getTempSensor().getAltitude(altitude);

      sprintf(msg, "%.01f`C", temperature);
      _printMenuText(80, 16, msg, false);
      sprintf(msg, "%.01f%%", humidity);
      _printMenuText(80, 26, msg, false);
      sprintf(msg, "%.0fm", altitude);
      _printMenuText(80, 36, msg, false);
    }
    else {
      _printMenuText(64, 16, "Sensor not found", false, eCentreJustify);
    }

    _printMenuText(_display.xCentre(), 53, " \021      Exit      \020 ", true, eCentreJustify);
  }

  return true;
}



bool 
CBME280Screen::keyHandler(uint8_t event)
{
  if(event & keyPressed) {
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

