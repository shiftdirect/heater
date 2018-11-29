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

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <Arduino.h>
#include "FontTypes.h"


class CProtocol;
class C128x64_OLED;
class CScreen;

class CScreenManager {
  static const int _maxScreens = 5;
  CScreen* _pScreen[_maxScreens];
  CScreen* _pActiveScreen;
  C128x64_OLED* _pDisplay;
  int _currentScreen;
  bool _bReqUpdate;
  void _switchScreen();
public:
  CScreenManager();
  ~CScreenManager();
  void init();
  void checkUpdate(const CProtocol& CtlFrame, const CProtocol& HtrFrame);
  void animate();
  void nextScreen();
  void prevScreen();
  void keyHandler(uint8_t event);
  void reqUpdate();
};

class CScreen {
protected:
  C128x64_OLED& _display;
  CScreenManager& _ScreenManager;
  void showBTicon();
  void showWifiIcon();
  void showBatteryIcon(float voltage);
  void _drawSelectionBox(int x, int y, const char* str, int border = 3, int radius = 4);
  void _drawSelectionBoxRightJustified(int x, int y, const char* str, int border = 3, int radius = 4);
  void _drawSelectionBoxCentreJustified(int x, int y, const char* str, int border = 3, int radius = 4);
  void _drawMenuText(int x, int y, const char* str);
  void _drawMenuText(int x, int y, bool selected, const char* str, int border = 3, int radius = 4);
  void _drawMenuTextCentreJustified(int x, int y, const char* str);
  void _drawMenuTextCentreJustified(int x, int y, bool selected, const char* str, int border = 3, int radius = 4);
  void _drawMenuTextRightJustified(int x, int y, const char* str);
  void _drawMenuTextRightJustified(int x, int y, bool selected, const char* str, int border = 3, int radius = 4);
  void _printInverted(int x, int y, const char* str);
  void _printInvertedConditional(int x, int y, bool selected, const char* str);
public:
  CScreen(C128x64_OLED& disp, CScreenManager& mgr); 
  virtual ~CScreen(); 
  virtual void animate();
  virtual void show(const CProtocol& CtlFrame, const CProtocol& HtrFrame);
  virtual void keyHandler(uint8_t event) {};
};

class CAutoFont {
  C128x64_OLED& _display;
public:
  CAutoFont(C128x64_OLED& disp, const FONT_INFO* pFont);
  ~CAutoFont(); 
};

#endif // __DISPLAY_H__
