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
#include <driver/adc.h>
#include "BTCConfig.h"


const gpio_num_t UART_Tx = GPIO_NUM_1;
const gpio_num_t LED_Pin = GPIO_NUM_2;
const gpio_num_t UART_Rx = GPIO_NUM_3;
const gpio_num_t HC05_KeyPin = GPIO_NUM_4;
const gpio_num_t TxEnbPin = GPIO_NUM_5;
const gpio_num_t Tx433MHz_pin = GPIO_NUM_12;      // HSPI std pins
const gpio_num_t Rx433MHz_pin = GPIO_NUM_13;      //  "
const gpio_num_t GPIOout2_pin = GPIO_NUM_14;      //  "
#if USE_JTAG == 1
const gpio_num_t DS18B20_Pin = GPIO_NUM_33; 
#else
const gpio_num_t DS18B20_Pin = GPIO_NUM_15; 
#endif
const gpio_num_t Rx1Pin = GPIO_NUM_16;
const gpio_num_t Tx1Pin = GPIO_NUM_17;
const gpio_num_t Tx2Pin = GPIO_NUM_18;
const gpio_num_t Rx2Pin = GPIO_NUM_19;
const gpio_num_t OLED_SDA_pin = GPIO_NUM_21;     // I2C std pins
const gpio_num_t OLED_SCL_pin = GPIO_NUM_22;     //  "
const gpio_num_t HC05_SensePin = GPIO_NUM_23;
const gpio_num_t GPIOin2_pin = GPIO_NUM_25;
const gpio_num_t GPIOin1_pinV21V10 = GPIO_NUM_26;
const adc2_channel_t GPIOalg_pinINVALID = ADC2_CHANNEL_9; // GPIO 26 - Cannot use ADC2 with WiFi enabled!!!
const uint8_t GPIOout1_pin = GPIO_NUM_27;

const gpio_num_t keyUp_pin = GPIO_NUM_32;
const gpio_num_t GPIOin1_pinV20 = GPIO_NUM_33;
const adc1_channel_t GPIOalg_pin = ADC1_CHANNEL_5; // GPIO 33 - OK with Wifi, ADC1 channel
const gpio_num_t keyDown_pin = GPIO_NUM_34;      // input only, no chip pullup
const gpio_num_t keyCentre_pin = GPIO_NUM_35;    // input only, no chip pullup
const gpio_num_t keyRight_pin = GPIO_NUM_36;     // input only, no chip pullup
const gpio_num_t keyLeft_pin = GPIO_NUM_39;      // input only, no chip pullup


