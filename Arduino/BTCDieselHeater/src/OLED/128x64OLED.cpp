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


#include "128x64OLED.h"
#include "../Utility/DebugPort.h"

#define DBG DebugPort.print
#define DBGln DebugPort.println

//#define DEBUG_FONT

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Dot Factory Settings
// 
// Flip/Rotate      Padding Removal           Line Wrap         Descriptors
//   [X] Flip X       Height(Y): None           (O) At column     [X] Generate descriptor array
//   [ ] Flip Y       Width(X):  Tightest       ( ) At bitmap       Char Width:  In Bits
//       90deg                                                      Char Height: In Bits
//                                                                  Font Height: In Bits
// Comments                  Byte                                     [ ] Multiple descriptor arrays
//   [X] Variable Name         Bit layout: RowMajor                   
//   [X] BMP visualise   [#]   Order:      MSBfirst                 Create new when exceeds [80]
//   [X] Char descriptor       Format:     Hex                        
//    Style: Cpp               Leading:    0x                       Image width:  In Bits
//                                                                  Image height: In Bits
//  Variable name format                                          
//    Bitmaps:   const uint8_t PROGMEM {0}Bitmaps               Space char generation
//    Char Info: const FONT_CHAR_INFO PROGMEM {0}Descriptors      [X] Generate space bitmap
//    Font Info: const FONT_INFO {0}FontInfo                      [2] pixels for space char     
//    Width:     const uint8_t {0}Width                           
//    Height:    const uint8_t {0}Height
//
/////////////////////////////////////////////////////////////////////////////////////////////////////


// Hardware SPI constructor
C128x64_OLED::C128x64_OLED(int8_t DC, int8_t CS, int8_t RST) : OLED_BASE_CLASS(DC, CS, RST)
{
	m_pFontInfo = NULL;
}

// I2C constructor - note important difference in the parameters of the base classes here!!!
#if USE_ADAFRUIT_SH1106 == 1
C128x64_OLED::C128x64_OLED(int8_t SDA, int8_t SCL) : Adafruit_SH1106(SDA, SCL)  // pins for I2C
#elif USE_ADAFRUIT_SSD1306 == 1
C128x64_OLED::C128x64_OLED(int8_t SDA, int8_t SCL) : Adafruit_SSD1306(128, 64)  // width, height
#endif
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
      if (wrap && (cursor_x >= _width)) {
        cursor_y += ysize;
        cursor_x = 0;
      }
    }
  }
  else {
	  OLED_BASE_CLASS::write(c);
  }
#if ARDUINO >= 100
  return 1;
#endif
}

void C128x64_OLED::getTextExtents(const char* str, CRect& rect)
{
  rect.width = 0;
  rect.height = 0;
  if(m_pFontInfo) {
    while(*str) {
      unsigned char c = (unsigned char)*str++;
      if(c >= m_pFontInfo->StartChar && c <= m_pFontInfo->EndChar) {
    	  const FONT_CHAR_INFO* pCharInfo = &m_pFontInfo->pCharInfo[c - m_pFontInfo->StartChar];
        // and extract info from flash (program) storage
        int xsize = pgm_read_byte(&pCharInfo->Width);
        int ysize = pgm_read_byte(&pCharInfo->Height);
        if(ysize > rect.height) 
          rect.height = ysize;
        rect.width += xsize;
      }
      if(*str)  // check if next character exists, if so include space size
        rect.width += m_pFontInfo->SpaceWidth;
    }
  }
  else {
    int16_t x1, y1;
    OLED_BASE_CLASS::getTextBounds(str, 0, 0, &x1, &y1, &rect.width, &rect.height);
  }
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
    unsigned char* addr = (unsigned char*)&pCharInfo->Offset;
//	  uint8_t LSB = pgm_read_byte(&pCharInfo->Offset);
	  uint8_t LSB = pgm_read_byte(addr++);
	  uint8_t MSB = pgm_read_byte(addr);
    int BmpOffset = (MSB << 8) | LSB;
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
C128x64_OLED::printRightJustified(const char* str)
{
  CRect textRect;
  getTextExtents(str, textRect);
  textRect.xPos = cursor_x - textRect.width;  
  textRect.yPos = cursor_y;
  setCursor(textRect.xPos, textRect.yPos);
  print(str);
}

void 
C128x64_OLED::printCentreJustified(const char* str) 
{
  CRect textRect;
  getTextExtents(str, textRect);
  textRect.xPos = cursor_x - (textRect.width / 2);  // use X cursor position as centre point
  textRect.yPos = cursor_y;
  fillRect(textRect.xPos, textRect.yPos, textRect.width, textRect.height, BLACK);
  setCursor(textRect.xPos, textRect.yPos);
  print(str);
}

int  
C128x64_OLED::textHeight()
{
  CRect extents;
  getTextExtents("H", extents);
  return extents.height;
}
