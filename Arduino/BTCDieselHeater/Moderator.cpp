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

#include "Moderator.h"

void 
CModerator::shouldSend(bool set)
{
	_bShouldSend = false;
}

bool 
CModerator::shouldSend()
{
	return _bShouldSend;
}

void
CModerator::reset() 
{
  // install invalid values, retain maps (memory defrag reasons)
	for(auto it = fMemory.begin(); it != fMemory.end(); ++it)  it->second = -100;
	for(auto it = iMemory.begin(); it != iMemory.end(); ++it)  it->second = -100;
	for(auto it = cMemory.begin(); it != cMemory.end(); ++it)  it->second = -100;
}

bool
CModerator::shouldSend(const char* name, float value) 
{
	bool retval = true;
	auto it = fMemory.find(name);
  if(it != fMemory.end()) {
		retval = it->second != value;
		it->second = value;
	}
	else {
		fMemory[name] = value;
	}
  _bShouldSend |= retval;
	return retval;
}

bool
CModerator::shouldSend(const char* name, int value) 
{
	bool retval = true;
	auto it = iMemory.find(name);
  if(it != iMemory.end()) {
		retval = it->second != value;
		it->second = value;
	}
	else {
		iMemory[name] = value;
	}
  _bShouldSend |= retval;
	return retval;
}

bool
CModerator::shouldSend(const char* name, unsigned char value) 
{
	bool retval = true;
	auto it = cMemory.find(name);
  if(it != cMemory.end()) {
		retval = it->second != value;
		it->second = value;
	}
	else {
		cMemory[name] = value;
	}
  _bShouldSend |= retval;
	return retval;
}
