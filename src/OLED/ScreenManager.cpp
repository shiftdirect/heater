/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2019  Ray Jones <ray@mrjones.id.au>
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

#include "ScreenManager.h"
#include "DetailedScreen.h"
#include "BasicScreen.h"
#include "PrimingScreen.h"
#include "WiFiScreen.h"
#include "FuelMixtureScreen.h"
#include "SetClockScreen.h"
#include "SetTimerScreen.h"
#include "ClockScreen.h"
#include "RebootScreen.h"
#include "HeaterSettingsScreen.h"
#include "FuelCalScreen.h"
#include "SettingsScreen.h"
#include "ThermostatModeScreen.h"
#include "FontDumpScreen.h"
#include "TimerChartScreen.h"
#include "InheritSettingsScreen.h"
#include "GPIOInfoScreen.h"
#include "GPIOSetupScreen.h"
#include "VersionInfoScreen.h"
#include "HomeMenuSelScreen.h"
#include "TimeoutsScreen.h"
#include "HourMeterScreen.h"
#include "BTScreen.h"
#include "MenuTrunkScreen.h"
#include "MQTTScreen.h"
#include <Wire.h>
#include "../cfg/pins.h"
#include "../cfg/BTCConfig.h"
#include "KeyPad.h"
#include "fonts/Icons.h"
#include "fonts/MiniFont.h"
#include "fonts/MidiFont.h"
#include "../Protocol/Protocol.h"
#include "fonts/Arial.h"
#include <SPIFFS.h>
#include "../Utility/BoardDetect.h"

#pragma pack ( push, 1)
struct sBMPhdr {
  char b0;
  char b1;
  uint32_t filesize;
  uint16_t resv1;
  uint16_t resv2;
  uint32_t startofs;
  uint32_t hdrsize;
  uint32_t width;
  uint32_t height;
  uint16_t numcolorplanes;
  uint16_t bitsperpixel;
  uint32_t compmethod;
  uint32_t imagesize;
  uint32_t hRes;
  uint32_t vRes;
  uint32_t numColorsPalette;
  uint32_t numImportantColors;
};
#pragma pack (pop)

extern CScreenManager ScreenManager;

