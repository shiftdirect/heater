#include "OCRfont.h"
// 
//  Font data for OCR A Extended 8pt
// 
//  Generated by The Dot Factory:
//  http://www.eran.io/the-dot-factory-an-lcd-font-and-image-generator/
// 
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

// Character bitmaps for OCR A Extended 8pt
const uint8_t oCRAExtended_8ptBitmaps[] PROGMEM = 
{
	// @0 ' ' (2 pixels wide)
	0x00, 0x00, //             
	0x00, 0x00, //             

	// @4 ',' (3 pixels wide)
	0x80, // #  
	0x80, // #  
	0xE0, // ###

	// @7 '.' (1 pixels wide)
	0x80, // #

	// @8 '/' (5 pixels wide)
	0x02, //       #
	0x0C, //     ## 
	0x10, //    #   
	0x60, //  ##    
	0x80, // #      

	// @13 '0' (5 pixels wide)
	0xFE, // #######
	0x82, // #     #
	0x82, // #     #
	0x82, // #     #
	0xFE, // #######

	// @18 '1' (5 pixels wide)
	0x82, // #     #
	0x82, // #     #
	0xFE, // #######
	0x02, //       #
	0x0E, //     ###

	// @23 '2' (5 pixels wide)
	0x9E, // #  ####
	0x92, // #  #  #
	0x92, // #  #  #
	0x92, // #  #  #
	0xF2, // ####  #

	// @28 '3' (5 pixels wide)
	0x82, // #     #
	0x92, // #  #  #
	0x92, // #  #  #
	0x92, // #  #  #
	0xEE, // ### ###

	// @33 '4' (4 pixels wide)
	0xF8, // #####  
	0x08, //     #  
	0x08, //     #  
	0x7E, //  ######

	// @37 '5' (5 pixels wide)
	0x02, //       #
	0xF2, // ####  #
	0x92, // #  #  #
	0x92, // #  #  #
	0x9E, // #  ####

	// @42 '6' (5 pixels wide)
	0xFE, // #######
	0x0A, //     # #
	0x0A, //     # #
	0x0A, //     # #
	0x0E, //     ###

	// @47 '7' (5 pixels wide)
	0x80, // #      
	0x80, // #      
	0x8E, // #   ###
	0x90, // #  #   
	0xE0, // ###    

	// @52 '8' (5 pixels wide)
	0x1E, //    ####
	0xF2, // ####  #
	0x92, // #  #  #
	0xF2, // ####  #
	0x1E, //    ####

	// @57 '9' (5 pixels wide)
	0xE0, // ###    
	0xA0, // # #    
	0xA0, // # #    
	0xA0, // # #    
	0xFE, // #######

	// @62 ':' (1 pixels wide)
	0xA0, // # #

	// @63 'A' (5 pixels wide)
	0x06, //      ##
	0x3C, //   #### 
	0xC4, // ##   # 
	0x3C, //   #### 
	0x06, //      ##

	// @68 'B' (5 pixels wide)
	0xFE, // #######
	0x92, // #  #  #
	0x92, // #  #  #
	0x92, // #  #  #
	0x6C, //  ## ## 

	// @73 'C' (5 pixels wide)
	0x38, //   ###  
	0x44, //  #   # 
	0x82, // #     #
	0x82, // #     #
	0x82, // #     #

	// @78 'D' (5 pixels wide)
	0x82, // #     #
	0xFE, // #######
	0x82, // #     #
	0x44, //  #   # 
	0x38, //   ###  

	// @83 'E' (5 pixels wide)
	0xFE, // #######
	0x92, // #  #  #
	0x92, // #  #  #
	0x82, // #     #
	0x82, // #     #

	// @88 'F' (5 pixels wide)
	0xFE, // #######
	0xA0, // # #    
	0xA0, // # #    
	0xA0, // # #    
	0x80, // #      

	// @93 'G' (5 pixels wide)
	0x3E, //   #####
	0x42, //  #    #
	0x8A, // #   # #
	0x8A, // #   # #
	0x8E, // #   ###

	// @98 'H' (5 pixels wide)
	0xFE, // #######
	0x10, //    #   
	0x10, //    #   
	0x10, //    #   
	0xFE, // #######

	// @103 'I' (5 pixels wide)
	0x82, // #     #
	0x82, // #     #
	0xFE, // #######
	0x82, // #     #
	0x82, // #     #

	// @108 'J' (4 pixels wide)
	0x0C, //     ## 
	0x02, //       #
	0x02, //       #
	0xFC, // ###### 

	// @112 'K' (5 pixels wide)
	0xFE, // #######
	0x10, //    #   
	0x28, //   # #  
	0x44, //  #   # 
	0x82, // #     #

	// @117 'L' (5 pixels wide)
	0xFE, // #######
	0x02, //       #
	0x02, //       #
	0x02, //       #
	0x02, //       #

	// @122 'M' (5 pixels wide)
	0xFE, // #######
	0xC0, // ##     
	0x20, //   #    
	0xC0, // ##     
	0xFE, // #######

	// @127 'N' (5 pixels wide)
	0xFE, // #######
	0x60, //  ##    
	0x10, //    #   
	0x0C, //     ## 
	0xFE, // #######

	// @132 'O' (5 pixels wide)
	0x38, //   ###  
	0x44, //  #   # 
	0x82, // #     #
	0x44, //  #   # 
	0x38, //   ###  

	// @137 'P' (5 pixels wide)
	0xFE, // #######
	0x90, // #  #   
	0x90, // #  #   
	0x90, // #  #   
	0x60, //  ##    

	// @142 'Q' (5 pixels wide)
	0x3E, //   #####
	0x42, //  #    #
	0x4C, //  #  ## 
	0x8E, // #   ###
	0xFA, // ##### #

	// @147 'R' (5 pixels wide)
	0xFE, // #######
	0xA0, // # #    
	0xB0, // # ##   
	0xAC, // # # ## 
	0x42, //  #    #

	// @152 'S' (5 pixels wide)
	0x44, //  #   # 
	0xA2, // # #   #
	0x92, // #  #  #
	0x8A, // #   # #
	0x46, //  #   ##

	// @157 'T' (5 pixels wide)
	0xC0, // ##     
	0x80, // #      
	0xFE, // #######
	0x80, // #      
	0xC0, // ##     

	// @162 'U' (5 pixels wide)
	0xFC, // ###### 
	0x02, //       #
	0x02, //       #
	0x02, //       #
	0xFC, // ###### 

	// @167 'V' (5 pixels wide)
	0xE0, // ###    
	0x18, //    ##  
	0x06, //      ##
	0x18, //    ##  
	0xE0, // ###    

	// @172 'W' (5 pixels wide)
	0xFC, // ###### 
	0x02, //       #
	0x3C, //   #### 
	0x02, //       #
	0xFC, // ###### 

	// @177 'X' (5 pixels wide)
	0x82, // #     #
	0x6C, //  ## ## 
	0x10, //    #   
	0x6C, //  ## ## 
	0x82, // #     #

	// @182 'Y' (5 pixels wide)
	0xC0, // ##     
	0x20, //   #    
	0x1E, //    ####
	0x20, //   #    
	0xC0, // ##     

	// @187 'Z' (5 pixels wide)
	0x82, // #     #
	0x8E, // #   ###
	0x92, // #  #  #
	0xE2, // ###   #
	0x82, // #     #

	// @192 '\' (5 pixels wide)
	0x80, // #      
	0x60, //  ##    
	0x10, //    #   
	0x0C, //     ## 
	0x02, //       #

	// @197 '`' (3 pixels wide)
	0x80, // # 
	0x80, // # 
	0x40, //  #

	// @200 'a' (5 pixels wide)
	0x10, //    # 
	0xA8, // # # #
	0xA8, // # # #
	0xA8, // # # #
	0x78, //  ####

	// @205 'b' (5 pixels wide)
	0xFE, // #######
	0x14, //    # # 
	0x22, //   #   #
	0x22, //   #   #
	0x1C, //    ### 

	// @210 'c' (5 pixels wide)
	0x70, //  ### 
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #

	// @215 'd' (5 pixels wide)
	0x1C, //    ### 
	0x22, //   #   #
	0x22, //   #   #
	0x14, //    # # 
	0xFE, // #######

	// @220 'e' (5 pixels wide)
	0x70, //  ### 
	0xA8, // # # #
	0xA8, // # # #
	0xA8, // # # #
	0x68, //  ## #

	// @225 'f' (4 pixels wide)
	0x20, //   #    
	0x7E, //  ######
	0xA0, // # #    
	0x80, // #      

	// @229 'g' (5 pixels wide)
	0x72, //  ###  #
	0x8A, // #   # #
	0x8A, // #   # #
	0x52, //  # #  #
	0xFC, // ###### 

	// @234 'h' (5 pixels wide)
	0xFE, // #######
	0x10, //    #   
	0x20, //   #    
	0x20, //   #    
	0x1E, //    ####

	// @239 'i' (3 pixels wide)
	0x22, //   #   #
	0xBE, // # #####
	0x02, //       #

	// @242 'j' (4 pixels wide)
	0x01, 0x00, //        # 
	0x20, 0x80, //   #     #
	0x20, 0x80, //   #     #
	0xBF, 0x00, // # ###### 

	// @250 'k' (5 pixels wide)
	0xFE, // #######
	0x08, //     #  
	0x14, //    # # 
	0x14, //    # # 
	0x22, //   #   #

	// @255 'l' (3 pixels wide)
	0x82, // #     #
	0xFE, // #######
	0x02, //       #

	// @258 'm' (5 pixels wide)
	0xF8, // #####
	0x80, // #    
	0x78, //  ####
	0x80, // #    
	0x78, //  ####

	// @263 'n' (5 pixels wide)
	0xF8, // #####
	0x40, //  #   
	0x80, // #    
	0x80, // #    
	0x78, //  ####

	// @268 'o' (5 pixels wide)
	0x70, //  ### 
	0x88, // #   #
	0x88, // #   #
	0x88, // #   #
	0x70, //  ### 

	// @273 'p' (5 pixels wide)
	0xFE, // #######
	0x50, //  # #   
	0x88, // #   #  
	0x88, // #   #  
	0x70, //  ###   

	// @278 'q' (5 pixels wide)
	0x70, //  ###   
	0x88, // #   #  
	0x88, // #   #  
	0x50, //  # #   
	0xFE, // #######

	// @283 'r' (5 pixels wide)
	0xF8, // #####
	0x40, //  #   
	0x80, // #    
	0x80, // #    
	0x40, //  #   

	// @288 's' (5 pixels wide)
	0x48, //  #  #
	0xA8, // # # #
	0xA8, // # # #
	0xA8, // # # #
	0x90, // #  # 

	// @293 't' (5 pixels wide)
	0x20, //   #    
	0xFC, // ###### 
	0x22, //   #   #
	0x22, //   #   #
	0x04, //      # 

	// @298 'u' (5 pixels wide)
	0xF0, // #### 
	0x08, //     #
	0x08, //     #
	0x10, //    # 
	0xF8, // #####

	// @303 'v' (5 pixels wide)
	0xC0, // ##   
	0x30, //   ## 
	0x08, //     #
	0x30, //   ## 
	0xC0, // ##   

	// @308 'w' (5 pixels wide)
	0xF0, // #### 
	0x08, //     #
	0x70, //  ### 
	0x08, //     #
	0xF0, // #### 

	// @313 'x' (5 pixels wide)
	0x88, // #   #
	0x50, //  # # 
	0x20, //   #  
	0x50, //  # # 
	0x88, // #   #

	// @318 'y' (4 pixels wide)
	0xE2, // ###   #
	0x1E, //    ####
	0x18, //    ##  
	0xE0, // ###    

	// @322 'z' (5 pixels wide)
	0x88, // #   #
	0x98, // #  ##
	0xA8, // # # #
	0xC8, // ##  #
	0x88, // #   #
};

