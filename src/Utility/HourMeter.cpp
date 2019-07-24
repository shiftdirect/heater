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

#include "HourMeter.h"
#include "NVStorage.h"
#include "../RTC/RTCStore.h"
#include "../RTC/Clock.h"
#include "../Protocol/Protocol.h"


void 
CHourMeter::powerOnInit()
{
  _RunTime = 0;
  _GlowTime = 0;
}

void 
CHourMeter::monitor(const CProtocol& frame)
{
  Clock.get().secondstime();
}

unsigned long 
CHourMeter::getRunTime()
{
  return 0;
}

unsigned long 
CHourMeter::getGlowTime()
{
  return 0;
}