////////////////////////////////////////////////////////////////////////////////////////////////
// splash creen created using image2cpp http://javl.github.io/image2cpp/
//   Settings: 
//      Black background
//      Invert [X]
//      Arduino code, single bitmap
//      Identifier: DieselSplash
//      Draw Mode: Horizontal
//
/*const uint8_t DieselSplash [] PROGMEM = {
// 'Splash3, 128x64px
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x06, 0x00, 0x00, 0x00, 0x00, 
	0x02, 0x3e, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 
	0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 
	0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 
	0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 
	0x00, 0x08, 0x00, 0x00, 0x01, 0xf0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x08, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x88, 0x00, 0x00, 
	0x00, 0x10, 0x00, 0x00, 0x20, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x84, 0x00, 0x00, 
	0x00, 0x18, 0x00, 0x00, 0x20, 0x40, 0x00, 0x20, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x84, 0x00, 0x00, 
	0x00, 0x14, 0x00, 0x00, 0x40, 0x40, 0x00, 0x10, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x84, 0x00, 0x00, 
	0x00, 0x52, 0x00, 0x00, 0x40, 0x20, 0x00, 0x08, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x82, 0x00, 0x00, 
	0x00, 0x34, 0x00, 0x00, 0x40, 0x10, 0x00, 0x04, 0x03, 0xe0, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 
	0x00, 0x18, 0x00, 0x00, 0x80, 0x10, 0x00, 0x02, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 
	0x00, 0x34, 0x00, 0x00, 0x80, 0x08, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 
	0x00, 0x52, 0x00, 0x00, 0x80, 0x08, 0x01, 0xf1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 
	0x00, 0x14, 0x00, 0x01, 0x00, 0x04, 0x3e, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 
	0x00, 0x18, 0x00, 0x01, 0x00, 0x07, 0xc0, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 
	0x00, 0x10, 0x00, 0x06, 0x80, 0x1c, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x03, 0xc4, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x18, 0x40, 0x64, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x3c, 0x78, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x3c, 0x40, 0x84, 0x00, 0x00, 0x01, 0x80, 0x00, 0x01, 0xc0, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x26, 0x23, 0x04, 0x00, 0x00, 0x00, 0x60, 0x00, 0x1e, 0x00, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x41, 0x88, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x43, 0x1c, 0x02, 0x00, 0x00, 0x00, 0x1c, 0x01, 0xe0, 0x00, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x41, 0xf8, 0x02, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x00, 0x00, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x40, 0x88, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x40, 0x88, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x40, 0x88, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x60, 0x88, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x30, 0x88, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x10, 0x88, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x19, 0x88, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x0f, 0x88, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x07, 0xf8, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x04, 0x02, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x07, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xc2, 0x00, 0x00, 0xf8, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x1f, 0x20, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x03, 0xe8, 0x20, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x7c, 0x08, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x08, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x80, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x01, 0x46, 0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x10, 0x00, 0x50, 0x00, 0x00, 
	0x00, 0x02, 0x28, 0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x10, 0x00, 0x50, 0x00, 0x00, 
	0x00, 0x02, 0x28, 0x73, 0x2d, 0xc9, 0x5a, 0x8c, 0xb0, 0x20, 0x31, 0xdd, 0x66, 0x53, 0x2c, 0x00, 
	0x00, 0x03, 0xee, 0x44, 0xb1, 0x29, 0x63, 0x52, 0xc0, 0x20, 0x4a, 0x51, 0x89, 0x54, 0xb0, 0x00, 
	0x00, 0x02, 0x28, 0x47, 0xa1, 0x29, 0x42, 0x5e, 0x80, 0x20, 0x4a, 0x51, 0x09, 0x57, 0xa0, 0x00, 
	0x00, 0x02, 0x28, 0x44, 0x21, 0x2b, 0x42, 0x50, 0x80, 0x21, 0x4a, 0x51, 0x09, 0x54, 0x20, 0x00, 
	0x00, 0x02, 0x28, 0x33, 0x21, 0xc5, 0x42, 0x4c, 0x80, 0x1e, 0x32, 0x4d, 0x06, 0x53, 0x20, 0x00
};
*/


