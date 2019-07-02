// 
//  Font data for Tahoma 24pt
//
//  Generated by The Dot Factory:
//  http://www.eran.io/the-dot-factory-an-lcd-font-and-image-generator/
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Dot Factory Settings
// 
// Flip/Rotate      Padding Removal           Line Wrap         Descriptors
//   [X] Flip X       Height(Y): Tightest       (O) At column     [X] Generate descriptor array
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
//    Char Info: const FONT_CHAR_INFO PROGMEM {0}Descriptors      [ ] Generate space bitmap
//    Font Info: const FONT_INFO {0}FontInfo                      [2] pixels for space char     
//    Width:     const uint8_t {0}Width                           
//    Height:    const uint8_t {0}Height
//
/////////////////////////////////////////////////////////////////////////////////////////////////////


#include "Tahoma24.h"

// Character bitmaps for Tahoma 24pt
const uint8_t tahoma_24ptBitmaps [] PROGMEM = 
{
	// @0 ' ' (2 pixels wide)
	0x00, 0x00, 0x00, 0x00, 0x00, //                                        
	0x00, 0x00, 0x00, 0x00, 0x00, //                                        

	// @10 '.' (6 pixels wide)
	0xFC, // ######
	0xFC, // ######
	0xFC, // ######
	0xFC, // ######
	0xFC, // ######
	0xFC, // ######

	// @16 '0' (18 pixels wide)
	0x03, 0xFF, 0x80, //       ###########      
	0x0F, 0xFF, 0xE0, //     ###############    
	0x3F, 0xFF, 0xF8, //   ###################  
	0x7F, 0xFF, 0xFC, //  ##################### 
	0x7F, 0xFF, 0xFC, //  ##################### 
	0xFF, 0xFF, 0xFE, // #######################
	0xFC, 0x00, 0x7E, // ######           ######
	0xF8, 0x00, 0x3E, // #####             #####
	0xF0, 0x00, 0x1E, // ####               ####
	0xF0, 0x00, 0x1E, // ####               ####
	0xF8, 0x00, 0x3E, // #####             #####
	0xFC, 0x00, 0x7E, // ######           ######
	0xFF, 0xFF, 0xFE, // #######################
	0x7F, 0xFF, 0xFC, //  ##################### 
	0x7F, 0xFF, 0xFC, //  ##################### 
	0x3F, 0xFF, 0xF8, //   ###################  
	0x0F, 0xFF, 0xE0, //     ###############    
	0x03, 0xFF, 0x80, //       ###########      

	// @70 '1' (16 pixels wide)
	0x1E, 0x00, 0x1E, //    ####            ####
	0x1E, 0x00, 0x1E, //    ####            ####
	0x1E, 0x00, 0x1E, //    ####            ####
	0x1E, 0x00, 0x1E, //    ####            ####
	0x3E, 0x00, 0x1E, //   #####            ####
	0x7F, 0xFF, 0xFE, //  ######################
	0xFF, 0xFF, 0xFE, // #######################
	0xFF, 0xFF, 0xFE, // #######################
	0xFF, 0xFF, 0xFE, // #######################
	0xFF, 0xFF, 0xFE, // #######################
	0xFF, 0xFF, 0xFE, // #######################
	0x00, 0x00, 0x1E, //                    ####
	0x00, 0x00, 0x1E, //                    ####
	0x00, 0x00, 0x1E, //                    ####
	0x00, 0x00, 0x1E, //                    ####
	0x00, 0x00, 0x1E, //                    ####

	// @118 '2' (17 pixels wide)
	0x00, 0x00, 0x1E, //                    ####
	0x7E, 0x00, 0x3E, //  ######           #####
	0x7C, 0x00, 0x7E, //  #####           ######
	0x78, 0x00, 0x7E, //  ####            ######
	0xF8, 0x00, 0xFE, // #####           #######
	0xF0, 0x01, 0xFE, // ####           ########
	0xF0, 0x03, 0xFE, // ####          #########
	0xF0, 0x07, 0xFE, // ####         ##########
	0xF0, 0x1F, 0xDE, // ####       ####### ####
	0xF8, 0x3F, 0x9E, // #####     #######  ####
	0xFF, 0xFF, 0x1E, // ################   ####
	0xFF, 0xFE, 0x1E, // ###############    ####
	0x7F, 0xFC, 0x1E, //  #############     ####
	0x7F, 0xF8, 0x1E, //  ############      ####
	0x3F, 0xF0, 0x1E, //   ##########       ####
	0x0F, 0xC0, 0x1E, //     ######         ####
	0x00, 0x00, 0x1E, //                    ####

	// @169 '3' (17 pixels wide)
	0x00, 0x00, 0xFC, //                 ###### 
	0x7C, 0x00, 0x7C, //  #####           ##### 
	0x78, 0x00, 0x3C, //  ####             #### 
	0x78, 0x00, 0x3E, //  ####             #####
	0xF0, 0x00, 0x1E, // ####               ####
	0xF0, 0x78, 0x1E, // ####     ####      ####
	0xF0, 0x78, 0x1E, // ####     ####      ####
	0xF0, 0x78, 0x1E, // ####     ####      ####
	0xF0, 0x78, 0x1E, // ####     ####      ####
	0xF8, 0xF8, 0x3E, // #####   #####     #####
	0xFF, 0xFC, 0x3E, // ##############    #####
	0xFF, 0xFF, 0xFE, // #######################
	0x7F, 0xFF, 0xFC, //  ##################### 
	0x7F, 0xDF, 0xFC, //  ######### ########### 
	0x3F, 0x8F, 0xF8, //   #######   #########  
	0x1F, 0x0F, 0xF0, //    #####    ########   
	0x00, 0x03, 0xE0, //               #####    

	// @220 '4' (18 pixels wide)
	0x00, 0x07, 0xC0, //              #####     
	0x00, 0x1F, 0xC0, //            #######     
	0x00, 0x3F, 0xC0, //           ########     
	0x00, 0xFF, 0xC0, //         ##########     
	0x01, 0xFF, 0xC0, //        ###########     
	0x07, 0xF3, 0xC0, //      #######  ####     
	0x0F, 0xE3, 0xC0, //     #######   ####     
	0x3F, 0x83, 0xC0, //   #######     ####     
	0x7F, 0x03, 0xC0, //  #######      ####     
	0xFF, 0xFF, 0xFE, // #######################
	0xFF, 0xFF, 0xFE, // #######################
	0xFF, 0xFF, 0xFE, // #######################
	0xFF, 0xFF, 0xFE, // #######################
	0xFF, 0xFF, 0xFE, // #######################
	0xFF, 0xFF, 0xFE, // #######################
	0x00, 0x03, 0xC0, //               ####     
	0x00, 0x03, 0xC0, //               ####     
	0x00, 0x03, 0xC0, //               ####     

	// @274 '5' (17 pixels wide)
	0x00, 0x00, 0x7C, //                  ##### 
	0xFF, 0xF8, 0x3C, // #############     #### 
	0xFF, 0xF8, 0x3C, // #############     #### 
	0xFF, 0xF0, 0x3E, // ############      #####
	0xFF, 0xF0, 0x1E, // ############       ####
	0xFF, 0xF0, 0x1E, // ############       ####
	0xFF, 0xF0, 0x1E, // ############       ####
	0xF0, 0xF0, 0x1E, // ####    ####       ####
	0xF0, 0xF0, 0x1E, // ####    ####       ####
	0xF0, 0xF8, 0x3E, // ####    #####     #####
	0xF0, 0xFC, 0x7E, // ####    ######   ######
	0xF0, 0xFF, 0xFC, // ####    ############## 
	0xF0, 0xFF, 0xFC, // ####    ############## 
	0xF0, 0x7F, 0xFC, // ####     ############# 
	0xF0, 0x7F, 0xF8, // ####     ############  
	0xF0, 0x3F, 0xF0, // ####      ##########   
	0x00, 0x0F, 0xC0, //             ######     

	// @325 '6' (17 pixels wide)
	0x00, 0xFF, 0x80, //         #########      
	0x07, 0xFF, 0xE0, //      ##############    
	0x0F, 0xFF, 0xF8, //     #################  
	0x1F, 0xFF, 0xFC, //    ################### 
	0x3F, 0xFF, 0xFC, //   #################### 
	0x7F, 0xFF, 0xFE, //  ######################
	0x7F, 0x70, 0x7E, //  ####### ###     ######
	0xFC, 0x70, 0x1E, // ######   ###       ####
	0xF8, 0xF0, 0x1E, // #####   ####       ####
	0xF0, 0xF0, 0x1E, // ####    ####       ####
	0xF0, 0xF8, 0x3E, // ####    #####     #####
	0xF0, 0xFF, 0xFE, // ####    ###############
	0xF0, 0xFF, 0xFC, // ####    ############## 
	0xF0, 0x7F, 0xFC, // ####     ############# 
	0xF8, 0x7F, 0xF8, // #####    ############  
	0x00, 0x3F, 0xF0, //           ##########   
	0x00, 0x0F, 0xC0, //             ######     

	// @376 '7' (17 pixels wide)
	0xF0, 0x00, 0x00, // ####                   
	0xF0, 0x00, 0x02, // ####                  #
	0xF0, 0x00, 0x0E, // ####                ###
	0xF0, 0x00, 0x3E, // ####              #####
	0xF0, 0x00, 0xFE, // ####            #######
	0xF0, 0x03, 0xFE, // ####          #########
	0xF0, 0x0F, 0xFE, // ####        ###########
	0xF0, 0x3F, 0xFE, // ####      #############
	0xF0, 0xFF, 0xF8, // ####    #############  
	0xF3, 0xFF, 0xE0, // ####  #############    
	0xFF, 0xFF, 0x80, // #################      
	0xFF, 0xFE, 0x00, // ###############        
	0xFF, 0xF8, 0x00, // #############          
	0xFF, 0xE0, 0x00, // ###########            
	0xFF, 0x80, 0x00, // #########              
	0xFE, 0x00, 0x00, // #######                
	0xF8, 0x00, 0x00, // #####                  

	// @427 '8' (18 pixels wide)
	0x00, 0x03, 0xE0, //               #####    
	0x0F, 0x07, 0xF8, //     ####     ########  
	0x3F, 0xCF, 0xF8, //   ########  #########  
	0x7F, 0xEF, 0xFC, //  ########## ########## 
	0x7F, 0xFF, 0xFC, //  ##################### 
	0x7F, 0xFF, 0xFE, //  ######################
	0xFF, 0xFC, 0x3E, // ##############    #####
	0xF8, 0xF8, 0x1E, // #####   #####      ####
	0xF0, 0xF8, 0x1E, // ####    #####      ####
	0xF0, 0x7C, 0x1E, // ####     #####     ####
	0xF8, 0x7C, 0x1E, // #####    #####     ####
	0xFF, 0xFE, 0x3E, // ###############   #####
	0xFF, 0xFF, 0xFE, // #######################
	0x7F, 0xFF, 0xFC, //  ##################### 
	0x7F, 0xDF, 0xFC, //  ######### ########### 
	0x3F, 0x8F, 0xF8, //   #######   #########  
	0x1F, 0x0F, 0xF0, //    #####    ########   
	0x00, 0x03, 0xE0, //               #####    

	// @481 '9' (17 pixels wide)
	0x07, 0xE0, 0x00, //      ######            
	0x1F, 0xF8, 0x00, //    ##########          
	0x3F, 0xFC, 0x3E, //   ############    #####
	0x7F, 0xFC, 0x1E, //  #############     ####
	0x7F, 0xFE, 0x1E, //  ##############    ####
	0xFF, 0xFE, 0x1E, // ###############    ####
	0xF8, 0x3E, 0x1E, // #####     #####    ####
	0xF0, 0x1E, 0x1E, // ####       ####    ####
	0xF0, 0x1E, 0x3E, // ####       ####   #####
	0xF0, 0x1C, 0x7E, // ####       ###   ######
	0xFC, 0x1D, 0xFC, // ######     ### ####### 
	0xFF, 0xFF, 0xFC, // ###################### 
	0x7F, 0xFF, 0xF8, //  ####################  
	0x7F, 0xFF, 0xF0, //  ###################   
	0x3F, 0xFF, 0xE0, //   #################    
	0x0F, 0xFF, 0xC0, //     ##############     
	0x03, 0xFE, 0x00, //       #########        

	// @532 ':' (5 pixels wide)
	0x07, 0x80, 0xf0, //      ####       #### 
	0x0F, 0xc1, 0xf8, //     ######     ######
	0x0F, 0xc1, 0xf8, //     ######     ######
	0x0F, 0xc1, 0xf8, //     ######     ######
	0x07, 0x80, 0xf0, //      ####       #### 
	0x00, 0x00, 0x00, //                      

	// @550 'C' (12 pixels wide)
	0x07, 0xE0, //      ######     
	0x1F, 0xF8, //    ##########   
	0x3F, 0xFC, //   ############  
	0x7F, 0xFE, //  ############## 
	0xF8, 0x1F, // #####      #####
	0xF0, 0x0F, // ####        ####
	0xE0, 0x07, // ###          ###
	0xE0, 0x07, // ###          ###
	0xE0, 0x07, // ###          ###
	0xE0, 0x07, // ###          ###
	0x70, 0x0E, //  ###        ### 
	0x78, 0x1E, //  ####      #### 

	// @574 'F' (9 pixels wide)
	0xFF, 0xFF, // ################
	0xFF, 0xFF, // ################
	0xFF, 0xFF, // ################
	0xFF, 0xFF, // ################
	0xE3, 0x80, // ###   ###       
	0xE3, 0x80, // ###   ###       
	0xE3, 0x80, // ###   ###       
	0xE3, 0x80, // ###   ###       
	0xE3, 0x80, // ###   ###       
	0xE3, 0x80, // ###   ###       

	// @594 '`' (8 pixels wide)
	0x3C, 0x00, //   ####          
	0x7E, 0x00, //  ######         
	0xE7, 0x00, // ###  ###        
	0xC3, 0x00, // ##    ##        
	0xC3, 0x00, // ##    ##        
	0xE7, 0x00, // ###  ###        
	0x7E, 0x00, //  ######         
	0x3C, 0x00, //   ####          

	// @610 '-' (10 pixels wide)
	0xF0, // ####
	0xF0, // ####
	0xF0, // ####
	0xF0, // ####
	0xF0, // ####
	0xF0, // ####
	0xF0, // ####
	0xF0, // ####
	0xF0, // ####
	0xF0, // ####

  	// @620 ' ' (6 pixels wide)
	0x00, 0x00, //             
	0x00, 0x00, //             
	0x00, 0x00, //             
	0x00, 0x00, //             
	0x00, 0x00, //             
	0x00, 0x00, //             

	// @632 '.' (5 pixels wide)
	0x00, 0x00, 0x00, //             
	0x00, 0x00, 0x3c, //                   #### 
	0x00, 0x00, 0x7e, //                  ######
	0x00, 0x00, 0x7e, //                  ######
	0x00, 0x00, 0x7e, //                  ######
	0x00, 0x00, 0x3c, //                   #### 
	0x00, 0x00, 0x00, //             

};



