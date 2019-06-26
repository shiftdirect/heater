/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2018  Ray Jones <ray@mrjones.id.au>
 * Copyright (C) 2018  James Clark
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


// Place Holder Config File - User config vars and defines to be moved here

//////////////////////////////////////////////////////////////////////////////
// Configure bluetooth options
// ** Recommended to use HC-05 for now **
// If none are enabled, we'll use an abstract class that only reports 
// to the debug port what would  have been sent
//
#define USE_HC05_BLUETOOTH     1
#define USE_BLE_BLUETOOTH      0
#define USE_CLASSIC_BLUETOOTH  0

//////////////////////////////////////////////////////////////////////////////
// Configure WiFi options
//
// *** Presently ESP32 Bluetooth and WiFi do not co-exist well (ala don't work!) ***
//     HC-05 works OK with WiFi
//
#define USE_WIFI      1
#define USE_OTA       1
#define USE_WEBSERVER 1
#define USE_SPIFFS    1  

#define USE_PORTAL_TRIGGER_PIN 0



///////////////////////////////////////////////////////////////////////////////
// debug reporting options
//
// true: each frame of data is reported on a new lines
// false: controller, then heater response frmaes are reported on a single line (excel CSV worthy!)
//
#define REPORT_RAW_DATA    0              /* can toggle using <B> from debug console */
#define TERMINATE_OEM_LINE 0              /* when an OEM controller exists */
#define TERMINATE_BTC_LINE 0              /* when an OEM controller does not exist */
#define REPORT_OEM_RESYNC  0              /* report idle time if OEM controller detected */
#define REPORT_STATE_MACHINE_TRANSITIONS 0  /* state machine changes reported to DebugPort */
#define REPORT_BLUEWIRE_RECYCLES 1        /* best default on as this is abnormal behaviour can toggle using <W> on debug console */

///////////////////////////////////////////////////////////////////////////////
// LED monitoring
//
//   1: enable specific LED function
//   0: disable specific LED function
//
#define RX_LED  1   /* flash when receiving blue wire data */
#define BT_LED  0   /* flash when sending bluetooth data */


///////////////////////////////////////////////////////////////////////////////
//  DS18B20 temperature sensing
//
#define MIN_TEMPERATURE_INTERVAL 500

///////////////////////////////////////////////////////////////////////////////
// Real Time Clock support
//
// only select one option to use the indicated hardware
// if none are selected, RTC_Millis will be used, which is volatile (based upon millis())
//
#define RTC_USE_DS3231  1
#define RTC_USE_DS1307  0
#define RTC_USE_PCF8523 0
  

///////////////////////////////////////////////////////////////////////////////
// Blue wire handling
//
#define SUPPORT_OEM_CONTROLLER 1           /* 0=we send without regard to an OEM's data, 1=co-exist with OEM controller */

///////////////////////////////////////////////////////////////////////////////
// Communications reporting
//
#define REPORT_JSON_TRANSMIT 1    

///////////////////////////////////////////////////////////////////////////////
// Adafruit OLED selection
//
#define USE_ADAFRUIT_SH1106  1    
#define USE_ADAFRUIT_SSD1306 0


///////////////////////////////////////////////////////////////////////////////
// Protocol exploration
//
//#define PROTOCOL_INVESTIGATION

///////////////////////////////////////////////////////////////////////////////
// Software based watchdog
//
#define USE_SW_WATCHDOG 1