const uint8_t DieselSplash [] PROGMEM = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                        #                                                        
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                             #######                    ##                                                       
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                            #       #                   # #                                                      
	0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x08, 0x00, 0x05, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                           #         #                # #  #                                                     
	0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0xC4, 0x00, 0x03, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                          #   #####   #                ## #                                                      
	0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x20, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                             #     #                    ##                                                       
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x03, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                            #       #                  ## #                                                      
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x05, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                               ###                    # #  #                                                     
	0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x40, 0x00, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                              #   #                     # #                                                      
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                        ##                                                       
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                #                       #                                                        
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //            #                                                                                                                    
	0x00, 0x30, 0x03, 0x03, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //           ##          ##      ##                        #                                                                       
	0x00, 0x30, 0x07, 0x86, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //           ##         ####    ##                        ##                                                                       
	0x00, 0x78, 0x0E, 0xC6, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //          ####       ### ##   ##                        ##                                                                       
	0x00, 0x78, 0x0C, 0xC6, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //          ####       ##  ##   ##                        ##                                                                       
	0x00, 0xD8, 0x0C, 0x0C, 0x0E, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x0E, 0x00, 0x1C, //         ## ##       ##      ##      ###            ## ##                                ###                 ###            ###  
	0x00, 0xCC, 0x18, 0x0C, 0x1F, 0x06, 0x3B, 0x1C, 0x00, 0x30, 0x31, 0xCC, 0x00, 0x1F, 0x06, 0x38, //         ##  ##     ##       ##     #####     ##   ### ##   ###            ##      ##   ###  ##             #####     ##   ###   
	0x01, 0x8C, 0x18, 0xFF, 0xF3, 0x06, 0x73, 0x3E, 0x18, 0x30, 0x33, 0x8C, 0xF0, 0x33, 0x06, 0x70, //        ##   ##     ##   ############  ##     ##  ###  ##  #####    ##     ##      ##  ###   ##  ####      ##  ##     ##  ###    
	0x03, 0x8C, 0x3F, 0x8C, 0x33, 0x0E, 0xC3, 0x66, 0x38, 0x30, 0x76, 0x0D, 0xF8, 0x33, 0x0E, 0xC0, //       ###   ##    #######   ##    ##  ##    ### ##    ## ##  ##   ###     ##     ### ##     ## ######     ##  ##    ### ##      
	0x03, 0x0C, 0x3E, 0x0C, 0x36, 0x17, 0x83, 0xC6, 0x58, 0x70, 0xBC, 0x0F, 0xB8, 0x76, 0x17, 0x80, //       ##    ##    #####     ##    ## ##    # ####     ####   ##  # ##    ###    # ####      ##### ###    ### ##    # ####       
	0x07, 0xFE, 0x18, 0x0C, 0x3C, 0x27, 0x03, 0x86, 0x98, 0x71, 0x38, 0x0F, 0x18, 0xBC, 0x27, 0x00, //      ##########    ##       ##    ####    #  ###      ###    ## #  ##    ###   #  ###       ####   ##   # ####    #  ###        
	0x0F, 0xCE, 0x18, 0x18, 0x30, 0x47, 0x03, 0x8F, 0x18, 0xDE, 0x38, 0x0E, 0x19, 0x30, 0x47, 0x00, //     ######  ###    ##      ##     ##     #   ###      ###   ####   ##   ## ####   ###       ###    ##  #  ##     #   ###        
	0x1C, 0x07, 0x18, 0x18, 0x31, 0x86, 0x03, 0x1E, 0x19, 0x8C, 0x30, 0x0E, 0x1E, 0x31, 0x86, 0x00, //    ###       ###   ##      ##     ##   ##    ##       ##   ####    ##  ##   ##    ##        ###    ####   ##   ##    ##         
	0x38, 0x03, 0x18, 0x18, 0x3F, 0x06, 0x03, 0x38, 0x1F, 0x00, 0x30, 0x0C, 0x0C, 0x3F, 0x06, 0x00, //   ###         ##   ##      ##     ######     ##       ##  ###      #####          ##        ##      ##    ######     ##         
	0x30, 0x01, 0x98, 0x18, 0x1E, 0x04, 0x01, 0xF0, 0x0C, 0x00, 0x20, 0x00, 0x00, 0x1E, 0x04, 0x00, //   ##           ##  ##      ##      ####      #         #####        ##            #                        ####      #          
	0x20, 0x01, 0xB0, 0x30, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //   #            ## ##      ##                            ##                                                                      
	0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                   ##                                                                                                            
	0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                  ###                                                                                                            
	0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x21, 0x24, 0x00, //                 ###                                                               ##   #                  #    #  #  #          
	0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x20, 0x24, 0x00, //                 ##                                                                #    #                  #       #  #          
	0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x19, 0x28, 0xCA, 0x30, 0xE1, 0xC9, 0x06, 0x39, 0x25, 0x20, //                                              ##    ##  #  # #   ##  # #   ##    ###    ###  #  #     ##   ###  #  #  # #  #     
	0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x25, 0x69, 0x2D, 0x49, 0x21, 0x29, 0x09, 0x29, 0x49, 0x20, //                                             #  #  #  # # ## #  #  # ## # #  #  #  #    #  # #  #    #  #  # #  # #  #  #  #     
	0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x45, 0xAB, 0xC8, 0xF2, 0x22, 0x29, 0x10, 0x4A, 0x49, 0x20, //                                            #   # #   # ## # # ####  #   ####  #   #   #   # #  #   #     #  # #  #  #  #  #     
	0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x45, 0xB2, 0x08, 0x82, 0x22, 0x2A, 0x10, 0x4A, 0x49, 0x40, //                                            #   # #   # ## ##  #     #   #     #   #   #   # # #    #     #  # #  #  #  # #      
	0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x49, 0x22, 0x50, 0x92, 0x42, 0x44, 0x11, 0x4A, 0x48, 0x80, //                                            #  #  #  #  #  #   #  # #    #  #  #  #    #  #   #     #   # #  # #  #  #   #       
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x31, 0x21, 0x90, 0x61, 0x81, 0x84, 0x0E, 0x4A, 0x6C, 0x80, //                                            ###    ##   #  #    ##  #     ##    ##      ##    #      ###  #  # #  ## ##  #       
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x80, //                                            #                                                 #                          #       
	0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                           ##                                                                                    
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //                                                                                                                                 
};


