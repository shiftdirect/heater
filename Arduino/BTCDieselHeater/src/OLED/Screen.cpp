#include <Arduino.h>
#include "Screen.h"

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
  _display.getTextExtents(str, extents);
  extents.Expand(border);
  _display.drawRoundRect(extents.xPos, extents.yPos, extents.width, extents.height, radius, WHITE);
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

