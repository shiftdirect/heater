#include <Arduino.h>
#include "Screen.h"
#include "128x64OLED.h"
#include "ScreenManager.h"

// base class functionality for screens

CScreen::CScreen(C128x64_OLED& disp, CScreenManager& mgr) : 
  _display(disp), 
  _ScreenManager(mgr) 
{
}


CScreen::~CScreen()
{
}


bool
CScreen::animate()
{
  return false;
}


void 
CScreen::show()
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

