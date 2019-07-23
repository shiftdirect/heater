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

#include "MidiFont.h"
// 
//  Font data for Segoe UI 8pt
// 

// Character bitmaps for Segoe UI 8pt
const uint8_t PROGMEM segoeUI_8ptBitmaps [] = 
{
	// @0 ' ' (2 pixels wide)
	0x00, 0x00, //              
	0x00, 0x00, //              

	// @4 '-' (3 pixels wide)
	0x08, // #
	0x08, // #
	0x08, // #

	// @7 '.' (1 pixels wide)
	0x03, // ##

	// @8 '0' (6 pixels wide)
	0x3E, //   ##### 
	0xC3, // ##    ##
	0x81, // #      #
	0x81, // #      #
	0xC3, // ##    ##
	0x7C, //  #####  

	// @14 '1' (3 pixels wide)
	0x40, //  #      
	0x40, //  #      
	0xFF, // ########

	// @17 '2' (4 pixels wide)
	0x43, //  #    ##
	0x85, // #    # #
	0x89, // #   #  #
	0x71, //  ###   #

	// @21 '3' (4 pixels wide)
	0x81, // #      #
	0x91, // #  #   #
	0x91, // #  #   #
	0x6E, //  ## ### 

	// @25 '4' (6 pixels wide)
	0x0C, //     ##  
	0x14, //    # #  
	0x24, //   #  #  
	0xC4, // ##   #  
	0xFF, // ########
	0x04, //      #  

	// @31 '5' (4 pixels wide)
	0xF1, // ####   #
	0x91, // #  #   #
	0x91, // #  #   #
	0x8E, // #   ### 

	// @35 '6' (5 pixels wide)
	0x3E, //   ##### 
	0x51, //  # #   #
	0x91, // #  #   #
	0x91, // #  #   #
	0x8E, // #   ### 

	// @40 '7' (4 pixels wide)
	0x80, // #       
	0x87, // #    ###
	0xB8, // # ###   
	0xC0, // ##      

	// @44 '8' (5 pixels wide)
	0x6E, //  ## ### 
	0x91, // #  #   #
	0x91, // #  #   #
	0x91, // #  #   #
	0x6E, //  ## ### 

	// @49 '9' (5 pixels wide)
	0x71, //  ###   #
	0x89, // #   #  #
	0x89, // #   #  #
	0x8A, // #   # # 
	0x7C, //  #####  

	// @54 ':' (1 pixels wide)
	0xCC, // ##  ##

	// @55 'C' (5 pixels wide)
	0x3C, //   ####  
	0x42, //  #    # 
	0x81, // #      #
	0x81, // #      #
	0x81, // #      #

	// @60 'F' (4 pixels wide)
	0xFF, // ########
	0x90, // #  #    
	0x90, // #  #    
	0x90, // #  #    

	// @64 '`' (2 pixels wide)
	0x80, // # 
	0x40, //  #
};

