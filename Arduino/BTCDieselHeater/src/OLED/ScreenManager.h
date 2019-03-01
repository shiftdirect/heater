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

#ifndef __SCREEN_MANAGER_H__
#define __SCREEN_MANAGER_H__

#include <Arduino.h>
#include <vector>

class CProtocol;
class C128x64_OLED;
class CScreen;
class CRebootScreen;

class CScreenManager {
  std::vector<CScreen*> _Screens;
  std::vector<CScreen*> _TimerScreens;
  std::vector<CScreen*> _SettingsScreens;
  CScreen* _SetTimeScreen;
  CScreen* _InheritScreen;
  C128x64_OLED* _pDisplay;
  int _currentScreen;
  int _timerScreen;
  int _settingScreen;
  bool _bSetTimeScreenActive;
  bool _bInheritScreenActive;
  unsigned long _DimTime;
  bool _bReqUpdate;
  void _enterScreen();
  void _leaveScreen();
  CRebootScreen* _pRebootScreen;
public:
  CScreenManager();
  ~CScreenManager();
  void begin(bool bNoClock);
  bool checkUpdate();
  bool animate();
  void refresh();
  void nextScreen();
  void prevScreen();
  void keyHandler(uint8_t event);
  void reqUpdate();
  void showRebootMsg(const char* content[2], long delayTime);
  void selectTimerScreen(bool show);
  void selectSetTimeScreen(bool show);
  void selectSettingsScreen(bool show);
  void selectInheritScreen(bool show);
};

#endif // __SCREEN_MANAGER_H__
