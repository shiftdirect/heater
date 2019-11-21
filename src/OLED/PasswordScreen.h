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
#ifndef __PASSWORDSCREEN_H__
#define __PASSWORDSCREEN_H__

#include <stdint.h>
//#include "ScreenHeader.h"
#include "UIEditScreen.h"

class C128x64_OLED;
class CScreenManager;

class CPasswordScreen : public CUIEditScreen {
  int  _PWdig[4];
  bool _bGetPassword;
  bool _bPasswordOK;
  int  _PWcol;
  unsigned long _SaveTime;
  static long __Expiry;
  void __initPassword(bool get);
protected:
  bool _showPassword();
  void _holdPassword();
  void _getPassword();
  bool _isPasswordOK();
  void _initUI();
  virtual void _saveNV() {};
//  int  _rowSel;
public:
  CPasswordScreen(C128x64_OLED& display, CScreenManager& mgr);
  bool show();
  bool keyHandler(uint8_t event);
  bool isPasswordBusy();
};

#endif