// Character descriptors for Segoe UI 8pt
// { [Char width in bits], [Char height in bits], [Offset into segoeUI_8ptCharBitmaps in bytes] }
const FONT_CHAR_INFO PROGMEM segoeUI_8ptDescriptors[] = 
{
	{2, 13, 0}, 		// ' ' 
	{0, 0, 0}, 		// '!' 
	{0, 0, 0}, 		// '"' 
	{0, 0, 0}, 		// '#' 
	{0, 0, 0}, 		// '$' 
	{0, 0, 0}, 		// '%' 
	{0, 0, 0}, 		// '&' 
	{0, 0, 0}, 		// ''' 
	{0, 0, 0}, 		// '(' 
	{0, 0, 0}, 		// ')' 
	{0, 0, 0}, 		// '*' 
	{0, 0, 0}, 		// '+' 
	{0, 0, 0}, 		// ',' 
	{3, 8, 4}, 		// '-' 
	{1, 8, 7}, 		// '.' 
	{0, 0, 0}, 		// '/' 
	{6, 8, 8}, 		// '0' 
	{3, 8, 14}, 		// '1' 
	{4, 8, 17}, 		// '2' 
	{4, 8, 21}, 		// '3' 
	{6, 8, 25}, 		// '4' 
	{4, 8, 31}, 		// '5' 
	{5, 8, 35}, 		// '6' 
	{4, 8, 40}, 		// '7' 
	{5, 8, 44}, 		// '8' 
	{5, 8, 49}, 		// '9' 
	{1, 6, 54}, 		// ':' 
	{0, 0, 0}, 		// ';' 
	{0, 0, 0}, 		// '<' 
	{0, 0, 0}, 		// '=' 
	{0, 0, 0}, 		// '>' 
	{0, 0, 0}, 		// '?' 
	{0, 0, 0}, 		// '@' 
	{0, 0, 0}, 		// 'A' 
	{0, 0, 0}, 		// 'B' 
	{5, 8, 55}, 		// 'C' 
	{0, 0, 0}, 		// 'D' 
	{0, 0, 0}, 		// 'E' 
	{4, 8, 60}, 		// 'F' 
	{0, 0, 0}, 		// 'G' 
	{0, 0, 0}, 		// 'H' 
	{0, 0, 0}, 		// 'I' 
	{0, 0, 0}, 		// 'J' 
	{0, 0, 0}, 		// 'K' 
	{0, 0, 0}, 		// 'L' 
	{0, 0, 0}, 		// 'M' 
	{0, 0, 0}, 		// 'N' 
	{0, 0, 0}, 		// 'O' 
	{0, 0, 0}, 		// 'P' 
	{0, 0, 0}, 		// 'Q' 
	{0, 0, 0}, 		// 'R' 
	{0, 0, 0}, 		// 'S' 
	{0, 0, 0}, 		// 'T' 
	{0, 0, 0}, 		// 'U' 
	{0, 0, 0}, 		// 'V' 
	{0, 0, 0}, 		// 'W' 
	{0, 0, 0}, 		// 'X' 
	{0, 0, 0}, 		// 'Y' 
	{0, 0, 0}, 		// 'Z' 
	{0, 0, 0}, 		// '[' 
	{0, 0, 0}, 		// '\' 
	{0, 0, 0}, 		// ']' 
	{0, 0, 0}, 		// '^' 
	{0, 0, 0}, 		// '_' 
	{2, 2, 64}, 		// '`' 
};

// Font information for Segoe UI 8pt
const FONT_INFO segoeUI_8ptFontInfo =
{
	13, //  Character height
	' ', //  Start character
	'`', //  End character
	1,    //  Width, in pixels, of space character
	segoeUI_8ptDescriptors, //  Character descriptor array
	segoeUI_8ptBitmaps, //  Character bitmap array
};



// Character bitmaps for Segoe UI 8pt
const uint8_t PROGMEM segoeUI_Italic_8ptBitmaps [] = 
{
	// @0 ' ' (2 pixels wide)
	0x00, 0x00, //              
	0x00, 0x00, //              

	// @4 '-' (3 pixels wide)
	0x08, // #
	0x08, // #
	0x08, // #

	// @7 '.' (1 pixels wide)
	0x3, // ##

	// @8 '0' (6 pixels wide)
	0x0C, //     ##  
	0x33, //   ##  ##
	0xC1, // ##     #
	0x81, // #      #
	0x86, // #    ## 
	0x78, //  ####   

	// @14 '1' (3 pixels wide)
	0x41, //  #     #
	0x7E, //  ###### 
	0xC0, // ##      

	// @17 '2' (6 pixels wide)
	0x03, //       ##
	0x05, //      # #
	0x85, // #    # #
	0x89, // #   #  #
	0x91, // #  #   #
	0x60, //  ##     

	// @23 '3' (6 pixels wide)
	0x01, //        #
	0x01, //        #
	0x91, // #  #   #
	0x91, // #  #   #
	0xAE, // # # ### 
	0x40, //  #      

	// @29 '4' (6 pixels wide)
	0x04, //      #  
	0x0C, //     ##  
	0x14, //    # #  
	0x67, //  ##  ###
	0xFC, // ######  
	0x84, // #    #  

	// @35 '5' (6 pixels wide)
	0x01, //        #
	0x31, //   ##   #
	0xD1, // ## #   #
	0x91, // #  #   #
	0x8E, // #   ### 
	0x80, // #       

	// @41 '6' (5 pixels wide)
	0x3F, //   ######
	0x51, //  # #   #
	0x91, // #  #   #
	0x9E, // #  #### 
	0x80, // #       

	// @46 '7' (5 pixels wide)
	0x01, //        #
	0x86, // #    ## 
	0x98, // #  ##   
	0xA0, // # #     
	0xC0, // ##      

	// @51 '8' (6 pixels wide)
	0x06, //      ## 
	0x2B, //   # # ##
	0xD1, // ## #   #
	0x91, // #  #   #
	0x9E, // #  #### 
	0x60, //  ##     

	// @57 '9' (6 pixels wide)
	0x01, //        #
	0x71, //  ###   #
	0x89, // #   #  #
	0x8B, // #   # ##
	0x96, // #  # ## 
	0x78, //  ####   

	// @63 ':' (2 pixels wide)
	0x0C, //     ##
	0xC0, // ##    

	// @65 'C' (6 pixels wide)
	0x3E, //   ##### 
	0x43, //  #    ##
	0x81, // #      #
	0x81, // #      #
	0x81, // #      #
	0x80, // #       

	// @71 'F' (6 pixels wide)
	0x01, //        #
	0x1E, //    #### 
	0xE8, // ### #   
	0x88, // #   #   
	0x88, // #   #   
	0x80, // #       

	// @77 '`' (1 pixels wide)
	0x80, // #
};