void storeSplashScreen(const uint8_t* image)
{
  Preferences preferences;

  DebugPort.println("Storing new splash screen");
  preferences.begin("splashscreen", false);
  preferences.putBytes("image", image, 1024);
  preferences.end();    
}

void loadSplashScreen(uint8_t* image)
{
  Preferences preferences;

  preferences.begin("splashscreen", false);
  int size = preferences.getBytes("image", image, 1024);
  preferences.end();    

  if(size == 0) {
    storeSplashScreen(DieselSplash);
    memcpy(image, DieselSplash, 1024);
  }
}

void checkSplashScreenUpdate()
{
  if(SPIFFS.exists("/splash.bmp")) {  // If a splash.bmp file was uploaded
    File file = SPIFFS.open("/splash.bmp", "rb");          // Open it
    sBMPhdr header;
    file.readBytes((char*)&header, 0x3e);
    do {
      if(header.b0 != 'B' || header.b1 != 'M') {
        DebugPort.println("Bad BMP header");
        break;
      }
      if(header.width != 128 || header.height != 64) {
        DebugPort.println("Bad BMP size");
        break;
      }
      file.seek(header.startofs);
      bool bOK = true;
      uint8_t image[1024];
      uint8_t line[128];
      switch(header.bitsperpixel) {
        case 1:
          DebugPort.println("Reading monochrome bitmap file for splash screen");
          memset(image, 0, 1024);
          for(int i=0; i<64; i++) {
            file.readBytes((char*)line, 16);
            for(int j=0; j < 16; j++)
              line[j] ^= 0xff;    // invert black/white
            memcpy(&image[(63-i)*16], line, 16);
          }
          break;
        case 4:
          DebugPort.println("Reading 16 color bitmap file for splash screen");
          memset(image, 0, 1024);
          for(int i=0; i<64; i++) {
            file.readBytes((char*)line, 64);
            for(int j=0; j < 16; j++) {
              uint8_t packed = 0;
              for(int k=0; k<4; k++) {
                packed <<= 2;
                uint8_t hold = line[k+j*4];
                if((hold & 0xf0) == 0)
                  packed |= 0x02;
                if((hold & 0x0f) == 0)
                  packed |= 0x01;
              }
              line[j] = packed;    
            }
            memcpy(&image[(63-i)*16], line, 16);
          }
          break;
        case 8:
          DebugPort.println("Reading 256 color bitmap file for splash screen");
          memset(image, 0, 1024);
          for(int i=0; i<64; i++) {
            file.readBytes((char*)line, 128);
            for(int j=0; j < 16; j++) {
              uint8_t packed = 0;
              for(int k=0; k<8; k++) {
                packed <<= 1;
                if(line[k+j*8] == 0)
                  packed |= 0x01;
              }
              line[j] = packed;    
            }
            memcpy(&image[(63-i)*16], line, 16);
          }
          break;
        default:
          DebugPort.println("Bad BMP bpp");
          bOK = false;
          break;
      }  

      if(bOK)
        storeSplashScreen(image);

    } while(0);
    
    file.close();
    SPIFFS.remove("/splash.bmp");

    ScreenManager.showSplash();
    delay(2000);
  }
}


CScreenManager::CScreenManager() 
{
  _pDisplay = NULL;
  _menu = -1;
  _subMenu = 0;
  _rootMenu = -1;
  _bReqUpdate = false;
  _DimTime_ms = millis() + 60000;
  _MenuTimeout = millis() + 60000;
  _pRebootScreen = NULL;
  _bDimmed = false;
}

CScreenManager::~CScreenManager()
{
	for(int i=0; i < _Screens.size(); i++) {
  	for(int j=0; j < _Screens[i].size(); j++) {
  		if(_Screens[i][j]) {
			  delete _Screens[i][j];
			  _Screens[i][j] = NULL;
		  }
    }
  }
  if(_pDisplay) {
    delete _pDisplay; _pDisplay = NULL;
  }
}

