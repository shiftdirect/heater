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

class CProtocolPackage;
class BTCDateTime;


extern void ToggleOnOff();
extern void requestOn();
extern void requestOff();
extern void reqTempChange(int val);
extern void reqThermoToggle();
extern void setThermostatMode(unsigned char);
extern void reqPumpPrime(bool on);
extern float getActualTemperature();
extern void  setPumpMin(float);
extern void  setPumpMax(float);
extern void  setFanMin(short);
extern void  setFanMax(short);
extern void  saveNV();
extern const CProtocolPackage& getHeaterInfo();
extern const BTCDateTime& getCurrentTime();

#define LOWERLIMIT(A, B) if(A < B) A = B
#define UPPERLIMIT(A, B) if(A > B) A = B
#define ROLLUPPERLIMIT(A, B, C) if(A > B) A = C        
#define ROLLLOWERLIMIT(A, B, C) if(A < B) A = C        
