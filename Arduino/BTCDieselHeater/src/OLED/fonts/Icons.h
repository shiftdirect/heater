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
extern const uint8_t GPIO1OFFIcon[];
extern const uint8_t GPIO1ONIcon[];
extern const uint8_t GPIO2OFFIcon[];
extern const uint8_t GPIO2ONIcon[];
extern const uint8_t CrossIcon[];
extern const uint8_t TickIcon[];
// Bitmap sizes for verticalRepeat
const uint8_t verticalRepeatWidthPixels = 6;
const uint8_t verticalRepeatHeightPixels = 15;

// Bitmap sizes for GPIOIcons
const uint8_t GPIOIconWidthPixels = 9;
const uint8_t GPIOIconHeightPixels = 9;

// Bitmap sizes for TickIcons
const uint8_t TickIconWidth = 5;
const uint8_t TickIconHeight = 5;

// Bitmap for open
extern const uint8_t OpenIcon[];
const uint8_t OpenIconWidth = 13;
const uint8_t OpenIconHeight = 7;

// Bitmap for close
extern const uint8_t CloseIcon[];
const uint8_t CloseIconWidth = 13;
const uint8_t CloseIconHeight = 7;

// Bitmap for BulbOn
extern const uint8_t BulbOnIcon[];
const uint8_t BulbOnIconWidth = 9;
const uint8_t BulbOnIconHeight = 8;

// Bitmap for BulbOff
extern const uint8_t BulbOffIcon[];
const uint8_t BulbOffIconWidth = 9;
const uint8_t BulbOffIconHeight = 8;

// Bitmap for start
extern const uint8_t startIcon[];
const uint8_t startWidth = 5;
const uint8_t startHeight = 9;

// Bitmap sizes for stop
extern const uint8_t stopIcon[];
const uint8_t stopWidth = 6;
const uint8_t stopHeight = 8;

// Bitmap for displayTimeout
extern const uint8_t displayTimeoutIcon[];
const uint8_t displayTimeoutWidth = 24;
const uint8_t displayTimeoutHeight = 10;

// Bitmap for menuTimeout
extern const uint8_t menuTimeoutIcon[];
const uint8_t menuTimeoutWidth = 24;
const uint8_t menuTimeoutHeight = 10;

// Bitmap for timeout
extern const uint8_t timeoutIcon[];
const uint8_t timeoutWidth = 9;
const uint8_t timeoutHeight = 10;

// Bitmap for refresh
extern const uint8_t refreshIcon[];
const uint8_t refreshWidth = 13;
const uint8_t refreshHeight = 11;

// Bitmap for thermostat modes
extern const uint8_t thermostatIcon[];
const uint8_t thermostatWidth = 28;
const uint8_t thermostatHeight = 34;

// Bitmap for gPIO
extern const uint8_t GPIOIcon[];
const uint8_t GPIOWidth = 20;
const uint8_t GPIOHeight = 33;

// Bitmap for firmware
extern const uint8_t firmwareIcon[];
const uint8_t firmwareWidth = 26;
const uint8_t firmwareHeight = 21;

// Bitmap for hardware
extern const uint8_t hardwareIcon[];
const uint8_t hardwareWidth = 16;
const uint8_t hardwareHeight = 15;

