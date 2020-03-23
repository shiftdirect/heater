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

#include <Arduino.h>
#include "Icons.h"
#include "../Screen.h"

// 'Thermometer', 8x50px
const uint8_t bodyThermometerIcon [] PROGMEM = {
	0x00,   // 
  0x18,   //    ##   
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x26,   //   #  ##  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x26,   //   #  ##  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x26,   //   #  ##  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x3c,   //   ####
  0x7e,   //  ######
  0xff,   // ########
  0xff,   // ########
  0xff,   // ########
  0x7e,   //  ######
  0x3c    //   ####
};
const BITMAP_INFO BodyThermometerIconInfo(8, 50, bodyThermometerIcon);

// 'ThermometerActual', 8x50px
const uint8_t ambientThermometerIcon [] PROGMEM = {
	0x00,   // 
  0x18,   //    ##   
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x26,   //   #  ##  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x26,   //   #  ##  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x26,   //   #  ##  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x26,   //   #  ##  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x24,   //   #  #  
  0x3c,   //   ####
  0x7e,   //  ######
  0xff,   // ########
  0xff,   // ########
  0xff,   // ########
  0x7e,   //  ######
  0x3c    //   ####
};
const BITMAP_INFO AmbientThermometerIconInfo(8, 50, ambientThermometerIcon);


// 'ThermoPtr', 3x5px
const uint8_t thermoPtr [] PROGMEM = {
	0x80, //   #
  0xc0, //   ##
  0xe0, //   ###
  0xc0, //   ##
	0x80  //   #
};
const BITMAP_INFO ThermoPtrIconInfo(3, 5, thermoPtr);

// 'ThermoPtr', 3x5px
const uint8_t thermoOpenPtr [] PROGMEM = {
  0x80, //   #
  0x40, //    #
  0x20, //     #
  0x40, //    #
  0x80  //   #
};
const BITMAP_INFO ThermoOpenPtrIconInfo(3, 5, thermoOpenPtr);

// 'ThermoPtrHigh', 3x3px
const uint8_t thermoPtrLow [] PROGMEM = {
  0xe0, //   ####
  0x60, //     ##
  0x20, //      #
};
const BITMAP_INFO ThermoPtrLowIconInfo(3, 3, thermoPtrLow);

// 'ThermoPtrLow', 3x3px
const uint8_t thermoPtrHigh [] PROGMEM = {
  0x20, //     #
  0x60, //    ##
  0xe0  //   ###
};
const BITMAP_INFO ThermoPtrHighIconInfo(3, 3, thermoPtrHigh);

// 'Bluetooth icon', 6x11px
const uint8_t BTicon [] PROGMEM = {
  0x20, //   #
  0x30, //   ##
  0x28, //   # #
  0xa4, // # #  #
  0x68, //  ## #
  0x30, //   ##
  0x68, //  ## #
  0xa4, // # #  #
  0x28, //   # #
  0x30, //   ##
  0x20  //   #
};
const BITMAP_INFO BluetoothIconInfo(6, 11, BTicon);

// 'wifiIcon', 13x10px
const uint8_t wifiIcon [] PROGMEM = {
  0x1f, 0xc0, //    #######
  0x20, 0x20, //   #       #
  0x40, 0x10, //  #         #
  0x8f, 0x88, // #   #####   #
  0x10, 0x40, //    #     #
  0x20, 0x20, //   #       #
  0x07, 0x00, //      ###
  0x08, 0x80, //     #   #
  0x00, 0x00, //
  0x02, 0x00  //       #
};
const BITMAP_INFO WifiIconInfo(13, 10, wifiIcon);

// 'wifiIconWide', 13x10px
const uint8_t wifiwideIcon [] PROGMEM = {
  0x1f, 0xc0, 0x00, 0x00, //    #######
  0x20, 0x20, 0x00, 0x00, //   #       #
  0x40, 0x10, 0x00, 0x00, //  #         #
  0x8f, 0x88, 0x00, 0x00, // #   #####   #
  0x10, 0x40, 0x00, 0x00, //    #     #
  0x20, 0x20, 0x00, 0x00, //   #       #
  0x07, 0x00, 0x00, 0x00, //      ###
  0x08, 0x80, 0x00, 0x00, //     #   #
  0x00, 0x00, 0x00, 0x00, //
  0x02, 0x00, 0x00, 0x00, //       #
  0x00, 0x00, 0x00, 0x00  //
};
const BITMAP_INFO WifiWideIconInfo(25, 11, wifiwideIcon);


// 'wifiInIcon, 5x5px
const uint8_t wifiInIcon [] PROGMEM = {
  0x70,   //  ###
  0x70,   //  ###
  0xf8,   // #####
  0x70,   //  ###
  0x20    //   #
};
const BITMAP_INFO WifiInIconInfo(5, 5, wifiInIcon);

// 'wifiOutIcon, 5x5px
const uint8_t wifiOutIcon [] PROGMEM = {
  0x20,   //   #
  0x70,   //  ###
  0xf8,   // #####
  0x70,   //  ###
  0x70,   //  ###
  0x70,   //  ###
  0x70    //  ###
};
const BITMAP_INFO WifiOutIconInfo(5, 5, wifiOutIcon);

// 'BatteryIcon', 15x10px
const uint8_t BatteryIcon [] PROGMEM = {
	0x30, 0x18, //   ##       ##
  0xff, 0xfe, // ###############
  0x80, 0x02, // #             #
  0xb6, 0xda, // # ## ## ## ## #
  0xb6, 0xda, // # ## ## ## ## #
  0xb6, 0xda, // # ## ## ## ## #
  0xb6, 0xda, // # ## ## ## ## #
  0xb6, 0xda, // # ## ## ## ## #
  0x80, 0x02, // #             #
  0xff, 0xfe  // ###############
};
const BITMAP_INFO BatteryIconInfo(15, 10, BatteryIcon);


// 'GlowPlugIcon', 16x9px
const uint8_t GlowPlugIcon [] PROGMEM = {
  0x71, 0xc7, //  ###   ###   ###
  0x0e, 0x38, //     ###   ###
  0x14, 0x14, //    # #     # #
  0x12, 0x24, //    #  #   #  #
  0x11, 0x44, //    #   # #   #
  0x11, 0x44, //    #   # #   #
  0x11, 0x44, //    #   # #   #
  0x0a, 0x28, //     # #   # #
  0x0e, 0x38  //     ###   ###
};
const BITMAP_INFO GlowPlugIconInfo(16, 9, GlowPlugIcon);


