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

// 'Thermometer', 8x50px
#define W_BULB_ICON 8
#define H_BULB_ICON 50
extern const unsigned char ambientThermometerIcon [];
extern const unsigned char bodyThermometerIcon [];

// 'ThermoPtr', 3x5px
#define W_PTR_ICON 3
#define H_PTR_ICON 5
extern const unsigned char thermoPtr [];

// 'Bluetooth icon', 6x11px
#define W_BT_ICON  6
#define H_BT_ICON 11
extern const unsigned char BTicon [];

// 'wifiIcon', 13x10px
#define W_WIFI_ICON 13
#define H_WIFI_ICON 10
extern const unsigned char wifiIcon [];

// 'wifiInIcon', 5x5px
#define W_WIFIIN_ICON 5
#define H_WIFIIN_ICON 5
extern const unsigned char wifiInIcon [];

// 'wifiOutIcon', 5x5px
#define W_WIFIOUT_ICON 5
#define H_WIFIOUT_ICON 5
extern const unsigned char wifiOutIcon [];

// 'BatteryIcon', 15x10px
#define W_BATT_ICON 15
#define H_BATT_ICON 10
extern const unsigned char BatteryIcon [];

// 'GlowPlugIcon', 16x9px
#define W_GLOW_ICON 16
#define H_GLOW_ICON 9
extern const unsigned char GlowPlugIcon [];

// 'HeatRise', 17x2px
#define W_HEAT_ICON 17
#define H_HEAT_ICON 2
extern const unsigned char GlowHeatIcon [];

#define W_FAN_ICON 16
#define H_FAN_ICON 16
// 'Fan3_1a', 16x16px
extern const unsigned char FanIcon1 [];
// 'Fan3_2a', 16x16px
extern const unsigned char FanIcon2 [];
// 'Fan3_3a', 16x16px
extern const unsigned char FanIcon3 [];
// 'Fan3_4a', 16x16px
extern const unsigned char FanIcon4 [];


// 'FuelIcon', 7x12px
#define W_FUEL_ICON 7
#define H_FUEL_ICON 12
extern const unsigned char FuelIcon [];

// 'Target', 13x13px
#define W_TARGET_ICON 13
#define H_TARGET_ICON 13
extern const unsigned char TargetIcon [];

#define W_TIMER_ICON 15
#define H_TIMER_ICON 15
extern const unsigned char repeatIcon [];
extern const unsigned char timerID1Icon [];
extern const unsigned char timerID2Icon [];
extern const unsigned char timerIcon [];
extern const unsigned char largeTimerIcon [];
extern const uint8_t verticalRepeatIcon [];
// Bitmap sizes for verticalRepeat
const uint8_t verticalRepeatWidthPixels = 6;
const uint8_t verticalRepeatHeightPixels = 15;

