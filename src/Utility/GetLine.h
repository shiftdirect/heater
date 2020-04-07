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

#ifndef __GETSTRINGLINE_H__
#define __GETSTRINGLINE_H__

class CGetLine {
  char _buffer[128];
  int _idx;
  int _getMode;   // 0: default string 1: targetted string, 2: numeric
  int _maxlen;
  char* _pTarget;
  int _Numeric;
  bool _showStars;
  bool _doNum(char rxVal);
  void _copyTarget();
  void _zeroTarget();
public:
  CGetLine();
  void reset();
  void reset(char* result, int maxlen);
  void reset(int numeric);
  bool handle(char rxVal);
  int getNumeric() { return _Numeric; }
  const char* getString() { return _buffer; }
  int getLen() { return _idx; }
  void maskEntry() { _showStars = true; };
};

#endif