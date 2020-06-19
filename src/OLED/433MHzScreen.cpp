/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2020  Ray Jones <ray@mrjones.id.au>
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


///////////////////////////////////////////////////////////////////////////
//
// C433MHzScreen
//
// This screen allows the pairing of 433MHz remotes
//
///////////////////////////////////////////////////////////////////////////

#include "433MHzScreen.h"
#include "KeyPad.h"
#include "fonts/Arial.h"
#include "../RTC/Clock.h"
#include "../Utility/macros.h"
#include "../Utility/NVStorage.h"
#include "../Protocol/433MHz.h"
#include "fonts/Icons.h"

extern bool pair433MHz;
static const int column[] = { 64, 84, 104, 120 };
static const int line[] = { 42, 31, 20 };

C433MHzScreen::C433MHzScreen(C128x64_OLED& display, CScreenManager& mgr) : CUIEditScreen(display, mgr) 
{
  _initUI();
}

void
C433MHzScreen::onSelect()
{
  CScreen::onSelect();
  _initUI();
  pair433MHz = true;
  UHFremote.getCodes(_rawCodes);
}

void 
C433MHzScreen::onExit()
{
  pair433MHz = false;
}


void
C433MHzScreen::_initUI()
{
  CUIEditScreen::_initUI();
  _repeatCount = 0;
}


bool 
C433MHzScreen::show()
{
  _display.clearDisplay();

  if(!CUIEditScreen::show()) {

    _showTitle("433MHz Remote");

    _drawBitmap(61, 13, medStopIconInfo);
    _drawBitmap(81, 12, medStartIconInfo);
    _drawBitmap(100, 14, dnIconInfo);
    _drawBitmap(116, 13, upIconInfo);

    _printMenuText(5, line[2], "Remote 1", _rowSel == 3 && _colSel == 0);
    _printMenuText(5, line[1], "Remote 2", _rowSel == 2 && _colSel == 0);
    _printMenuText(5, line[0], "Remote 3", _rowSel == 1 && _colSel == 0);

    if(_rowSel == 0) {
      _printMenuText(_display.xCentre(), 53, " \021      Exit      \020 ", true, eCentreJustify);
    }
    else {
      switch(_colSel) {
        case 0:
          _printMenuText(_display.xCentre(), 54, " \020 to start teaching ", false, eCentreJustify);
          break;
        case 1:
          _printMenuText(_display.xCentre(), 54, " Teach \"Off\"", false, eCentreJustify);
          break;
        case 2:
          _printMenuText(_display.xCentre(), 54, " Teach \"On\" ", false, eCentreJustify);
          break;
        case 3:
          _printMenuText(_display.xCentre(), 54, " Teach \"Decrease\" ", false, eCentreJustify);
          break;
        case 4:
          _printMenuText(_display.xCentre(), 54, " Teach \"Increase\" ", false, eCentreJustify);
          break;
      }
    }
  }

  return true;
}

bool
C433MHzScreen::animate()
{

  if(_saveBusy()) {
    return false;
  }

  if(UHFremote.available()) {
    UHFremote.read(_code);
    DebugPort.printf("UHF remote code = %08lX\r\n", _code);
    if(_colSel) {
      if(_code) {
        _rawCodes[_rowSel-1][_colSel-1] = _code;
      }
      else {
        _colSel++;
        WRAPLIMITS(_colSel, 0, 4);
      }
    }
  }
  for(int row = 0; row < 3; row++) {
    for(int col = 0; col < 4; col++) {
      int xPos = column[col];
      int yPos = line[row];
      bool rowColMatch = (row == (_rowSel-1)) && (col == (_colSel-1));
      if(_rawCodes[row][col]) {
        if(rowColMatch)
          _printMenuText(xPos, yPos, "*", true, eCentreJustify);
        else 
          _printInverted(xPos, yPos, "*", _rawCodes[row][col] == _code, eCentreJustify);
      }
      else {
        _printMenuText(xPos, yPos, " ", rowColMatch, eCentreJustify);
      }
    }
  }
  return true;
}