void 
CScreenManager::begin(bool bNoClock)
{

  // 128 x 64 OLED support (I2C)
  // xxxx_SWITCHCAPVCC = generate display voltage from 3.3V internally
  _pDisplay = new C128x64_OLED(OLED_SDA_pin, OLED_SCL_pin);
#if USE_ADAFRUIT_SH1106 == 1
  _pDisplay->begin(SH1106_SWITCHCAPVCC);
   Wire.begin(OLED_SDA_pin, OLED_SCL_pin, 800000);   // speed up I2C from the default crappy 100kHz set via the adafruit begin!
#elif USE_ADAFRUIT_SSD1306 == 1
  _pDisplay->begin(SSD1306_SWITCHCAPVCC, 0x3c);
  _pDisplay->ssd1306_command(SSD1306_SETPRECHARGE); // 0xd9
  _pDisplay->ssd1306_command(0x1F);  // correct lame reversal of OLED current phases
#endif

  // replace adafruit splash screen
  showSplash();

  delay(2000);

  DebugPort.println("Creating Screens");

  std::vector<CScreen*> menuloop;
  // create root menu loop
  if(NVstore.getUserSettings().NoHeater) {
    menuloop.push_back(new CMenuTrunkScreen(*_pDisplay, *this));
    menuloop.push_back(new CBasicScreen(*_pDisplay, *this));            //  basic control
    if(!bNoClock)
      menuloop.push_back(new CClockScreen(*_pDisplay, *this));          //  clock
    if(getBoardRevision() != 0 && getBoardRevision() != BRD_V2_NOGPIO)            // has GPIO support
      menuloop.push_back(new CGPIOInfoScreen(*_pDisplay, *this));         //  GPIO info
  }
  else {
    menuloop.push_back(new CDetailedScreen(*_pDisplay, *this));         //  detail control
    menuloop.push_back(new CBasicScreen(*_pDisplay, *this));            //  basic control
    if(!bNoClock)
      menuloop.push_back(new CClockScreen(*_pDisplay, *this));          //  clock
    menuloop.push_back(new CPrimingScreen(*_pDisplay, *this));          //  mode / priming
    if(getBoardRevision() != 0 && getBoardRevision() != BRD_V2_NOGPIO)            // has GPIO support
      menuloop.push_back(new CGPIOInfoScreen(*_pDisplay, *this));         //  GPIO info
    menuloop.push_back(new CMenuTrunkScreen(*_pDisplay, *this));
  }
  _Screens.push_back(menuloop);

  // create timer screens loop
  menuloop.clear();
  menuloop.push_back(new CTimerChartScreen(*_pDisplay, *this, 0)); // timer chart
  menuloop.push_back(new CSetTimerScreen(*_pDisplay, *this, 0)); // set timer 1
  menuloop.push_back(new CSetTimerScreen(*_pDisplay, *this, 1)); // set timer 2
  menuloop.push_back(new CSetTimerScreen(*_pDisplay, *this, 2)); // set timer 3
  menuloop.push_back(new CSetTimerScreen(*_pDisplay, *this, 3)); // set timer 4
  menuloop.push_back(new CSetTimerScreen(*_pDisplay, *this, 4)); // set timer 5
  menuloop.push_back(new CSetTimerScreen(*_pDisplay, *this, 5)); // set timer 6
  menuloop.push_back(new CSetTimerScreen(*_pDisplay, *this, 6)); // set timer 7
  menuloop.push_back(new CSetTimerScreen(*_pDisplay, *this, 7)); // set timer 8
  menuloop.push_back(new CSetTimerScreen(*_pDisplay, *this, 8)); // set timer 9
  menuloop.push_back(new CSetTimerScreen(*_pDisplay, *this, 9)); // set timer 10
  menuloop.push_back(new CSetTimerScreen(*_pDisplay, *this, 10)); // set timer 11
  menuloop.push_back(new CSetTimerScreen(*_pDisplay, *this, 11)); // set timer 12
  menuloop.push_back(new CSetTimerScreen(*_pDisplay, *this, 12)); // set timer 13
  menuloop.push_back(new CSetTimerScreen(*_pDisplay, *this, 13)); // set timer 14
  _Screens.push_back(menuloop);

  // create heater tuning screens loop - password protected
  menuloop.clear();
  menuloop.push_back(new CFuelMixtureScreen(*_pDisplay, *this));      //  mixture tuning
  menuloop.push_back(new CHeaterSettingsScreen(*_pDisplay, *this));   // heater system tuning
  menuloop.push_back(new CFuelCalScreen(*_pDisplay, *this));          // fuel pump calibration
  _Screens.push_back(menuloop);

  // create User Settings screens loop 
  menuloop.clear();
  if(NVstore.getUserSettings().NoHeater) {
    menuloop.push_back(new CNoHeaterHomeMenuSelScreen(*_pDisplay, *this)); // No Heater Home menu settings screen
    if(getBoardRevision() != 0 && getBoardRevision() != BRD_V2_NOGPIO)   // has GPIO support ?
      menuloop.push_back(new CGPIOSetupScreen(*_pDisplay, *this)); // GPIO settings screen
  }
  else {
    menuloop.push_back(new CThermostatModeScreen(*_pDisplay, *this)); // thermostat settings screen
    menuloop.push_back(new CHomeMenuSelScreen(*_pDisplay, *this)); // Home menu settings screen
    menuloop.push_back(new CTimeoutsScreen(*_pDisplay, *this)); // Other options screen
    if(getBoardRevision() != 0 && getBoardRevision() != BRD_V2_NOGPIO)   // has GPIO support ?
      menuloop.push_back(new CGPIOSetupScreen(*_pDisplay, *this)); // GPIO settings screen
  }
  _Screens.push_back(menuloop);

  // create System Settings screens loop 
  menuloop.clear();
  menuloop.push_back(new CVersionInfoScreen(*_pDisplay, *this)); // GPIO settings screen
  if(!NVstore.getUserSettings().NoHeater) {
    menuloop.push_back(new CHourMeterScreen(*_pDisplay, *this)); // Hour Meter screen
  }
  menuloop.push_back(new CWiFiScreen(*_pDisplay, *this));
  menuloop.push_back(new CMQTTScreen(*_pDisplay, *this));
  menuloop.push_back(new CBTScreen(*_pDisplay, *this));
  _Screens.push_back(menuloop);

  // create branch screens
  menuloop.clear();
  menuloop.push_back(new CSetClockScreen(*_pDisplay, *this));         // clock set branch screen
  menuloop.push_back(new CInheritSettingsScreen(*_pDisplay, *this));  // inherit OEM settings branch screen
  menuloop.push_back(new CFontDumpScreen(*_pDisplay, *this)); // font dump branch screen
  menuloop.push_back(new CSettingsScreen(*_pDisplay, *this));         //  Tuning info
  _Screens.push_back(menuloop);

  _menu = 0;
#if RTC_USE_DS3231==0 && RTC_USE_DS1307==0 && RTC_USE_PCF8523==0
  _rootMenu = 2;   // bring up clock set screen first if using millis based RTC!
  _subMenu = 2;
#else
	_rootMenu = 1;   // basic control screen
  _subMenu = 1;
#endif

  _enterScreen();
}