// 'HeatRise', 17x2px
const uint8_t GlowHeatIcon [] PROGMEM = {
  0x80, 0x00, 0x80, // #               #
  0x40, 0x01, 0x00  //  #             #
};
const BITMAP_INFO GlowHeatIconInfo(17, 2, GlowHeatIcon);

// 'Fan3_1a', 16x16px
const uint8_t FanIcon1 [] PROGMEM = {
  0x03, 0xc0, //       ####
  0x04, 0x20, //      #    #  
  0x04, 0x20, //      #    #  
  0x04, 0x20, //      #    #  
  0x04, 0x20, //      #    #  
  0x03, 0xc0, //       ####
  0x07, 0xe0, //      ######
  0x06, 0x60, //      ##  ##
  0x7e, 0x7e, //  ######  ######
  0x87, 0xe1, // #   ###  ###   #
  0x87, 0xe1, // #   ###  ###   #
  0x84, 0x21, // #    #    #    #
  0x84, 0x21, // #    #    #    #
  0x78, 0x1e, //  ####      ####
  0x00, 0x00, 
  0x00, 0x00
};
// 'Fan3_2a', 16x16px
const uint8_t FanIcon2 [] PROGMEM = {
  0x00, 0x78, //          ####
  0x00, 0x84, //         #    #
  0x00, 0x84, //         #    #
  0x00, 0x84, //         #    #
  0x00, 0x84, //         #    #
  0x7b, 0xf8, //  #### #######
  0x87, 0xe0, // #    ######
  0x86, 0x60, // #    ##  ##
  0x86, 0x60, // #    ##  ##
  0x87, 0xe0, // #    ######
  0x7b, 0xf8, //  #### #######
  0x00, 0x84, //         #    #
  0x00, 0x84, //         #    #
  0x00, 0x84, //         #    #
  0x00, 0x84, //         #    #
  0x00, 0x78, //          ####
};
// 'Fan3_3a', 16x16px
const uint8_t FanIcon3 [] PROGMEM = {
  0x00, 0x00, 
  0x00, 0x00, 
  0x78, 0x1e, //  ####      ####
  0x84, 0x21, // #    #    #    #
  0x84, 0x21, // #    #    #    #
  0x87, 0xe1, // #   ###  ###   #
  0x87, 0xe1, // #   #### ###   #
  0x7e, 0x7e, //  ######  ######
  0x06, 0x60, //      ##  ##
  0x07, 0xe0, //      ######
  0x03, 0xc0, //       ####
  0x04, 0x20, //      #    #  
  0x04, 0x20, //      #    #  
  0x04, 0x20, //      #    #  
  0x04, 0x20, //      #    #  
  0x03, 0xc0  //       ####
};
// 'Fan3_4a', 16x16px
const uint8_t FanIcon4 [] PROGMEM = {
  0x1e, 0x00, //    ####
  0x21, 0x00, //   #    #
  0x21, 0x00, //   #    #
  0x21, 0x00, //   #    #
  0x21, 0x00, //   #    #
  0x1f, 0xde, //    ####### #####
  0x07, 0xe1, //      ######    #
  0x06, 0x61, //      ##  ##    #
  0x06, 0x61, //      ##  ##    #
  0x07, 0xe1, //      ######    #
  0x1f, 0xde, //    ####### #####
  0x21, 0x00, //   #    #
  0x21, 0x00, //   #    #
  0x21, 0x00, //   #    #
  0x21, 0x00, //   #    #
  0x1e, 0x00  //    ####
};
const BITMAP_INFO FanIcon1Info(16, 16, FanIcon1);
const BITMAP_INFO FanIcon2Info(16, 16, FanIcon2);
const BITMAP_INFO FanIcon3Info(16, 16, FanIcon3);
const BITMAP_INFO FanIcon4Info(16, 16, FanIcon4);


// 'FuelIcon', 7x12px
const uint8_t FuelIcon [] PROGMEM = {
  0x10, //    #
  0x10, //    #
  0x38, //   ###
  0x38, //   ###
  0x7c, //  #####
  0x7c, //  #####
  0xfe, // #######
  0xfe, // #######
  0xfe, // #######
  0xfe, // #######
  0x7c, //  #####
  0x38  //   ###
};
const BITMAP_INFO FuelIconInfo(7, 12, FuelIcon);

// 
//  Image data for FuelIconSmall
// 

const uint8_t PROGMEM FuelIconSmall[]  =
{
	0x20, //   #  
	0x20, //   #  
	0x70, //  ### 
	0x70, //  ### 
	0xF8, // #####
	0xF8, // #####
	0xF8, // #####
	0xF8, // #####
	0x70, //  ### 
};
const BITMAP_INFO FuelIconSmallInfo(5, 9, FuelIconSmall);


// 'Target', 13x13px
const uint8_t TargetIcon [] PROGMEM = {
  0x0f, 0x80, //     #####
  0x10, 0x40, //    #     #
  0x20, 0x20, //   #       #
  0x47, 0x10, //  #   ###   #
  0x88, 0x88, // #   #   #   #
  0x92, 0x48, // #  #  #  #  #
  0x97, 0x48, // #  # ### #  #
  0x92, 0x48, // #  #  #  #  #
  0x88, 0x88, // #   #   #   #
  0x47, 0x10, //  #   ###   #
  0x20, 0x20, //   #       #
  0x10, 0x40, //    #     #
  0x0f, 0x80  //     #####
};
const BITMAP_INFO TargetIconInfo(13, 13, TargetIcon);

// 'repeat', 15x15px
const uint8_t repeatIcon [] PROGMEM = {
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x02, //               #
  0x00, 0x02, //               #
  0xf0, 0x04, // ####         #
  0xe0, 0x04, // ###          #
  0xe0, 0x08, // ###         #
  0x98, 0x30, // #  #      ##
  0x07, 0xc0  //      #####
};
const BITMAP_INFO RepeatIconInfo(15, 13, repeatIcon);

