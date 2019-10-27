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

//#include "../../lib/TelnetSpy/TelnetSpy.h"
#include "ABTelnetSpy.h"

#ifndef __DEBUGPORT_H__
#define __DEBUGPORT_H__

class CProtocol;

extern ABTelnetSpy DebugPort;

void DebugReportFrame(const char* hdr, const CProtocol& Frame, const char* ftr);

#endif // __DEBUGPORT_H__
