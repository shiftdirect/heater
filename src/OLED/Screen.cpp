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

#include <Arduino.h>
#include "Screen.h"
#include "fonts/Arial.h"

// base class functionality for screens

CScreen::CScreen(C128x64_OLED& disp, CScreenManager& mgr) : 
  _display(disp), 
  _ScreenManager(mgr) 
{
   _showOEMerror = 0;
}


CScreen::~CScreen()
{
}


bool
CScreen::animate()
{
  if(_showOEMerror) {
    _display.clearDisplay();
    _display.fillRect(8, 20, 112, 24, WHITE);
    if(_showOEMerror & 0x01) {
      _printInverted(_display.xCentre(), 23, "Other controller ", true, eCentreJustify);
      _printInverted(_display.xCentre(), 32, "Operation blocked", true, eCentreJustify);
    }
    _showOEMerror--;
    return true;
  }
  return false;
}


bool 
CScreen::show()
{
  return false;
}

void 
CScreen::onSelect()
{
  _display.clearDisplay();
}

void
CScreen::onExit()
{  
}

void 
CScreen::_printMenuText(int x, int y, const char* str, bool selected, eJUSTIFY justify, int border, int radius)
{
  // position output, according to justification
  CRect extents;
  extents.xPos = x;
  extents.yPos = y;
  _display.getTextExtents(str, extents);
  _adjustExtents(extents, justify, str);

  _display.setCursor(extents.xPos, extents.yPos);
  _display.print(str);
  if(selected) {
    extents.Expand(border);
    _display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
  }
}

void
CScreen::_drawMenuSelection(CRect extents, const char* str, int border, int radius)
{
  CRect resize(extents);
  _display.getTextExtents(str, resize);
//  resize.Expand(border);
//  _display.drawRoundRect(resize.xPos, resize.yPos, resize.width, resize.height, radius, WHITE);
  _drawMenuSelection(resize, border, radius);
}

void
CScreen::_drawMenuSelection(const CRect& extents, int border, int radius)
{
  CRect resize(extents);
  resize.Expand(border);
  _display.drawRoundRect(resize.xPos, resize.yPos, resize.width, resize.height, radius, WHITE);
}

void
CScreen::_printInverted(int x, int y, const char* str, bool selected, eJUSTIFY justify)
{
  // position output, according to justification
  CRect extents;
  extents.xPos = x;
  extents.yPos = y;
  _adjustExtents(extents, justify, str);

  if(selected) {
    _display.setTextColor(BLACK, WHITE);
    extents.Expand(1);
    _display.fillRect(extents.xPos, extents.yPos, extents.width, extents.height, WHITE);
    extents.Expand(-1);
  }
  _display.setCursor(extents.xPos, extents.yPos);
  _display.print(str);
  _display.setTextColor(WHITE, BLACK);
}

void
CScreen::_scrollMessage(int y, const char* str, int& charOffset)
{
  char msg[20];
  int maxIndex = strlen(str) - 20;
  strncpy(msg, &str[charOffset], 19);
  msg[19] = 0;
  _printMenuText(_display.xCentre(), y, msg, false, eCentreJustify);

  charOffset++;
  if(charOffset >= maxIndex) {
    charOffset = 0;
  }
}

void
CScreen::_adjustExtents(CRect& extents, eJUSTIFY justify, const char* str)
{
  _display.getTextExtents(str, extents);
  switch(justify) {
    case eLeftJustify:
      break;
    case eCentreJustify:
      extents.xPos -= extents.width/2;
      break;
    case eRightJustify:
      extents.xPos -= extents.width;
      break;
  }
}

void 
CScreen::_reqOEMWarning()
{
  _showOEMerror = 10;
}

void 
CScreen::_drawBitmap(int x, int y, const BITMAP_INFO& info, uint16_t colour, uint16_t bg)
{
  if(bg == 0xffff) {
     // normal mode - does not erase background
    _display.drawBitmap(x, y, info.pBitmap, info.width, info.height, colour);
  } 
  else {
     // overwrite mode - erases background
    _display.drawBitmap(x, y, info.pBitmap, info.width, info.height, colour, bg);
  }
}


void
CScreen::_showTitle(const char* title)
{
  CTransientFont AF(_display, &arial_8ptBoldFontInfo);
  _printMenuText(_display.xCentre(), -1, title, false, eCentreJustify);
  _display.drawFastHLine(0, 10, 128, WHITE);
}

void
CScreen::_showConfirmMessage()
{
  _showTitle("Saving Settings");
  _printMenuText(_display.xCentre(), 35, "Press UP to", false, eCentreJustify);
  _printMenuText(_display.xCentre(), 43, "confirm save", false, eCentreJustify);
}

void
CScreen::_showStoringMessage()
{
  _display.writeFillRect(34, 19, 60, 26, WHITE);
  CTransientFont AF(_display, &arial_8ptBoldFontInfo);
  _printInverted(_display.xCentre(), 27, " STORING ", true, eCentreJustify);
}


// a class used for temporary alternate fonts usage
// Reverts to standard inbuilt font when the instance falls out of scope
CTransientFont::CTransientFont(C128x64_OLED& disp, const FONT_INFO* pFont) :
  _display(disp)
{
  _display.setFontInfo(pFont);
  _display.setTextColor(WHITE, BLACK);
}


CTransientFont::~CTransientFont() 
{
  _display.setFontInfo(NULL);
}