// Character descriptors for Segoe UI 8pt
// { [Char width in bits], [Char height in bits], [Offset into segoeUI_8ptCharBitmaps in bytes] }
const FONT_CHAR_INFO PROGMEM segoeUI_Italic_8ptDescriptors[] = 
{
	{2, 13, 0}, 		// ' ' 
	{0, 0, 0}, 		// '!' 
	{0, 0, 0}, 		// '"' 
	{0, 0, 0}, 		// '#' 
	{0, 0, 0}, 		// '$' 
	{0, 0, 0}, 		// '%' 
	{0, 0, 0}, 		// '&' 
	{0, 0, 0}, 		// ''' 
	{0, 0, 0}, 		// '(' 
	{0, 0, 0}, 		// ')' 
	{0, 0, 0}, 		// '*' 
	{0, 0, 0}, 		// '+' 
	{0, 0, 0}, 		// ',' 
	{3, 8, 4}, 		// '-' 
	{1, 8, 7}, 		// '.' 
	{0, 0, 0}, 		// '/' 
	{6, 8, 8}, 		// '0' 
	{3, 8, 14}, 		// '1' 
	{6, 8, 17}, 		// '2' 
	{6, 8, 23}, 		// '3' 
	{6, 8, 29}, 		// '4' 
	{6, 8, 35}, 		// '5' 
	{5, 8, 41}, 		// '6' 
	{5, 8, 46}, 		// '7' 
	{6, 8, 51}, 		// '8' 
	{6, 8, 57}, 		// '9' 
	{2, 6, 63}, 		// ':' 
	{0, 0, 0}, 		// ';' 
	{0, 0, 0}, 		// '<' 
	{0, 0, 0}, 		// '=' 
	{0, 0, 0}, 		// '>' 
	{0, 0, 0}, 		// '?' 
	{0, 0, 0}, 		// '@' 
	{0, 0, 0}, 		// 'A' 
	{0, 0, 0}, 		// 'B' 
	{6, 8, 65}, 		// 'C' 
	{0, 0, 0}, 		// 'D' 
	{0, 0, 0}, 		// 'E' 
	{6, 8, 71}, 		// 'F' 
	{0, 0, 0}, 		// 'G' 
	{0, 0, 0}, 		// 'H' 
	{0, 0, 0}, 		// 'I' 
	{0, 0, 0}, 		// 'J' 
	{0, 0, 0}, 		// 'K' 
	{0, 0, 0}, 		// 'L' 
	{0, 0, 0}, 		// 'M' 
	{0, 0, 0}, 		// 'N' 
	{0, 0, 0}, 		// 'O' 
	{0, 0, 0}, 		// 'P' 
	{0, 0, 0}, 		// 'Q' 
	{0, 0, 0}, 		// 'R' 
	{0, 0, 0}, 		// 'S' 
	{0, 0, 0}, 		// 'T' 
	{0, 0, 0}, 		// 'U' 
	{0, 0, 0}, 		// 'V' 
	{0, 0, 0}, 		// 'W' 
	{0, 0, 0}, 		// 'X' 
	{0, 0, 0}, 		// 'Y' 
	{0, 0, 0}, 		// 'Z' 
	{0, 0, 0}, 		// '[' 
	{0, 0, 0}, 		// '\' 
	{0, 0, 0}, 		// ']' 
	{0, 0, 0}, 		// '^' 
	{0, 0, 0}, 		// '_' 
	{1, 1, 77}, 		// '`' 
};

//
// Font information for Segoe UI 8pt
const FONT_INFO segoeUI_Italic_8ptFontInfo =
{
	13, //  Character height
	' ', //  Start character
	'`', //  End character
	1,    //  Width, in pixels, of space character
	segoeUI_Italic_8ptDescriptors, //  Character descriptor array
	segoeUI_Italic_8ptBitmaps, //  Character bitmap array
};