// 'repeat', 5x11px
const uint8_t verticalRepeatIcon [] PROGMEM = {
  0x78,  //  ####
  0x38,  //   ###
  0x38,  //   ###
  0x48,  //  #  #
  0x80,  // #
  0x80,  // #
  0x80,  // #
  0x80,  // #
  0x80,  // #
  0x40,  //  #
  0x40,  //  #
  0x20,  //   #
};
const BITMAP_INFO verticalRepeatIconInfo(5, 11, verticalRepeatIcon);


// 'timer', 15x15px
const uint8_t timerIcon [] PROGMEM = {
  0x07, 0xc0, //      #####
  0x09, 0x20, //     #  #  #
  0x11, 0x10, //    #   #   #
  0x21, 0x08, //   #    #    #
  0x21, 0x08, //   #    #    #
  0x21, 0xE8, //   #    #### #
  0x20, 0x08, //   #         #
  0x20, 0x08, //   #         #
  0x10, 0x10, //    #       #
  0x08, 0x20, //     #     #
  0x07, 0xc0, //      #####
};
const BITMAP_INFO TimerIconInfo(15, 11, timerIcon);

// 'timer', 15x15px
const uint8_t timerIconRpt [] PROGMEM = {
  0x07, 0xc0, //      #####
  0x09, 0x20, //     #  #  #
  0x11, 0x10, //    #   #   #
  0x21, 0x08, //   #    #    #
  0x21, 0x08, //   #    #    #
  0x21, 0xE8, //   #    #### #
  0x20, 0x08, //   #         #
  0x3E, 0x08, //   #####     #
  0x30, 0x10, //   ##       #
  0x28, 0x20, //   # #     #
  0x27, 0xc0, //   #  #####
};
const BITMAP_INFO TimerIconRptInfo(15, 11, timerIconRpt);

// 'large timer', 15x15px
const uint8_t largeTimerIcon[] PROGMEM  =
{
  0x07, 0xC0, //      #####     
  0x19, 0x30, //    ##  #  ##   
  0x21, 0x08, //   #    #    #  
  0x41, 0x04, //  #     #     # 
  0x41, 0x04, //  #     #     # 
  0x81, 0x02, // #      #      #
  0x81, 0xF2, // #      #####  #
  0x80, 0x02, // #             #
  0x80, 0x02, // #             #
  0x80, 0x02, // #             #
  0x40, 0x04, //  #           # 
  0x40, 0x04, //  #           # 
  0x20, 0x08, //   #         #  
  0x18, 0x30, //    ##     ##   
  0x0F, 0xE0, //     #######    
};
const BITMAP_INFO LargeTimerIconInfo(15, 15, largeTimerIcon);

const uint8_t PROGMEM verticalLargeRepeatIcon [] =
{
  0x78, //  #### 
  0x38, //   ### 
  0x38, //   ### 
  0x48, //  #  # 
  0x80, // #     
  0x80, // #     
  0x80, // #     
  0x80, // #     
  0x80, // #     
  0x80, // #     
  0x80, // #     
  0x40, //  #    
  0x40, //  #   
  0x20, //   #  
  0x20, //   #    
};
const BITMAP_INFO VerticalRepeatLargeIconInfo(5, 15, verticalLargeRepeatIcon);



const uint8_t PROGMEM CrossIcon[]  =
{
  0x88,   // #   #
  0x50,   //  # # 
  0x20,   //   #  
  0x50,   //  # # 
  0x88,   // #   #
};
const BITMAP_INFO CrossIconInfo(5, 5, CrossIcon);

const uint8_t PROGMEM TickIcon[]  =
{
  0x00,   //  
  0x08,   //     # 
  0x10,   //    #  
  0xa0,   // # #  
  0x40,   //  #  
};

const uint8_t PROGMEM OpenIcon[]  =
{
	0x1F, 0xC0, //    #######   
	0x02, 0x00, //       #      
	0x02, 0x00, //       #      
	0x3A, 0xE0, //   ### # ###  
	0xEA, 0xB8, // ### # # # ###
	0x3A, 0xE0, //   ### # ###  
	0x00, 0x00, //              
};
const BITMAP_INFO OpenIconInfo(13, 7, OpenIcon);

const uint8_t PROGMEM CrossLgIcon[]  =
{
	0x82, //  #     #
	0x44, //   #   #
	0x28, //    # # 
	0x10, //     #      
	0x28, //    # #    
	0x44, //   #   #  
	0x82, //  #     #
};
const BITMAP_INFO CrossLgIconInfo(7, 7, CrossLgIcon);


const uint8_t PROGMEM CloseIcon[]  =
{
	0x00, 0x00, //              
	0x00, 0x00, //              
	0x3F, 0xE0, //   #########  
	0x3A, 0xE0, //   ### # ###  
	0xFA, 0xF8, // ##### # #####
	0x3A, 0xE0, //   ### # ###  
	0x02, 0x00, //       #      
};
const BITMAP_INFO CloseIconInfo(13, 7, CloseIcon);


const uint8_t PROGMEM BulbOnIcon[]  =
{
	0x08, 0x00, //     #    
	0x41, 0x00, //  #     # 
	0x1C, 0x00, //    ###   
	0x22, 0x00, //   #   #  
	0xA2, 0x80, // # #   # #
	0x1C, 0x00, //    ###   
	0x14, 0x00, //    # #   
	0x1C, 0x00, //    ###   
};
const BITMAP_INFO BulbOnIconInfo(9, 8, BulbOnIcon);


const uint8_t PROGMEM bulbOn2aBitmap[]  =
{
	0x14, 0x00, //    # #   
	0x80, 0x80, // #       #
	0x1C, 0x00, //    ###   
	0xA2, 0x80, // # #   # #
	0x22, 0x00, //   #   #  
	0x1C, 0x00, //    ###   
	0x14, 0x00, //    # #   
	0x1C, 0x00, //    ###   
};
const BITMAP_INFO BulbOn2IconInfo(9, 8, bulbOn2aBitmap);