bool 
CScreenManager::checkUpdate()
{
  long dimTimeout = NVstore.getUserSettings().dimTime;

  // manage dimming or blanking the display, according to user defined inactivity interval
  if(dimTimeout && _DimTime_ms) {
    long tDelta = millis() - _DimTime_ms;
    if(tDelta > 0) {
      // time to dim the display
      _dim(true);
      _DimTime_ms = 0;

      if(dimTimeout < 0) {
        _pDisplay->clearDisplay();
        _pDisplay->display();   // blank screen
      }
    }
  }

  if(NVstore.getUserSettings().menuTimeout && _MenuTimeout) {
    long tDelta = millis() - _MenuTimeout;
    if(tDelta > 0) {
      _MenuTimeout = 0;
      // we will be blanking the display, transit through a dim stage first
      if(dimTimeout < 0)
        _dim(true);

      _leaveScreen();
      // fall back to main menu 
      selectMenu(RootMenuLoop);
      // upon dim timeout - sticky root menu screens are the first 3 in the list:
      //   Detailed Control
      //   Basic Control
      //   Clock
      // return to those upon timeout, otherwise return to Basic Control screen
      if((_rootMenu > 2) || ((_rootMenu == 0) && NVstore.getUserSettings().NoHeater)) {
        uint8_t userHomeMenu = NVstore.getUserSettings().HomeMenu.onTimeout;
        if(userHomeMenu) {  // allow user to override defualt screen
          userHomeMenu--;
          DebugPort.print("Screen Manager: Menu timeout, falling back to user preferred screen: "); 
          switch(userHomeMenu) {
            case 0: DebugPort.println("Detailed control menu"); break;
            case 1: DebugPort.println("Basic control menu"); break;
            case 2: DebugPort.println("Clock menu"); break;
          }
          _rootMenu = _subMenu = userHomeMenu;  
        }
        else {
          _rootMenu = _subMenu = 1;
          DebugPort.println("Screen Manager: Menu timeout, falling back to Basic control screen"); 
        }
      }
      _enterScreen();
    }
  }  
  
  static int prevRunState = -1;
  int runState = getHeaterInfo().getRunStateEx(); 
  if(runState != prevRunState) {
    if(runState > 0 && prevRunState == 0) {
      // heater has started
      uint8_t userStartMenu = NVstore.getUserSettings().HomeMenu.onStart;
      if(userStartMenu && userStartMenu <= 3) {  // allow user to override default screen
        userStartMenu--;
        DebugPort.print("Screen Manager: Heater start detected, switching to user preferred screen: "); 
        switch(userStartMenu) {
          case 0: DebugPort.println("Detailed control menu"); break;
          case 1: DebugPort.println("Basic control menu"); break;
          case 2: DebugPort.println("Clock menu"); break;
        }
        _rootMenu = _subMenu = userStartMenu;  
        _enterScreen();
      }
    }
    if(runState == 0 && prevRunState > 0) {
      // heater has stopped
      uint8_t userStopMenu = NVstore.getUserSettings().HomeMenu.onStop;
      if(userStopMenu && userStopMenu <= 3) {  // allow user to override default screen
        userStopMenu--;
        DebugPort.print("Screen Manager: Heater stop detected, switching to user preferred screen: "); 
        switch(userStopMenu) {
          case 0: DebugPort.println("Detailed control menu"); break;
          case 1: DebugPort.println("Basic control menu"); break;
          case 2: DebugPort.println("Clock menu"); break;
        }
        _rootMenu = _subMenu = userStopMenu;  
        _enterScreen();
      }
    }
    prevRunState = runState;
  }


  if(_bReqUpdate) {
    if((dimTimeout < 0) && (_DimTime_ms == 0)) {
      // no screen updates, we should be blanked!
    }
    else {
      if(_pRebootScreen) {
        _pRebootScreen->show();
        _bReqUpdate = false;
        return true;
      }
      else {
        if(_menu >= 0) {
          _Screens[_menu][_subMenu]->show();
          _bReqUpdate = false;
          return true;
        }
      }
    }
  }
  return false;
}
  
