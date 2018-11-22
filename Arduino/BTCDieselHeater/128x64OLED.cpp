/*
 * DrawFont.cpp
 *
 *  Created on: Aug 6, 2014
 *      Author: ray
 */

#include "128x64OLED.h"
#include "DebugPort.h"

#define DBG DebugPort.print
#define DBGln DebugPort.println

//#define DEBUG_FONT

C128x64_OLED::C128x64_OLED(int8_t DC, int8_t CS, int8_t RST) : Adafruit_SH1106(DC, CS, RST)
{
	m_pFontInfo = NULL;
}

size_t C128x64_OLED::write(uint8_t c) 
{
  if(m_pFontInfo) {
    if (c == '\n') {
      cursor_y += textsize*8;
      cursor_x  = 0;
    } else if (c == '\r') {
      // skip em
    } else {
      int xsize, ysize;
      drawDotFactoryChar(cursor_x, cursor_y, c, textcolor, textbgcolor, m_pFontInfo, xsize, ysize);
      cursor_x += xsize + m_pFontInfo->SpaceWidth;
      if (wrap && (cursor_x > (_width - 8))) {
        cursor_y += ysize;
        cursor_x = 0;
      }
    }
  }
  else {
	  Adafruit_SH1106::write(c);
  }
#if ARDUINO >= 100
  return 1;
#endif
}



void
C128x64_OLED::drawDotFactoryChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, const FONT_INFO* pFontDescriptor, int& xsize, int& ysize)
{
#ifdef DEBUG_FONT
  char pr = c;
  DBG(pr); DBG(F(" fg=")); DBG(color); DBG(F(" bg=")); DBGln(bg);
#endif

	uint16_t char2print = c;

  if(c >= pFontDescriptor->StartChar && c <= pFontDescriptor->EndChar) {

#ifdef DEBUG_FONT
  	char pr = c;
	  DBG(pr);
#endif

	  // point to info for selected character
	  const FONT_CHAR_INFO* pCharInfo = &pFontDescriptor->pCharInfo[c - pFontDescriptor->StartChar];
    // and extract info from flash (program) storage
	  int BmpOffset = pgm_read_byte(&pCharInfo->Offset);
    xsize = pgm_read_byte(&pCharInfo->Width);
    ysize = pgm_read_byte(&pCharInfo->Height);

    // point to bitmap data for selected character
	  const uint8_t* pBitmap = &pFontDescriptor->pBitmaps[BmpOffset];

#ifdef DEBUG_FONT
  	DBG(F(" [")); DBG(int(pCharInfo)); DBG(']');
	  DBG(F(" (")); DBG(xsize); DBG(','); DBG(ysize); DBGln(')');
	  delay(1000);
#endif

    uint8_t mask = 0x80;
    uint8_t line = 0;
	  for(int8_t j=0; j < xsize/*pCharInfo->Width*/; j++) {
      for (int8_t i=0; i < ysize/*pCharInfo->Height*/; i++ ) {
    	  if((i & 0x07) == 0) {
	        line = pgm_read_byte(pBitmap++);
    	  }
        if(line & mask) {
          drawPixel(x+j, y+i, color);
        }
        else if(bg != color) {
          drawPixel(x+j, y+i, bg);
        }
        line <<= 1;
  	  }
	  }
  }
}

void
C128x64_OLED::printRightJustify(const char* str, int yPos, int RHS)
{
  int xPos = RHS - strlen(str) * 6;  // standard font width
  setCursor(xPos, yPos);
  print(str);
}