const uint8_t PROGMEM BulbOffIcon[]  =
{
	0x00, 0x00, //      
	0x00, 0x00, //      
	0x1C, 0x00, //    ### 
	0x22, 0x00, //   #   #
	0x22, 0x00, //   #   #
	0x1C, 0x00, //    ### 
	0x14, 0x00, //    # # 
	0x1C, 0x00, //    ### 
};
const BITMAP_INFO BulbOffIconInfo(9, 8, BulbOffIcon);


const uint8_t PROGMEM startIcon[]  =
{
	0x80, // #    
	0xC0, // ##   
	0xE0, // ###  
	0xF0, // #### 
	0xF8, // #####
	0xF0, // #### 
	0xE0, // ###  
	0xC0, // ##   
	0x80, // #    
};
const BITMAP_INFO StartIconInfo(5, 9, startIcon);

const uint8_t PROGMEM miniStartIcon[]  =
{
	0x80, // #    
	0xC0, // ##   
	0xE0, // ###  
	0xC0, // ## 
	0x80, // #
};
const BITMAP_INFO miniStartIconInfo(3, 5, miniStartIcon);


const uint8_t PROGMEM stopIcon[]  =
{
	0x00, //       
	0xFC, // ######
	0xFC, // ######
	0xFC, // ######
	0xFC, // ######
	0xFC, // ######
	0xFC, // ######
	0x00, //       
};
const BITMAP_INFO StopIconInfo(6, 8, stopIcon);

const uint8_t PROGMEM miniStopIcon[]  =
{
	0x00, //       
	0xE0, // ###
	0xE0, // ###
	0xE0, // ###
	0x00, //       
};
const BITMAP_INFO miniStopIconInfo(3, 5, miniStopIcon);

const uint8_t PROGMEM displayTimeoutIcon[]  =
{
	0xFF, 0xE1, 0xFF, // ###########    #########
	0x80, 0x20, 0x82, // #         #     #     # 
	0x80, 0x20, 0x82, // #         #     #     # 
	0x80, 0x20, 0x44, // #         #      #   #  
	0x80, 0x20, 0x28, // #         #       # #   
	0x80, 0x20, 0x44, // #         #      #   #  
	0xFF, 0xE0, 0x92, // ###########     #  #  # 
	0x0E, 0x00, 0xBA, //     ###         # ### # 
	0x3F, 0x80, 0xFE, //   #######       ####### 
	0x00, 0x01, 0xFF, //                #########
};
const BITMAP_INFO DisplayTimeoutIconInfo(24, 10, displayTimeoutIcon);

const uint8_t PROGMEM menuTimeoutIcon[]  =
{
	0x00, 0x01, 0xFF, //                #########
	0xFF, 0xC0, 0x82, // ##########      #     # 
	0x00, 0x00, 0x82, //                 #     # 
	0xFF, 0x00, 0x44, // ########         #   #  
	0x00, 0x00, 0x28, //                   # #   
	0xFC, 0x00, 0x44, // ######           #   #  
	0x00, 0x00, 0x92, //                 #  #  # 
	0xFF, 0x80, 0xBA, // #########       # ### # 
	0x00, 0x00, 0xFE, //                 ####### 
	0x00, 0x01, 0xFF, //                #########
};
const BITMAP_INFO MenuTimeoutIconInfo(24, 10, menuTimeoutIcon);

const uint8_t PROGMEM menuIcon[]  =
{
	0x00, 0x00,  //            
	0xFF, 0xC0,  // ########## 
	0x00, 0x00,  //            
	0xFF, 0x00,  // ########   
	0x00, 0x00,  //            
	0xFC, 0x00,  // ######     
	0x00, 0x00,  //            
	0xFF, 0x80,  // #########  
	0x00, 0x00,  //            
	0x00, 0x00,  //            
};
const BITMAP_INFO MenuIconInfo(10, 10, menuIcon);

const uint8_t PROGMEM timeoutIcon[]  =
{
	0xFF, 0x80, // #########
	0x41, 0x00, //  #     # 
	0x41, 0x00, //  #     # 
	0x22, 0x00, //   #   #  
	0x14, 0x00, //    # #   
	0x22, 0x00, //   #   #  
	0x49, 0x00, //  #  #  # 
	0x5D, 0x00, //  # ### # 
	0x7F, 0x00, //  ####### 
	0xFF, 0x80, // #########
};
const BITMAP_INFO TimeoutIconInfo(9, 10, timeoutIcon);

const uint8_t PROGMEM refreshIcon[]  =
{
	0x01, 0x00, //        #     
	0x00, 0x80, //         #    
	0x7F, 0xC8, //  #########  #
	0x80, 0x88, // #       #   #
	0x81, 0x08, // #      #    #
	0x80, 0x08, // #           #
	0x84, 0x08, // #    #      #
	0x88, 0x08, // #   #       #
	0x9F, 0xF0, // #  ######### 
	0x08, 0x00, //     #        
	0x04, 0x00, //      #       
};
const BITMAP_INFO RefreshIconInfo(13, 11, refreshIcon);

const uint8_t PROGMEM thermostatIcon[]  =
{
	0x00, 0x00, 0x07, 0x00, //                      ###    
	0x00, 0x00, 0x0E, 0x00, //                     ###     
	0x00, 0x00, 0x0C, 0x40, //                     ##   #  
	0x00, 0x00, 0x0C, 0xC0, //                     ##  ##  
	0x00, 0x00, 0x1F, 0xC0, //                    #######  
	0x00, 0x00, 0x3F, 0x80, //                   #######   
	0x00, 0x00, 0x7C, 0x00, //                  #####      
	0x00, 0x00, 0xF8, 0x00, //                 #####       
	0x38, 0x01, 0xF0, 0x00, //   ###          #####        
	0x44, 0x01, 0xE0, 0x00, //  #   #         ####         
	0x44, 0x00, 0xC0, 0x00, //  #   #          ##          
	0x45, 0xC0, 0x00, 0x00, //  #   # ###                  
	0x44, 0x00, 0x00, 0x00, //  #   #                      
	0x55, 0xC0, 0x08, 0x00, //  # # # ###          #       
	0x54, 0x00, 0x1C, 0x00, //  # # #             ###      
	0x55, 0xC0, 0x2A, 0x00, //  # # # ###        # # #     
	0x54, 0x00, 0x08, 0x00, //  # # #              #       
	0x54, 0x00, 0x00, 0x00, //  # # #                      
	0x54, 0x00, 0x08, 0x00, //  # # #              #       
	0x54, 0x00, 0x2A, 0x00, //  # # #            # # #     
	0x54, 0x00, 0x1C, 0x00, //  # # #             ###      
	0x54, 0x00, 0x08, 0x00, //  # # #              #       
	0x92, 0x00, 0x00, 0x00, // #  #  #                     
	0xBA, 0x00, 0x00, 0x00, // # ### #                     
	0xBA, 0x00, 0x00, 0x00, // # ### #                     
	0xBA, 0x00, 0x00, 0x00, // # ### #                     
	0x82, 0x00, 0x00, 0x00, // #     #                     
	0x7C, 0x00, 0x00, 0x80, //  #####                  #   
	0x00, 0x01, 0xB0, 0xC0, //                ## ##    ##  
	0x00, 0x01, 0xB0, 0xE0, //                ## ##    ### 
	0x00, 0x01, 0xB0, 0xF0, //                ## ##    ####
	0x00, 0x01, 0xB0, 0xE0, //                ## ##    ### 
	0x00, 0x01, 0xB0, 0xC0, //                ## ##    ##  
	0x00, 0x00, 0x00, 0x80, //                         #   
};
const BITMAP_INFO ThermostatIconInfo(28, 34, thermostatIcon);


