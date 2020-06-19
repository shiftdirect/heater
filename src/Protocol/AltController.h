/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2020  Ray Jones <ray@mrjones.id.au>
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
#ifndef __ALTCONTROLLER_H__
#define __ALTCONTROLLER_H__

#include <FreeRTOS.h>
#include "../Utility/UtilClasses.h"

// struct sAltHeaterData {
//   unsigned long Active;
//   bool On;
//   bool Stopping;
//   bool Glow;
//   bool Fan;
//   bool Pump;
//   bool Plateau;
//   int  Demand;
//   int  BodyT;
//   int  Volts;
//   int  Error;
//   int  pumpRate[6];
//   sAltHeaterData();
//   int  runState();
//   int  errState();
//   float  getPumpRate();
// };

// void  decodeAltHeater(int rxdata);

// void reqAltPower();
// void reqAltStatus();
// void reqAltVolts();
// void reqAltBodyT();
// bool isAltActive();
// void checkAltRxEvents();
// void checkAltTxEvents();



// extern sAltHeaterData AltHeaterData;

#endif