bool 
C433MHzScreen::keyHandler(uint8_t event)
{

  if(CUIEditScreen::keyHandler(event)) {   // manage password collection and NV save confirm
    return true;
  }

  if(event & keyPressed) {
    _repeatCount = 0;
    // press CENTRE
    if(event & key_Centre) {
    }
    // press LEFT 
    if(event & key_Left) {
      if(_rowSel == 0) {
        _ScreenManager.prevMenu();
      }
      else {
        _colSel--;
        WRAPLOWERLIMIT(_colSel, 0, 4);
      }
    }
    // press RIGHT 
    if(event & key_Right) {
      if(_rowSel == 0) {
        _ScreenManager.nextMenu();
      }
      else {
        _colSel++;
        WRAPUPPERLIMIT(_colSel, 4, 0);
      }
    }
    // press UP 
    if(event & key_Up) {
      _rowSel++;
      _colSel = 0;
      UPPERLIMIT(_rowSel, 4);
    }
    // press DOWN
    if(event & key_Down) {
      _rowSel--;
      _colSel = 0;
      LOWERLIMIT(_rowSel, 0);
    }
  }

  if(event & keyRepeat) {
    _repeatCount++;
    UPPERLIMIT(_repeatCount, 5);
    if(_repeatCount == 2) {
      if(_rowSel && _colSel) {
        _rawCodes[_rowSel-1][_colSel-1] = 0;  // scrub code for button
        _colSel++;
        WRAPLIMITS(_colSel, 0, 4);
      }
    }
  }

  if(event & keyReleased) {
    // press CENTRE
    if(event & key_Centre) {
      if(_rowSel == 0) {
        _ScreenManager.selectMenu(CScreenManager::RootMenuLoop);  // force return to main menu
      }
      else if(_repeatCount == 0) {
        _confirmSave();   // enter save confirm mode
        _rowSel = 0;
      }
    }
  }

  _ScreenManager.reqUpdate();
  return true;
}

// Data word in NV ram is stored as follows
//
//  | 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
//  |                        |                       |                       |                       |
//  |                        |                       |           |  Enabled  |       Key Codes       |
//  |                    Unique ID (20 bits)                     | U  D  R  S| KCU | KCD | KCR | KCS | 
//
//  Key enabled bits
//     U = Up     - key code in b7-b6
//     D = Down   - key code in b5-b5
//     R = Run    - key code in b3-b2
//     S = Stop   - key code in b1-b0
//
//  key code bits
//     00 => 0x01
//     01 => 0x02
//     10 => 0x04
//     11 => 0x08
//
/*void
C433MHzScreen::_decode(int idx)
{
  unsigned long code = _savedCodes[idx];
  for(int i=0; i<4; i++) {
    int mask = 0x100 << i;
    if(code & mask) {
      int uniqueID = (code >> 8) & 0xFFFFF0;
      int shift = (code >> (i*2)) & 0x3;
      int keyCode = 1 << shift;
      _rawCodes[idx][i] = uniqueID | keyCode;
    }
    else 
      _rawCodes[idx][i] = 0;
  }
}*/

/*int
C433MHzScreen::_encode(int idx)
{
  unsigned long uniqueCode = _rawCodes[idx][0] & 0xFFFFF0;
  
  // confirm all recorded keys share the same unique code
  for(int i=1; i<4; i++) {
    if(_rawCodes[idx][i] && (uniqueCode != (_rawCodes[idx][i] & 0xFFFFF0))) {
      return -1;
    }
  }

  // start building the encoded value for NV storage
  unsigned long encoded = uniqueCode << 8;
  for(int i=0; i<4; i++) {
    if(_rawCodes[idx][i]) {
      int keyCode = _rawCodes[idx][i] & 0xf;
      switch(keyCode) {
        case 1:
          encoded |= (0 << i*2);
          break;
        case 2:
          encoded |= (1 << i*2);
          break;
        case 4:
          encoded |= (2 << i*2);
          break;
        case 8:
          encoded |= (3 << i*2);
          break;
        default:
          return -2;
          break;
      }
      encoded |= (0x100 << i);
    }
  }
  _savedCodes[idx] = encoded;
  return 0;
}*/

void  
C433MHzScreen::_saveNV()
{
  UHFremote.saveNV(_rawCodes);
  // sUserSettings userSettings = NVstore.getUserSettings();
  // for(int i=0; i<3; i++) {
  //   int err = _encode(i);
  //   if(err != 0) {
  //     DebugPort.printf("Error encoding UHF code (%d)\r\n", err);
  //     return;
  //   }
  //   userSettings.UHFcode[i] = _savedCodes[i];
  // }

  // DebugPort.println("UHF Remote encodes");
  // for(int i = 0; i<3; i++) {
  //   DebugPort.printf("0x%08lX 0x%08lX 0x%08lX 0x%08lX => 0x%08lX\r\n", _rawCodes[i][0], _rawCodes[i][1], _rawCodes[i][2], _rawCodes[i][3], _savedCodes[i]);
  // }
  // NVstore.setUserSettings(userSettings);
  // NVstore.save();
}