const uint8_t PROGMEM GPIOIcon[]  =
{
	0x00, 0x00, 0x00, //                     
	0x00, 0x20, 0x00, //           #         
	0x01, 0x20, 0x00, //        #  #         
	0x00, 0xA0, 0x70, //         # #      ###
	0x3F, 0xE0, 0x10, //   #########        #
	0x00, 0xA5, 0x70, //         # #  # # ###
	0x01, 0x22, 0x40, //        #  #   #  #  
	0x00, 0x25, 0x70, //           #  # # ###
	0x00, 0x00, 0x00, //                     
	0x00, 0x00, 0x00, //                     
	0x00, 0x00, 0x00, //                     
	0x00, 0x00, 0x00, //                     
	0x20, 0x00, 0x00, //   #                 
	0x20, 0x80, 0x00, //   #     #           
	0x20, 0x40, 0x70, //   #      #       ###
	0x3F, 0xE0, 0x10, //   #########        #
	0x20, 0x45, 0x70, //   #      #   # # ###
	0x20, 0x82, 0x40, //   #     #     #  #  
	0x20, 0x05, 0x70, //   #          # # ###
	0x00, 0x00, 0x00, //                     
	0x00, 0x00, 0x00, //                     
	0x00, 0x00, 0x00, //                     
	0x00, 0x00, 0x00, //                     
	0x00, 0x00, 0x00, //                     
	0x08, 0x00, 0x00, //     #               
	0x09, 0x00, 0x00, //     #  #            
	0x05, 0x00, 0x00, //      # #            
	0x24, 0x08, 0x00, //   #  #      #       
	0x02, 0x00, 0x00, //       #             
	0x02, 0x00, 0x00, //       #             
	0x01, 0x00, 0x00, //        #            
	0xC3, 0x86, 0x00, // ##    ###    ##     
	0x01, 0x80, 0x00, //        ##           
};
const BITMAP_INFO GPIOIconInfo(20, 33, GPIOIcon);
const BITMAP_INFO GPIOIconNoAlgInfo(20, 20, GPIOIcon);


const uint8_t PROGMEM firmwareIcon[]  =
{
	0xFF, 0xFF, 0xFF, 0xC0, // ##########################
	0x80, 0x00, 0x00, 0x40, // #                        #
	0x9F, 0xFF, 0x2A, 0x40, // #  #############  # # #  #
	0x80, 0x00, 0x00, 0x40, // #                        #
	0xFF, 0xFF, 0xFF, 0xC0, // ##########################
	0x80, 0x00, 0x00, 0x40, // #                        #
	0x80, 0x00, 0x00, 0x40, // #                        #
	0x80, 0x7F, 0x00, 0x40, // #        #######         #
	0x80, 0x7F, 0x00, 0x40, // #        #######         #
	0x80, 0x7F, 0x00, 0x40, // #        #######         #
	0x80, 0x08, 0x00, 0x40, // #           #            #
	0x80, 0x08, 0x00, 0x40, // #           #            #
	0x83, 0xFF, 0xE0, 0x40, // #     #############      #
	0x82, 0x08, 0x20, 0x40, // #     #     #     #      #
	0x82, 0x08, 0x20, 0x40, // #     #     #     #      #
	0x8F, 0xBE, 0xF8, 0x40, // #   ##### ##### #####    #
	0x8F, 0xBE, 0xF8, 0x40, // #   ##### ##### #####    #
	0x8F, 0xBE, 0xF8, 0x40, // #   ##### ##### #####    #
	0x80, 0x00, 0x00, 0x40, // #                        #
	0x80, 0x00, 0x00, 0x40, // #                        #
	0xFF, 0xFF, 0xFF, 0xC0, // ##########################
};
const BITMAP_INFO FirmwareIconInfo(26, 21, firmwareIcon);


const uint8_t PROGMEM hardwareIcon[]  =
{
	0xFF, 0xFF, // ################
	0x80, 0x01, // #              #
	0x95, 0x09, // #  # # #    #  #
	0x95, 0x09, // #  # # #    #  #
	0xBF, 0x89, // # #######   #  #
	0xA0, 0x89, // # #     #   #  #
	0xA0, 0x81, // # #     #      #
	0xA0, 0x81, // # #     #      #
	0xA0, 0x81, // # #     #      #
	0xA0, 0x89, // # #     #   #  #
	0xBF, 0x89, // # #######   #  #
	0x95, 0x09, // #  # # #    #  #
	0x95, 0x09, // #  # # #    #  #
	0x80, 0x01, // #              #
	0xFF, 0xFF, // ################
};
const BITMAP_INFO HardwareIconInfo(16, 15, hardwareIcon);

// 
//  Image data for caution
// 