// Character descriptors for OCR A Extended 8pt
// { [Char width in bits], [Offset into oCRAExtended_8ptCharBitmaps in bytes] }
const FONT_CHAR_INFO oCRAExtended_8ptDescriptors[] PROGMEM = 
{
	{1, 7, 0}, 		//   
	{0, 0, 0}, 		// ! 
	{0, 0, 0}, 		// " 
	{0, 0, 0}, 		// # 
	{0, 0, 0}, 		// $ 
	{0, 0, 0}, 		// % 
	{0, 0, 0}, 		// & 
	{0, 0, 0}, 		// ' 
	{0, 0, 0}, 		// ( 
	{0, 0, 0}, 		// ) 
	{0, 0, 0}, 		// * 
	{0, 0, 0}, 		// + 
	{3, 3, 4}, 		// , 
	{0, 0, 0}, 		// - 
	{1, 1, 7}, 		// . 
	{5, 7, 8}, 		// / 
	{5, 7, 13}, 		// 0 
	{5, 7, 18}, 		// 1 
	{5, 7, 23}, 		// 2 
	{5, 7, 28}, 		// 3 
	{4, 7, 33}, 		// 4 
	{5, 7, 37}, 		// 5 
	{5, 7, 42}, 		// 6 
	{5, 7, 47}, 		// 7 
	{5, 7, 52}, 		// 8 
	{5, 7, 57}, 		// 9 
	{1, 3, 62}, 		// : 
	{0, 0, 0}, 		// ; 
	{0, 0, 0}, 		// < 
	{0, 0, 0}, 		// = 
	{0, 0, 0}, 		// > 
	{0, 0, 0}, 		// ? 
	{0, 0, 0}, 		// @ 
	{5, 7, 63}, 		// A 
	{5, 7, 68}, 		// B 
	{5, 7, 73}, 		// C 
	{5, 7, 78}, 		// D 
	{5, 7, 83}, 		// E 
	{5, 7, 88}, 		// F 
	{5, 7, 93}, 		// G 
	{5, 7, 98}, 		// H 
	{5, 7, 103}, 		// I 
	{4, 7, 108}, 		// J 
	{5, 7, 112}, 		// K 
	{5, 7, 117}, 		// L 
	{5, 7, 122}, 		// M 
	{5, 7, 127}, 		// N 
	{5, 7, 132}, 		// O 
	{5, 7, 137}, 		// P 
	{5, 7, 142}, 		// Q 
	{5, 7, 147}, 		// R 
	{5, 7, 152}, 		// S 
	{5, 7, 157}, 		// T 
	{5, 7, 162}, 		// U 
	{5, 7, 167}, 		// V 
	{5, 7, 172}, 		// W 
	{5, 7, 177}, 		// X 
	{5, 7, 182}, 		// Y 
	{5, 7, 187}, 		// Z 
	{0, 0, 0}, 		  // [ 
	{5, 7, 192}, 		// \ .
	{0, 0, 0}, 	  	// ] 
	{0, 0, 0}, 		  // ^ 
	{0, 0, 0}, 		  // _ 
	{3, 2, 197}, 		// ` 
	{5, 5, 200}, 		// a 
	{5, 7, 205}, 		// b 
	{5, 5, 210}, 		// c 
	{5, 7, 215}, 		// d 
	{5, 5, 220}, 		// e 
	{4, 7, 225}, 		// f 
	{5, 7, 229}, 		// g 
	{5, 7, 234}, 		// h 
	{3, 7, 239}, 		// i 
	{4, 9, 242}, 		// j 
	{5, 7, 250}, 		// k 
	{3, 7, 255}, 		// l 
	{5, 5, 258}, 		// m 
	{5, 5, 263}, 		// n 
	{5, 5, 268}, 		// o 
	{5, 7, 273}, 		// p 
	{5, 7, 278}, 		// q 
	{5, 5, 283}, 		// r 
	{5, 5, 288}, 		// s 
	{5, 7, 293}, 		// t 
	{5, 5, 298}, 		// u 
	{5, 5, 303}, 		// v 
	{5, 5, 308}, 		// w 
	{5, 5, 313}, 		// x 
	{4, 7, 318}, 		// y 
	{5, 5, 322}, 		// z 
};

// Font information for OCR A Extended 8pt
const FONT_INFO oCRAExtended_8ptFontInfo =
{
	12,  //  Character height
	' ', //  Start character
	'z', //  End character
	1,   //  Width, in pixels, of space character
	oCRAExtended_8ptDescriptors, //  Character descriptor array
	oCRAExtended_8ptBitmaps, //  Character bitmap array
};
