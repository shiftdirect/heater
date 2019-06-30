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

#include "MicroFont.h"
// 
//  Font data for a 3x5 font
//  Hand carved by Ray Jones, based upon The Dot Factory font tool output structures
// 

// Character bitmaps for a 3x4 font
const uint8_t microFontBitmaps[] PROGMEM =
{
	// @1 '0' (3 pixels wide)
	0x60,   //   ##
	0x81,   //  #  #
	0x60,   //   ##

	// @4 '1' (3 pixels wide)
	0x80,   //  #  
	0xf0,   //  ####
	0x00,   //      

	// @7 '2' (3 pixels wide)
	0xb0,   // # ##
	0xd0,   // ## #
	0x00,   //  

	// @10 '3' (3 pixels wide)
	0x90,   // #  #
	0xb0,   // # ##
	0xf0,   // ####

	// @13 '4' (3 pixels wide)
	0x60,   //   ##   
	0xb0,   //  # ##   
	0x00,   //      

	// @16 '5' (3 pixels wide)
	0xd0,   //  ## #
	0xb0,   //  # ##
	0x00,   //  

	// @19 '6' (3 pixels wide)
	0xf0,   //  ####
	0x30,   //    ##  
	0x00,   //     

	// @22 '7' (3 pixels wide)
	0x80,   //  #
	0xf0,   //  ####
	0x00,   //       

	// @25 '8' (3 pixels wide)
	0xf8,   //  #####
	0xa8,   //  # # #
	0xF8,   //  #####

	// @28 '9' (3 pixels wide)
	0xc0,   //  ## 
	0xf0,   //  ####
	0x00,   //      

	
};

// Character descriptors for a 3x5 font
// { [Char width in bits], [Char height in bits], [Offset into tahoma_16ptCharBitmaps in bytes] }
const FONT_CHAR_INFO miniFontDescriptors[] PROGMEM =
{
	{3, 5, 1},      // '0' 
	{3, 5, 4},      // '1' 
	{3, 5, 7},      // '2' 
	{3, 5, 10},     // '3' 
	{3, 5, 13},     // '4' 
	{3, 5, 16},     // '5' 
	{3, 5, 19},     // '6' 
	{3, 5, 22},     // '7' 
	{3, 5, 25},     // '8' 
	{3, 5, 28},     // '9' 
};

// Font information for Mini Font, a 3x5 font
// easier to leave in RAM, not that big anyway
const FONT_INFO microFontInfo =
{
	4,    //  Character height
	'0',  //  Start character
  '9',  // End character
	1,    //  Width, in pixels, of space character
	microFontDescriptors, //  Character descriptor array
	microFontBitmaps,     //  Character bitmap array
};