const uint8_t PROGMEM cautionIcon[]  =
{
	0x00, 0x07, 0x80, 0x00, //              ####             
	0x00, 0x0F, 0xC0, 0x00, //             ######            
	0x00, 0x1F, 0xE0, 0x00, //            ########           
	0x00, 0x3C, 0xF0, 0x00, //           ####  ####          
	0x00, 0x38, 0x70, 0x00, //           ###    ###          
	0x00, 0x73, 0x38, 0x00, //          ###  ##  ###         
	0x00, 0x77, 0xB8, 0x00, //          ### #### ###         
	0x00, 0xE7, 0x9C, 0x00, //         ###  ####  ###        
	0x00, 0xEF, 0xDC, 0x00, //         ### ###### ###        
	0x01, 0xCF, 0xCE, 0x00, //        ###  ######  ###       
	0x01, 0xDC, 0xEE, 0x00, //        ### ###  ### ###       
	0x03, 0x9C, 0xE7, 0x00, //       ###  ###  ###  ###      
	0x03, 0xBC, 0xF7, 0x00, //       ### ####  #### ###      
	0x07, 0x3C, 0xF3, 0x80, //      ###  ####  ####  ###     
	0x07, 0x7C, 0xFB, 0x80, //      ### #####  ##### ###     
	0x0E, 0x7C, 0xF9, 0xC0, //     ###  #####  #####  ###    
	0x0E, 0xFC, 0xFD, 0xC0, //     ### ######  ###### ###    
	0x1C, 0xFC, 0xFC, 0xE0, //    ###  ######  ######  ###   
	0x1D, 0xFF, 0xFE, 0xE0, //    ### ################ ###   
	0x39, 0xFF, 0xFE, 0x70, //   ###  ################  ###  
	0x3B, 0xFF, 0xFF, 0x70, //   ### ################## ###  
	0x73, 0xFC, 0xFF, 0x38, //  ###  ########  ########  ### 
	0x77, 0xFC, 0xFF, 0xB8, //  ### #########  ######### ### 
	0xE7, 0xFF, 0xFF, 0x9C, // ###  ####################  ###
	0xEF, 0xFF, 0xFF, 0xDC, // ### ###################### ###
	0xE0, 0x00, 0x00, 0x1C, // ###                        ###
	0xFF, 0xFF, 0xFF, 0xFC, // ##############################
	0x7F, 0xFF, 0xFF, 0xF8, //  ############################ 
	0x3F, 0xFF, 0xFF, 0xF0, //   ##########################  
};
const BITMAP_INFO CautionIconInfo(30, 29, cautionIcon);


// 
//  Image data for update
// 

const uint8_t PROGMEM updateIcon[]  =
{
	0xFF, 0x80, // #########
	0x80, 0x80, // #       #
	0xFF, 0x80, // #########
	0xF7, 0x80, // #### ####
	0xE3, 0x80, // ###   ###
	0xD5, 0x80, // ## # # ##
	0xF7, 0x80, // #### ####
	0xF7, 0x80, // #### ####
	0xFF, 0x80, // #########
	0xFF, 0x80, // #########
};
const BITMAP_INFO UpdateIconInfo(9, 10, updateIcon);


const uint8_t PROGMEM wwwIcon[]  =
{
	0x3F, 0x00, //   ######  
	0x61, 0x80, //  ##    ## 
	0xB3, 0x40, // # ##  ## #
	0x92, 0x40, // #  #  #  #
	0xFF, 0xC0, // ##########
	0x92, 0x40, // #  #  #  #
	0xB3, 0x40, // # ##  ## #
	0x61, 0x80, //  ##    ## 
	0x3F, 0x00, //   ######  
};
const BITMAP_INFO WWWIconInfo(10, 9, wwwIcon);


// 
//  Image data for bowser
// 

const uint8_t PROGMEM bowserIcon[]  =
{
	0x7E, 0x80, //  ###### # 
	0x7E, 0xC0, //  ###### ##
	0x42, 0xC0, //  #    # ##
	0x42, 0x40, //  #    #  #
	0x42, 0x40, //  #    #  #
	0x7E, 0x40, //  ######  #
	0x7F, 0x40, //  ####### #
	0x7F, 0x40, //  ####### #
	0x7F, 0xC0, //  #########
	0x7E, 0x00, //  ######   
	0x7E, 0x00, //  ######   
	0xFF, 0x00, // ########  
};
const BITMAP_INFO BowserIconInfo(10, 12, bowserIcon);


// 
//  Image data for degC
// 
const uint8_t PROGMEM degCIcon[]  =
{
	0x07, 0x00, 0x00, //      ###                
	0x18, 0xC1, 0x8E, //    ##   ##     ##   ### 
	0x27, 0x22, 0x51, //   #  ###  #   #  # #   #
	0x4F, 0x92, 0x50, //  #  #####  #  #  # #    
	0x8F, 0x89, 0x90, // #   #####   #  ##  #    
	0x4F, 0x90, 0x10, //  #  #####  #       #    
	0x27, 0x20, 0x11, //   #  ###  #        #   #
	0x18, 0xC0, 0x0E, //    ##   ##          ### 
	0x07, 0x00, 0x00, //      ###                
};
const BITMAP_INFO DegCIconInfo(23, 9, degCIcon);

const uint8_t PROGMEM degFIcon[] =
{
	0x07, 0x00, 0x00, //      ###               
	0x18, 0xC1, 0x9E, //    ##   ##     ##  ####
	0x27, 0x22, 0x50, //   #  ###  #   #  # #   
	0x4F, 0x92, 0x50, //  #  #####  #  #  # #   
	0x8F, 0x89, 0x9C, // #   #####   #  ##  ### 
	0x4F, 0x90, 0x10, //  #  #####  #       #   
	0x27, 0x20, 0x10, //   #  ###  #        #   
	0x18, 0xC0, 0x10, //    ##   ##         #   
	0x07, 0x00, 0x00, //      ###               
};
const BITMAP_INFO DegFIconInfo(23, 9, degFIcon);

// 
//  Image data for thermostatC
// 

