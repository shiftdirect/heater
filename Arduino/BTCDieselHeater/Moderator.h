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

#ifndef __BTC_MODERATOR_H__
#define __BTC_MODERATOR_H__

#include <map>

class CModerator {
	bool _bShouldSend;
  std::map<const char*, float> fMemory;
  std::map<const char*, int> iMemory;
  std::map<const char*, unsigned char> cMemory;
public:
	void shouldSend(bool reset);
	bool shouldSend();
  bool shouldSend(const char* name, float value);
  bool shouldSend(const char* name, int value);
  bool shouldSend(const char* name, unsigned char value);
	void reset();
};

#endif // __BTC_MODERATOR_H__