// 
//  Font data for Segoe UI 7pt
// 

// Character bitmaps for Segoe UI 7pt
const uint8_t PROGMEM segoeUI_7ptBitmaps [] = 
{
	// @0 ' ' (2 pixels wide)
	0x00, 0x00, //             
	0x00, 0x00, //             

	// @4 '-' (2 pixels wide)
	0x08, // #
	0x08, // #

	// @6 '.' (1 pixels wide)
	0x02, // #

	// @7 '0' (5 pixels wide)
	0x7C, //  ##### 
	0x82, // #     #
	0x82, // #     #
	0x82, // #     #
	0x7C, //  ##### 

	// @12 '1' (2 pixels wide)
	0x40, //  #     
	0xFE, // #######

	// @14 '2' (4 pixels wide)
	0x46, //  #   ##
	0x8A, // #   # #
	0x92, // #  #  #
	0x62, //  ##   #

	// @18 '3' (4 pixels wide)
	0x82, // #     #
	0x92, // #  #  #
	0x92, // #  #  #
	0x6C, //  ## ## 

	// @22 '4' (5 pixels wide)
	0x0C, //     ## 
	0x34, //   ## # 
	0x44, //  #   # 
	0xFE, // #######
	0x04, //      # 

	// @27 '5' (4 pixels wide)
	0xF2, // ####  #
	0x92, // #  #  #
	0x92, // #  #  #
	0x8C, // #   ## 

	// @31 '6' (5 pixels wide)
	0x3C, //   #### 
	0x62, //  ##   #
	0xA2, // # #   #
	0xA2, // # #   #
	0x9C, // #  ### 

	// @36 '7' (4 pixels wide)
	0x80, // #      
	0x86, // #    ##
	0xB8, // # ###  
	0xC0, // ##     

	// @40 '8' (5 pixels wide)
	0x6C, //  ## ## 
	0x92, // #  #  #
	0x92, // #  #  #
	0x92, // #  #  #
	0x6C, //  ## ## 

	// @45 '9' (5 pixels wide)
	0x72, //  ###  #
	0x8A, // #   # #
	0x8A, // #   # #
	0x8E, // #   ###
	0x78, //  ####  

	// @50 ':' (1 pixels wide)
	0x88, // #   #

	// @51 'C' (5 pixels wide)
	0x3C, //   #### 
	0x46, //  #   ##
	0x82, // #     #
	0x82, // #     #
	0x82, // #     #

	// @56 'F' (3 pixels wide)
	0xFE, // #######
	0x90, // #  #   
	0x90, // #  #   

		// @59 'V' (6 pixels wide)
	0xC0, // ##     
	0x38, //   ###  
	0x06, //      ##
	0x0E, //     ###
	0x70, //  ###   
	0x80, // #      

	// @65 '`' (2 pixels wide)
	0x80, // # 
	0x40, //  #
};