void 
CScreenManager::reqUpdate()
{
  _bReqUpdate = true;
}

bool 
CScreenManager::animate()
{
  if(_pRebootScreen) 
  	return false;

  if((NVstore.getUserSettings().dimTime < 0) && (_DimTime_ms == 0)) 
    // no screen updates, we should be blanked!
    return false;
  
  if(_menu >= 0) 
    return _Screens[_menu][_subMenu]->animate();
    
	return false;
}

void
CScreenManager::refresh()
{
	if(_pDisplay)
		_pDisplay->display();
}

void 
CScreenManager::_enterScreen()
{
  if(_menu >= 0) 
    _Screens[_menu][_subMenu]->onSelect();
		
  reqUpdate();
}

void
CScreenManager::_leaveScreen()
{
  if(_menu >= 0) 
    _Screens[_menu][_subMenu]->onExit();
}

void
CScreenManager::_changeSubMenu(int dir)
{
  _leaveScreen();
  _subMenu += dir;
  int bounds = _Screens[_menu].size() - 1;
  WRAPLIMITS(_subMenu, 0, bounds);
  if(_menu == 0)
    _rootMenu = _subMenu;  // track the root menu for when we branch then return
  _enterScreen();
}

void 
CScreenManager::nextMenu()
{
  if(_menu >= 0 && _menu != BranchMenu) {
    _changeSubMenu(+1);
  }
}

