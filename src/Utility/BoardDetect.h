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

// **** NOTE: internal codes do not match user codes ****
// internal codes follow the evoloution of discovering that the intial analog input
// could not be used and had to be swapped with GPIOin1 :-(
// the the easier to build PCB with no GPIO!
// The User IDs make more logical sense:
//   V2.0 base no GPIO
//   V2.1 only digital IO 
//   V2.2 full GPIO
#define BRD_V1_FULLGPIO    10
#define BRD_V2_GPIO_NOALG  20   // original V20 board - no cut traces - analog on wrong pin :-(
#define BRD_V2_FULLGPIO    21   // V2 board cut traces tranposed GPIO1 & Analog
#define BRD_V2_NOGPIO      22   // V2 board forced short on analog input - NO GPIO mode
#define BRD_V3_GPIO_NOALG  30   // V3 board - no cut traces - analog input grounded

int BoardDetect();
void BoardRevisionReset();
const char* getBoardRevisionString(int ID);