// Character descriptors for Segoe UI 7pt
// { [Char width in bits], [Char height in bits], [Offset into segoeUI_7ptCharBitmaps in bytes] }
const FONT_CHAR_INFO PROGMEM segoeUI_7ptDescriptors[] = 
{
	{2, 12, 0}, 		// ' ' 
	{0, 0, 0}, 		// '!' 
	{0, 0, 0}, 		// '"' 
	{0, 0, 0}, 		// '#' 
	{0, 0, 0}, 		// '$' 
	{0, 0, 0}, 		// '%' 
	{0, 0, 0}, 		// '&' 
	{0, 0, 0}, 		// ''' 
	{0, 0, 0}, 		// '(' 
	{0, 0, 0}, 		// ')' 
	{0, 0, 0}, 		// '*' 
	{0, 0, 0}, 		// '+' 
	{0, 0, 0}, 		// ',' 
	{2, 5, 4}, 		// '-' 
	{1, 7, 6}, 		// '.' 
	{0, 0, 0}, 		// '/' 
	{5, 7, 7}, 		// '0' 
	{2, 7, 12}, 		// '1' 
	{4, 7, 14}, 		// '2' 
	{4, 7, 18}, 		// '3' 
	{5, 7, 22}, 		// '4' 
	{4, 7, 27}, 		// '5' 
	{5, 7, 31}, 		// '6' 
	{4, 7, 36}, 		// '7' 
	{5, 7, 40}, 		// '8' 
	{5, 7, 45}, 		// '9' 
	{1, 5, 50}, 		// ':' 
	{0, 0, 0}, 		// ';' 
	{0, 0, 0}, 		// '<' 
	{0, 0, 0}, 		// '=' 
	{0, 0, 0}, 		// '>' 
	{0, 0, 0}, 		// '?' 
	{0, 0, 0}, 		// '@' 
	{0, 0, 0}, 		// 'A' 
	{0, 0, 0}, 		// 'B' 
	{5, 7, 51}, 		// 'C' 
	{0, 0, 0}, 		// 'D' 
	{0, 0, 0}, 		// 'E' 
	{3, 7, 56}, 		// 'F' 
	{0, 0, 0}, 		// 'G' 
	{0, 0, 0}, 		// 'H' 
	{0, 0, 0}, 		// 'I' 
	{0, 0, 0}, 		// 'J' 
	{0, 0, 0}, 		// 'K' 
	{0, 0, 0}, 		// 'L' 
	{0, 0, 0}, 		// 'M' 
	{0, 0, 0}, 		// 'N' 
	{0, 0, 0}, 		// 'O' 
	{0, 0, 0}, 		// 'P' 
	{0, 0, 0}, 		// 'Q' 
	{0, 0, 0}, 		// 'R' 
	{0, 0, 0}, 		// 'S' 
	{0, 0, 0}, 		// 'T' 
	{0, 0, 0}, 		// 'U' 
	{6, 7, 59}, 		// 'V' 
	{0, 0, 0}, 		// 'W' 
	{0, 0, 0}, 		// 'X' 
	{0, 0, 0}, 		// 'Y' 
	{0, 0, 0}, 		// 'Z' 
	{0, 0, 0}, 		// '[' 
	{0, 0, 0}, 		// '\' 
	{0, 0, 0}, 		// ']' 
	{0, 0, 0}, 		// '^' 
	{0, 0, 0}, 		// '_' 
	{2, 2, 65}, 		// '`' 
};

// Font information for Segoe UI 7pt
const FONT_INFO segoeUI_7ptFontInfo =
{
	12, //  Character height
	' ', //  Start character
	'`', //  End character
  1,
	segoeUI_7ptDescriptors, //  Character descriptor array
	segoeUI_7ptBitmaps, //  Character bitmap array
};

// 
//  Font data for Segoe UI 7pt
// 

// Character bitmaps for Segoe UI 7pt
const uint8_t PROGMEM segoeUI_Italic_7ptBitmaps [] = 
{
	// @0 ' ' (2 pixels wide)
	0x00, 0x00, //             
	0x00, 0x00, //             

	// @4 '-' (2 pixels wide)
	0x08, // #
	0x08, // #

	// @6 '.' (1 pixels wide)
	0x02, // #

	// @7 '0' (5 pixels wide)
	0x1C, //    ### 
	0x62, //  ##   #
	0x82, // #     #
	0x8C, // #   ## 
	0x70, //  ###   

	// @12 '1' (2 pixels wide)
	0x5E, //  # ####
	0xE0, // ###    

	// @14 '2' (5 pixels wide)
	0x06, //      ##
	0x46, //  #   ##
	0x8A, // #   # #
	0x92, // #  #  #
	0x60, //  ##    

	// @19 '3' (5 pixels wide)
	0x02, //       #
	0x12, //    #  #
	0x92, // #  #  #
	0xAC, // # # ## 
	0x40, //  #     

	// @24 '4' (5 pixels wide)
	0x18, //    ## 
	0x28, //   # # 
	0x48, //  #  # 
	0xFC, // ######
	0x88, // #   # 

	// @29 '5' (5 pixels wide)
	0x06, //      ##
	0x72, //  ###  #
	0x92, // #  #  #
	0x9C, // #  ### 
	0x80, // #      

	// @34 '6' (4 pixels wide)
  0x1C, //    ###
	0x72, //  ###  #
	0x92, // #  #  #
	0x9C, // #  ###
	0x80, // #      

	// @39 '7' (4 pixels wide)
	0x86, // #    ##
	0x98, // #  ##  
	0xA0, // # #    
	0xC0, // ##     

	// @43 '8' (5 pixels wide)
	0x0C, //     ## 
	0x72, //  ###  #
	0x92, // #  #  #
	0x9C, // #  ### 
	0x60, //  ##    

	// @48 '9' (5 pixels wide)
	0x02, //       #
	0x72, //  ###  #
	0x92, // #  #  #
	0x9C, // #  ### 
	0x70, //  ###   

	// @53 ':' (2 pixels wide)
	0x08, //     #
	0x80, // #    

	// @55 'C' (5 pixels wide)
	0x7C, //  ##### 
	0xC2, // ##    #
	0x82, // #     #
	0x82, // #     #
	0x80, // #      

	// @60 'F' (5 pixels wide)
	0x06, //      ##
	0x78, //  ####  
	0x90, // #  #   
	0x90, // #  #   
	0x80, // #      

	// @65 'V' (5 pixels wide)
	0xFC, // ###### 
	0x06, //      ##
	0x18, //    ##  
	0x20, //   #    
	0xC0, // ##     

	// @70 '`' (2 pixels wide)
	0x80, // # 
	0x40, //  #
};