const uint8_t PROGMEM thermostatDegCIcon[] =
{
	0x7F, 0xFF, 0xFC, //  ##################### 
	0xFF, 0xFF, 0xFE, // #######################
	0xE0, 0x07, 0xDE, // ###          ##### ####
	0xCC, 0x73, 0x8E, // ##  ##   ###  ###   ###
	0xD2, 0x8B, 0x06, // ## #  # #   # ##     ##
	0xD2, 0x83, 0xFE, // ## #  # #     #########
	0xCC, 0x83, 0xFE, // ##  ##  #     #########
	0xC0, 0x83, 0x06, // ##      #     ##     ##
	0xC0, 0x8B, 0x8E, // ##      #   # ###   ###
	0xC0, 0x73, 0xDE, // ##       ###  #### ####
	0xE0, 0x07, 0xFE, // ###          ##########
	0xFF, 0xFF, 0xFE, // #######################
	0x7F, 0xFF, 0xFC, //  ##################### 
};
const BITMAP_INFO ThermostatDegCIconInfo(23, 13, thermostatDegCIcon);


// 
//  Image data for thermostatDegF
// 

const uint8_t PROGMEM thermostatDegFIcon[] =
{
	0x7F, 0xFF, 0xFC, //  ##################### 
	0xFF, 0xFF, 0xFE, // #######################
	0xE0, 0x07, 0xDE, // ###          ##### ####
	0xCC, 0xF3, 0x8E, // ##  ##  ####  ###   ###
	0xD2, 0x83, 0x06, // ## #  # #     ##     ##
	0xD2, 0x83, 0xFE, // ## #  # #     #########
	0xCC, 0xE3, 0xFE, // ##  ##  ###   #########
	0xC0, 0x83, 0x06, // ##      #     ##     ##
	0xC0, 0x83, 0x8E, // ##      #     ###   ###
	0xC0, 0x83, 0xDE, // ##      #     #### ####
	0xE0, 0x07, 0xFE, // ###          ##########
	0xFF, 0xFF, 0xFE, // #######################
	0x7F, 0xFF, 0xFC, //  ##################### 
};
const BITMAP_INFO ThermostatDegFIconInfo(23, 13, thermostatDegFIcon);

// 
//  Image data for thermostatHz
// 

const uint8_t PROGMEM thermostatHzIcon[]  =
{
	0x7F, 0xFF, 0xFC, //  ##################### 
	0xFF, 0xFF, 0xFE, // #######################
	0xE0, 0x07, 0xDE, // ###          ##### ####
	0xC9, 0x03, 0x8E, // ##  #  #      ###   ###
	0xC9, 0x03, 0x06, // ##  #  #      ##     ##
	0xC9, 0x73, 0xFE, // ##  #  # ###  #########
	0xCF, 0x13, 0xFE, // ##  ####   #  #########
	0xC9, 0x23, 0x06, // ##  #  #  #   ##     ##
	0xC9, 0x43, 0x8E, // ##  #  # #    ###   ###
	0xC9, 0x73, 0xDE, // ##  #  # ###  #### ####
	0xE0, 0x07, 0xFE, // ###          ##########
	0xFF, 0xFF, 0xFE, // #######################
	0x7F, 0xFF, 0xFC, //  ##################### 
};
const BITMAP_INFO ThermostatHzIconInfo(23, 13, thermostatHzIcon);


// 
//  Image data for reset
// 

const uint8_t PROGMEM resetIcon[]  =
{
	0x9E, 0x00, 0x00, // #  ####          
	0xA1, 0x07, 0x00, // # #    #     ### 
	0xC0, 0x88, 0x80, // ##      #   #   #
	0xF0, 0x49, 0x80, // ####     #  #  ##
	0x00, 0x4A, 0x80, //          #  # # #
	0x00, 0x4C, 0x80, //          #  ##  #
	0x00, 0x48, 0x80, //          #  #   #
	0x40, 0x87, 0x00, //  #      #    ### 
	0x21, 0x00, 0x00, //   #    #         
	0x1E, 0x00, 0x00, //    ####          
};
const BITMAP_INFO resetIconInfo(17, 10, resetIcon);

// 
//  Image data for miniThermo
// 

const uint8_t PROGMEM miniThermoIcon[]  =
{
	0x30, //   ##  
	0x48, //  #  # 
	0x48, //  #  # 
	0x48, //  #  # 
	0x48, //  #  # 
	0x48, //  #  # 
	0x48, //  #  # 
	0x48, //  #  # 
	0x78, //  #### 
	0xFC, // ######
	0xFC, // ######
	0xFC, // ######
	0x78, //  #### 
};
const BITMAP_INFO miniThermoIconInfo(6, 13, miniThermoIcon);

// 
//  Image data for externalThermostat
// 

const uint8_t PROGMEM ExtThermoIcon[]  =
{
	0x00, 0x80,  //         #         
	0x04, 0x80,  //      #  #         
	0x02, 0x9c,  //       # #  ###
	0xFF, 0x84,  // #########    #
	0x02, 0x9c,  //       # #  ###
	0x04, 0x90,  //      #  #  #  
	0x00, 0x9c,  //         #  ###
};
const BITMAP_INFO ExtThermoIconInfo(14, 7, ExtThermoIcon);

const uint8_t PROGMEM ExtThermo2Icon[]  =
{
	0x33, 0xE0,  //   ##  #####
	0x42, 0xA0,  //  #    # # #
	0x30, 0x80,  //   ##    #  
	0x70, 0x80,  //  ###    #  
	0x88, 0x80,  // #   #   #  
	0x88, 0x80,  // #   #   #   
	0x70, 0x80,  //  ###    #  
};
const BITMAP_INFO ExtThermo2IconInfo(11, 7, ExtThermo2Icon);

const uint8_t PROGMEM inputIcon[]  =
{
	0x00, 0x80,  //         #  
	0x04, 0x80,  //      #  #  
	0x02, 0x80,  //       # #  
	0xFF, 0x80,  // #########  
	0x02, 0x80,  //       # #  
	0x04, 0x80,  //      #  #  
	0x00, 0x80,  //         #  
};
const BITMAP_INFO InputIconInfo(9, 7, inputIcon);

const uint8_t PROGMEM outputIcon[]  =
{
  0x80, 0x00,  //   #           
	0x82, 0x00,  //   #     #    
	0x81, 0x00,  //   #      #   
	0xFF, 0x80,  //   #########  
	0x81, 0x00,  //   #      #   
	0x82, 0x00,  //   #     #    
	0x80, 0x00,  //   #          
};
const BITMAP_INFO OutputIconInfo(9, 7, outputIcon);

