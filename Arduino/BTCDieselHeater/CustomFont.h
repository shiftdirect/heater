#include "Adafruit_SH1106.h"

#include "FontTypes.h"

class CCustomFont : public Adafruit_SH1106 {
	const FONT_INFO* m_pFontInfo;
public:
  CCustomFont(int8_t DC, int8_t CS, int8_t RST);

  void drawDotFactoryChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, const FONT_INFO* pFontDescriptor, int& xsize, int& ysize);
  void setFontInfo(const FONT_INFO* pFontInfo) { m_pFontInfo = pFontInfo; };
  void offsetCursor(int16_t x, int16_t y) {
    cursor_x += x;
    cursor_y += y;
  };

  size_t write(uint8_t c);
};
