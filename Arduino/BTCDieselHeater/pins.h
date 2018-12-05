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

#include <stdint.h>

const uint8_t LED_Pin = 2;
const uint8_t HC05_KeyPin = 4;
const uint8_t TxEnbPin = 5;
const uint8_t OLED_MISO_pin = 12;    // HSPI std pins
const uint8_t OLED_MOSI_pin = 13;    //  "
const uint8_t OLED_CLK_pin = 14;     //  "
const uint8_t DS18B20_Pin = 15; 
const uint8_t Rx1Pin = 16;
const uint8_t Tx1Pin = 17;
const uint8_t Rx2Pin = 18;
const uint8_t Tx2Pin = 19;
const uint8_t OLED_SDA_pin = 21;     // I2C std pins
const uint8_t OLED_SCL_pin = 22;     //  "
const uint8_t HC05_SensePin = 23;
const uint8_t OLED_DC_pin = 26;
const uint8_t OLED_CS_pin = 27;

const uint8_t keyLeft_pin = 25;
const uint8_t keyRight_pin = 34;
const uint8_t keyCentre_pin = 33;
const uint8_t keyUp_pin = 35;
const uint8_t keyDown_pin = 32;

const uint8_t ListenOnlyPin = 36;    // input only, no chip pullup
const uint8_t WiFi_TriggerPin = 39;  // input only, no chip pullup