// Character descriptors for Tahoma 24pt
// { [Char width in bits], [Char height in bits], [Offset into tahoma_24ptCharBitmaps in bytes] }
const FONT_CHAR_INFO tahoma_24ptDescriptors[] PROGMEM = 
{
	{5, 16, 620}, 		// ' ' 
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
	{10, 4, 610}, 		// '-' 
	{7, 23, 632}, 		// '.' 
	{0, 0, 0}, 		// '/' 
	{18, 23, 16}, 		// '0' 
	{16, 23, 70}, 		// '1' 
	{17, 23, 118}, 		// '2' 
	{17, 23, 169}, 		// '3' 
	{18, 23, 220}, 		// '4' 
	{17, 23, 274}, 		// '5' 
	{17, 23, 325}, 		// '6' 
	{17, 23, 376}, 		// '7' 
	{18, 23, 427}, 		// '8' 
	{17, 23, 481}, 		// '9' 
	{5, 23, 532}, 		// ':' 
	{0, 0, 0}, 		// ';' 
	{0, 0, 0}, 		// '<' 
	{0, 0, 0}, 		// '=' 
	{0, 0, 0}, 		// '>' 
	{0, 0, 0}, 		// '?' 
	{0, 0, 0}, 		// '@' 
	{0, 0, 0}, 		// 'A' 
	{0, 0, 0}, 		// 'B' 
	{12, 16, 550}, 		// 'C' 
	{0, 0, 0}, 		// 'D' 
	{0, 0, 0}, 		// 'E' 
	{10, 16, 574}, 		// 'F' 
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
	{8, 16, 594}, 		// '`' 
};

// Font information for Tahoma 24pt
const FONT_INFO tahoma_24ptFontInfo =
{
	39, //  Character height
	' ', //  Start character
	'`', //  End character
  2,   // width of space character
	tahoma_24ptDescriptors, //  Character descriptor array
	tahoma_24ptBitmaps, //  Character bitmap array
};

