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