// Character descriptors for Segoe UI 7pt
// { [Char width in bits], [Char height in bits], [Offset into segoeUI_7ptCharBitmaps in bytes] }
const FONT_CHAR_INFO PROGMEM segoeUI_Italic_7ptDescriptors[] = 
{
	{2, 12, 0}, 		// ' ' 
	{0, 0, 0}, 		// '!' 
	{0, 0, 0}, 		// '"' 
	{0, 0, 0}, 		// '#' 
	{0, 0, 0}, 		// '$' 
	{0, 0, 0}, 		// '%' 
	{0, 0, 0}, 		// '&' 
	{0, 0, 0}, 		// ''' 
	{0, 0, 0}, 		// '(' 
	{0, 0, 0}, 		// ')' 
	{0, 0, 0}, 		// '*' 
	{0, 0, 0}, 		// '+' 
	{0, 0, 0}, 		// ',' 
	{2, 5, 4}, 		// '-' 
	{1, 7, 6}, 		// '.' 
	{0, 0, 0}, 		// '/' 
	{5, 7, 7}, 		// '0' 
	{2, 7, 12}, 		// '1' 
	{5, 7, 14}, 		// '2' 
	{5, 7, 19}, 		// '3' 
	{5, 6, 24}, 		// '4' 
	{5, 7, 29}, 		// '5' 
	{4, 7, 34}, 		// '6' 
	{4, 7, 39}, 		// '7' 
	{5, 7, 43}, 		// '8' 
	{5, 7, 48}, 		// '9' 
	{2, 5, 53}, 		// ':' 
	{0, 0, 0}, 		// ';' 
	{0, 0, 0}, 		// '<' 
	{0, 0, 0}, 		// '=' 
	{0, 0, 0}, 		// '>' 
	{0, 0, 0}, 		// '?' 
	{0, 0, 0}, 		// '@' 
	{0, 0, 0}, 		// 'A' 
	{0, 0, 0}, 		// 'B' 
	{5, 7, 55}, 		// 'C' 
	{0, 0, 0}, 		// 'D' 
	{0, 0, 0}, 		// 'E' 
	{5, 7, 60}, 		// 'F' 
	{0, 0, 0}, 		// 'G' 
	{0, 0, 0}, 		// 'H' 
	{0, 0, 0}, 		// 'I' 
	{0, 0, 0}, 		// 'J' 
	{0, 0, 0}, 		// 'K' 
	{0, 0, 0}, 		// 'L' 
	{0, 0, 0}, 		// 'M' 
	{0, 0, 0}, 		// 'N' 
	{0, 0, 0}, 		// 'O' 
	{0, 0, 0}, 		// 'P' 
	{0, 0, 0}, 		// 'Q' 
	{0, 0, 0}, 		// 'R' 
	{0, 0, 0}, 		// 'S' 
	{0, 0, 0}, 		// 'T' 
	{0, 0, 0}, 		// 'U' 
	{5, 7, 65}, 		// 'V' 
	{0, 0, 0}, 		// 'W' 
	{0, 0, 0}, 		// 'X' 
	{0, 0, 0}, 		// 'Y' 
	{0, 0, 0}, 		// 'Z' 
	{0, 0, 0}, 		// '[' 
	{0, 0, 0}, 		// '\' 
	{0, 0, 0}, 		// ']' 
	{0, 0, 0}, 		// '^' 
	{0, 0, 0}, 		// '_' 
	{2, 2, 70},		// '`' 
};

// Font information for Segoe UI 7pt
const FONT_INFO segoeUI_Italic_7ptFontInfo =
{
	12, //  Character height
	' ', //  Start character
	'`', //  End character
  1,
	segoeUI_Italic_7ptDescriptors, //  Character descriptor array
	segoeUI_Italic_7ptBitmaps, //  Character bitmap array
};

