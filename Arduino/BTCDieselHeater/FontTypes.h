#ifndef __FONT_TYPES_H__
#define __FONT_TYPES_H__

#include <stdint.h>

#ifdef __AVR__
 #include <avr/io.h>
 #include <avr/pgmspace.h>
#else
 #define PROGMEM
#endif


typedef struct  {
  uint8_t Width;                  // Char width in bits
  uint8_t Height;
  uint8_t Offset;                 // Offset into bitmap array bytes)
} FONT_CHAR_INFO;

typedef struct  {
  uint8_t nBitsPerLine;         //  Character "height"
  uint8_t StartChar;             //  Start character
  uint8_t EndChar;               //  End character
  uint8_t SpaceWidth;
  const FONT_CHAR_INFO* pCharInfo;  //  Character descriptor array
  const unsigned char* pBitmaps;   //  Character bitmap array
} FONT_INFO;

#endif
