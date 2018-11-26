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

#include "Adafruit_SH1106.h"

#include "FontTypes.h"
#include "UtilClasses.h"

class C128x64_OLED : public Adafruit_SH1106 {
	const FONT_INFO* m_pFontInfo;
public:
  C128x64_OLED(int8_t DC, int8_t CS, int8_t RST);    // Hardware SPI constructor
  C128x64_OLED(int8_t SDA, int8_t SCL);              // I2C constructor

  void drawDotFactoryChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, const FONT_INFO* pFontDescriptor, int& xsize, int& ysize);
  void setFontInfo(const FONT_INFO* pFontInfo) { m_pFontInfo = pFontInfo; };
  void offsetCursor(int16_t x, int16_t y) {
    cursor_x += x;
    cursor_y += y;
  };
  void getTextExtents(const char* str, CRect& rect);

  void printRightJustified(const char* str);
  void printCentreJustified(const char* str); 

  int  xCentre() { return width() / 2; };

  size_t write(uint8_t c);
};
