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

#include "NVStorage.h"


class CMQTTsetup {
  char _buffer[128];
  int _idx;
  int _mode;
  bool _active;
  sMQTTparams _MQTTsetup;
  bool HandleMQTTsetup(char rxVal);
  void showMQTTmenu(bool init = false);
  bool getMQTTstring(char rxVal, int maxidx, char* pTargetString);
public:
  CMQTTsetup();
  bool Handle(char& rxVal);
  void setActive();
};