const uint8_t PROGMEM _1Icon[]  =
{
	0x00, //          
	0x44, //    #   # 
	0xC0, //   ##     
	0x40, //    #     
	0x44, //    #   # 
	0x40, //    #     
	0xE0, //   ###    
};
const BITMAP_INFO _1IconInfo(7, 7, _1Icon);

const uint8_t PROGMEM _2Icon[]  =
{
	0x00, //          
	0x64, //   ##   # 
	0x90, //  #  #    
	0x10, //     #    
	0x24, //    #   # 
	0x40, //   #      
	0xF0, //  ####    
};
const BITMAP_INFO _2IconInfo(7, 7, _2Icon);

const uint8_t PROGMEM algIcon[]  =
{
	0x08, 0x00,  //     #               
	0x09, 0x00,  //     #  #            
	0x05, 0x00,  //      # #            
	0x24, 0x08,  //   #  #      #       
	0x02, 0x00,  //       #             
	0x02, 0x00,  //       #             
	0x01, 0x00,  //        #            
	0xC3, 0x86,  // ##    ###    ##     
	0x01, 0x80,  //        ##           
};
const BITMAP_INFO algIconInfo(15, 9, algIcon);

// 
//  Image data for run
// 

const uint8_t PROGMEM runBitmap[]  =
{
	0x03, //       ##
	0x03, //       ##
	0x30, //   ##    
	0x4E, //  #  ### 
	0x05, //      # #
	0x08, //     #   
	0x14, //    # #  
	0x22, //   #   # 
	0x44, //  #   #  
};
const BITMAP_INFO RunIconInfo(8, 9, runBitmap);

// 
//  Image data for info
// 

const uint8_t PROGMEM infoBitmap[]  =
{
	0x3C, 0x00, //   ####   
	0x66, 0x00, //  ##  ##  
	0xE7, 0x00, // ###  ### 
	0xFF, 0x00, // ######## 
	0xE7, 0x00, // ###  ### 
	0xE7, 0x00, // ###  ### 
	0xE7, 0x00, // ###  ### 
	0x66, 0x00, //  ##  ##  
	0x3C, 0x00, //   ####   
};
const BITMAP_INFO InfoIconInfo(9, 9, infoBitmap);

// 
//  Image data for user
// 

const uint8_t PROGMEM userBitmap[]  =
{
	0x18,  //    ##    
	0x3C,  //   ####   
	0x3C,  //   ####   
	0x18,  //    ##    
	0x24,  //   #  #   
	0x7E,  //  ######  
	0xFF,  // ######## 
	0xFF,  // ######## 
	0xFF,  // ######## 
};
const BITMAP_INFO UserIconInfo(8, 9, userBitmap);


// 
//  Image data for password
// 

const uint8_t PROGMEM passwordIcon[]  =
{
	0x0E, 0x00, //     ###    
	0x1B, 0x00, //    ## ##   
	0x11, 0x00, //    #   #   
	0x11, 0x00, //    #   #   
	0x7F, 0xC0, //  ######### 
	0x71, 0xC0, //  ###   ### 
	0x71, 0xC0, //  ###   ### 
	0x7B, 0xC0, //  #### #### 
	0x7B, 0xC0, //  #### #### 
	0x7F, 0xC0, //  ######### 
	0x00, 0x00, //            
	0xEE, 0xE0, // ### ### ###
	0xAA, 0xA0, // # # # # # #
	0x22, 0x20, //   #   #   #
	0x44, 0x40, //  #   #   # 
	0x00, 0x00, //            
	0x44, 0x40, //  #   #   # 
};

const BITMAP_INFO passwordIconInfo(11, 17, passwordIcon);


// 
//  Image data for thresh
// 

const uint8_t PROGMEM threshIcon[]  =
{
	0xAA, 0x80, // # # # # #
	0x00, 0x00, //          
	0x30, 0x00, //   ##     
	0x49, 0x00, //  #  #  # 
	0xFF, 0x80, // #########
	0x49, 0x00, //  #  #  # 
	0x06, 0x00, //      ##  
	0x00, 0x00, //          
	0xAA, 0x80, // # # # # #
};

const BITMAP_INFO threshIconInfo(9, 9, threshIcon);

// 
//  Image data for onOff
// 

const uint8_t PROGMEM onOffIcon[]  =
{
  0x10,  //    #     
  0x54,  //  # # #   
  0x92,  // #  #  # 
  0x92,  // #  #  #  
  0x82,  // #     #  
  0x44,  //  #   #   
  0x38,  //   ###    
};

const BITMAP_INFO onOffIconInfo(7, 7, onOffIcon);
// 
//  Image data for frost
// 

const uint8_t PROGMEM frostIcon[]  =
{
	0x15, 0x00, //    # # #   
	0x0A, 0x00, //     # #    
	0xA4, 0xA0, // # #  #  # #
	0x44, 0x40, //  #   #   # 
	0xA4, 0xA0, // # #  #  # #
	0x15, 0x00, //    # # #   
	0x0E, 0x00, //     ###    
	0x15, 0x00, //    # # #   
	0xA4, 0xA0, // # #  #  # #
	0x44, 0x40, //  #   #   # 
	0xA4, 0xA0, // # #  #  # #
	0x0A, 0x00, //     # #    
	0x15, 0x00, //    # # #   
};

const BITMAP_INFO frostIconInfo(11, 13, frostIcon);


const uint8_t PROGMEM humidityIcon[]  =
{
	0x20, 0x10, //   #        #   
	0x50, 0x28, //  # #      # #  
	0x50, 0x28, //  # #      # #  
	0x88, 0x44, // #   #    #   # 
	0x88, 0x44, // #   #    #   # 
	0x88, 0x82, // #   #   #     #
	0x70, 0x82, //  ###    #     #
	0x04, 0x82, //      #  #     #
	0x0A, 0x44, //     # #  #   # 
	0x0A, 0x38, //     # #   ###  
	0x11, 0x00, //    #   #       
	0x11, 0x00, //    #   #       
	0x11, 0x00, //    #   #       
	0x0E, 0x00, //     ###        
};

const BITMAP_INFO humidityIconInfo(15, 14, humidityIcon);
