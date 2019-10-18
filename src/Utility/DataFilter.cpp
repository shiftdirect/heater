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

#include "DataFilter.h"
#include <math.h>
#include "macros.h"

CExpMean::CExpMean()
{
  reset(0);
  setAlpha(0.95);
  setRounding(0.1);
  setBounds(0, 0);
}

void
CExpMean::reset(float val)
{
  _bFresh = true;
  _val = val;
}

void 
CExpMean::update(float val)
{
  bool boundsOK = INBOUNDS(val, bounds.lower, bounds.upper);
  if(bounds.lower == 0 && bounds.upper == 0)
    boundsOK = true;  // if bounds were not defined

  if(boundsOK) {
    if(_bFresh) {
      _val = val;
      _bFresh = false;
    }

    _val = _val * _Alpha + val * (1-_Alpha);
  }
}

float
CExpMean::getValueRaw() const
{ 
  return _val;
}

float
CExpMean::getValue() const
{ 
  int round = int(_val * _roundingRecip + 0.5);
  return float(round) * _rounding;
}

void  
CExpMean::setRounding(float val)
{
  if(val > 0) {
    _rounding = val;
    _roundingRecip = 1.0 / val;  // divides are expensive, prep!
  }
}

void  
CExpMean::setAlpha(float val)
{
  _Alpha = val;
}

void
CExpMean::setBounds(float lower, float upper)
{
  bounds.lower = lower;
  bounds.upper = upper;
}