void 
CScreenManager::prevMenu()
{
  if(_menu >= 0 && _menu != BranchMenu) {
    _changeSubMenu(-1);
  }
}

void 
CScreenManager::keyHandler(uint8_t event)
{
  if(_bDimmed) {
    if(event & keyReleased) {
      _dim(false);
      bumpTimeout();
    }
    return;   // initial press when dimmed is always thrown away
  }

  bumpTimeout();

  // call key handler for active screen
  if(_menu >= 0) 
    _Screens[_menu][_subMenu]->keyHandler(event);
}

void
CScreenManager::bumpTimeout()
{
  long dimTime = NVstore.getUserSettings().dimTime;
  _DimTime_ms = (millis() + abs(dimTime)) | 1;
  _MenuTimeout = (millis() + NVstore.getUserSettings().menuTimeout) | 1;
}


void
CScreenManager::selectMenu(eUIMenuSets menuSet, int specific)
{
  _leaveScreen();
  if(_menu >= 0) {   // only true once we have created the screens
    _menu = menuSet;
    if(specific >= 0) {
      // targetting a specific menu
      _subMenu = specific;
      UPPERLIMIT(_subMenu, _Screens[_menu].size()-1);  // check bounds!
      DebugPort.printf("selectMenu %d %d\r\n", _menu, _subMenu);
    }
    else {
      // default sub menu behaviour
      if(_menu == 0)
        _subMenu = _rootMenu; // return to last used root menu
      else
        _subMenu = 0;  // branches always go to first sub menu
    }
  }
  _enterScreen();
}

void 
CScreenManager::showRebootMsg(const char* content[2], long delayTime)
{
  if(_pRebootScreen == NULL)
    _pRebootScreen = new CRebootScreen(*_pDisplay, *this);

  _pRebootScreen->setMessage(content, delayTime);
  _bReqUpdate = true;
  _dim(false);
}

void 
CScreenManager::clearDisplay()
{
  _pDisplay->clearDisplay();
}
  
void 
CScreenManager::showOTAMessage(int percent, eOTAmodes updateType)
{
  static int prevPercent = -1;

  if(percent != prevPercent) {
    prevPercent = percent;
    _pDisplay->clearDisplay();
    if(percent < 0)
      return;
      
    _pDisplay->setFontInfo(&arial_8ptBoldFontInfo);
    _pDisplay->setCursor(64, -1);
    _pDisplay->printCentreJustified("Firmware update");
    _pDisplay->setFontInfo(NULL);
    _pDisplay->drawFastHLine(0, 10, 128, WHITE);
    _pDisplay->setCursor(64,22);
    switch(updateType) {
      case eOTAnormal:  _pDisplay->printCentreJustified("OTA upload");     break;
      case eOTAbrowser: _pDisplay->printCentreJustified("Browser upload"); break;
      case eOTAWWW:     _pDisplay->printCentreJustified("Web download");     break;
    }
    if(percent) {
      _pDisplay->drawRect(14, 32, 100, 8, WHITE);
      _pDisplay->fillRect(14, 32, percent, 8, WHITE);
      char msg[16];
      sprintf(msg, "%d%%", percent);
      _pDisplay->setCursor(64,42);
      _pDisplay->printCentreJustified(msg);
    }
    _pDisplay->display();
  }
}

void
CScreenManager::_dim(bool state)
{
  _bDimmed = state;
  _pDisplay->dim(state);
}

void
CScreenManager::showSplash()
{
  _pDisplay->clearDisplay();
  uint8_t splash[1024];
  loadSplashScreen(splash);
  _pDisplay->drawBitmap(0, 0, splash, 128, 64, WHITE);
//  _pDisplay->drawBitmap(0, 0, DieselSplash, 128, 64, WHITE);
  _pDisplay->setCursor(90, 56);
  CTransientFont AF(*_pDisplay, &segoeUI_Italic_7ptFontInfo);  // temporarily use a midi font
  _pDisplay->setTextColor(WHITE);
  _pDisplay->print(getVersionStr());

  // Show initial display buffer contents on the screen --
  _pDisplay->display();
}
