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

#ifndef __BTC_DATAFILTER_H__
#define __BTC_DATAFILTER_H__

class CExpMean {
  float _val;
  bool _bFresh;
  float _Alpha;
  float _rounding;
  float _roundingRecip;
public:
  CExpMean();
  void  reset(float val);
  void  update(float val);
  void  setRounding(float val);
  void  setAlpha(float val);
  float getValue() const;
  float getValueRaw() const;
};

struct sFilteredData {
  CExpMean ipVolts;
  CExpMean GlowVolts;
  CExpMean GlowAmps;
  CExpMean Fan;
  CExpMean AmbientTemp;
};

#endif
