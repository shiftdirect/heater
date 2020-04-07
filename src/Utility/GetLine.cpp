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

#include "GetLine.h"
#include "DebugPort.h"
#include <string.h>

CGetLine::CGetLine()
{
  reset();
}

void
CGetLine::reset()
{
  _idx = 0;
  _getMode = 0;
  _pTarget = NULL;
  _Numeric = 0;
  _maxlen = sizeof(_buffer);
  _showStars = false;
}

void 
CGetLine::reset(char* result, int maxlen)
{
  reset();
  _getMode = 1;
  _pTarget = result;
  _maxlen = maxlen > sizeof(_buffer) ? sizeof(_buffer) : maxlen;
}
  
void 
CGetLine::reset(int numeric)
{
  reset();
  _getMode = 2;
  _Numeric = numeric;
}

bool 
CGetLine::handle(char rxVal)
{
  if(_getMode == 2) {
    return _doNum(rxVal);
  }

  if(rxVal < ' ') {
    if(_idx == 0) {
      if(_pTarget) 
        strcpy(_buffer, _pTarget);
    }
    if(rxVal == ('x' & 0x1f)) {  // CTRL-X - erase string, return done
      memset(_buffer, 0, sizeof(_buffer));
      _zeroTarget();
      return true;
    }
    if(rxVal == '\r')   // ignore CR
      return false;
    if(rxVal == '\n') {  // accept buffered string upon LF, return done
      _copyTarget();
      return true;
    }
    if(rxVal == 0x1b) {  // abort, no change upon ESC, return done
      return true;
    }
  }
  else {
    if(_idx == 0) memset(_buffer, 0, sizeof(_buffer));
    if(_showStars)
      DebugPort.print('*');
    else
      DebugPort.print(rxVal);
    _buffer[_idx++] = rxVal;
    if(_idx == _maxlen) {
      _copyTarget();
      return true;
    }
  }
  return false;
}


bool
CGetLine::_doNum(char rxVal)
{
  if(rxVal < ' ') {
    if(_idx == 0) sprintf(_buffer, "%d", _Numeric);
    if(rxVal == '\n') {
      int val = atoi(_buffer);
      _Numeric = val;
      return true;
    }
    if(rxVal == 0x1b) {
      return true;
    }
    return false;
  }
  DebugPort.print(rxVal);
  if(isdigit(rxVal)) {
    if(_idx == 0) memset(_buffer, 0, sizeof(_buffer));
    _buffer[_idx++] = rxVal;
    if(_idx == 5) {
      int val = atoi(_buffer);
      _Numeric = val;
      return true;
    }
  }
  else {
    return true;
  }
  return false;
}

void
CGetLine::_copyTarget()
{
  if(_pTarget) {
    strncpy(_pTarget, _buffer, _maxlen);
    _pTarget[_maxlen] = 0;
  }
}


void
CGetLine::_zeroTarget()
{
  if(_pTarget) 
    memset(_pTarget, 0, _maxlen+1);
}

