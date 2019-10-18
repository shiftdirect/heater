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
#include "DebugPort.h"
#include "MQTTsetup.h"

CMQTTsetup::CMQTTsetup()
{
  _active = false;
}

void 
CMQTTsetup::setActive()
{
  _active = true;
  showMQTTmenu(true);
}


void 
CMQTTsetup::showMQTTmenu(bool init)
{
  if(init)
    _MQTTsetup = NVstore.getMQTTinfo();

  DebugPort.print("\014");
  DebugPort.println("MQTT broker configuration");
  DebugPort.println("");
  DebugPort.printf("  <1> - set IP address, currently \"%s\"\r\n", _MQTTsetup.host);
  DebugPort.printf("  <2> - set port, currently %d\r\n", _MQTTsetup.port);
  DebugPort.printf("  <3> - set username, currently \"%s\"\r\n", _MQTTsetup.username);
  DebugPort.printf("  <4> - set password, currently \"%s\"\r\n", _MQTTsetup.password);
  DebugPort.printf("  <5> - set root topic, currently \"%s\"\r\n", _MQTTsetup.topicPrefix);
  DebugPort.printf("  <6> - set QoS, currently %d\r\n", _MQTTsetup.qos);
  DebugPort.printf("  <7> - set enabled, currently %s\r\n", _MQTTsetup.enabled ? "ON" : "OFF");
  DebugPort.printf("  <ENTER> - save and exit\r\n");
  DebugPort.printf("  <ESC> - abort\r\n");
}

bool 
CMQTTsetup::Handle(char& rxVal)
{
  if(_active) {
    _active = HandleMQTTsetup(rxVal);
    if(!_active)
      rxVal = 0;
    return true;
  }
  return false;
}

bool 
CMQTTsetup::HandleMQTTsetup(char rxVal)
{
  bool bJumptoMQTTmenuRoot = false;
  switch(_mode) {
    case 0:
      if(rxVal == 0x1b) {
        _MQTTsetup = NVstore.getMQTTinfo();
        return false;
      }
      if(rxVal == '\n') {
        NVstore.setMQTTinfo(_MQTTsetup);
        NVstore.save();
        return false;
      }
      if(rxVal >= '1' && rxVal <= '7') {
        _mode = rxVal - '0';
        _idx = 0;
        DebugPort.print("\014");
        switch(_mode) {
          case 1: DebugPort.printf("Enter MQTT broker's IP address (%s)", _MQTTsetup.host); break;
          case 2: DebugPort.printf("Enter MQTT broker's port (%d)", _MQTTsetup.port); break;
          case 3: DebugPort.printf("Enter MQTT broker's username (%s)", _MQTTsetup.username); break;
          case 4: DebugPort.printf("Enter MQTT broker's password (%s)", _MQTTsetup.password); break;
          case 5: DebugPort.printf("Enter root topic name (%s)", _MQTTsetup.topicPrefix); break;
          case 6: DebugPort.printf("Enter QoS level (%d)", _MQTTsetup.qos); break;
          case 7: DebugPort.printf("Enable MQTT? (Y)es / (N)o (%s)", _MQTTsetup.enabled ? "YES" : "NO"); break;
        }
        DebugPort.print("... ");
      }
      else {
        showMQTTmenu();
      }
      return true;
    case 1:  // enter MQTT broker IP
      if(getMQTTstring(rxVal, 31, _MQTTsetup.host)) {
        bJumptoMQTTmenuRoot = true;
      }
      break;
    case 2:  // enter MQTT broker port
      if(rxVal < ' ') {
        if(_idx == 0) sprintf(_buffer, "%d", _MQTTsetup.port);
        if(rxVal == '\n') {
          int val = atoi(_buffer);
          _MQTTsetup.port = val;
          bJumptoMQTTmenuRoot = true;
        }
        if(rxVal == 0x1b) {
          bJumptoMQTTmenuRoot = true;
        }
        break;
      }
      DebugPort.print(rxVal);
      if(isdigit(rxVal)) {
        if(_idx == 0) memset(_buffer, 0, sizeof(_buffer));
        _buffer[_idx++] = rxVal;
        if(_idx == 5) {
          int val = atoi(_buffer);
          _MQTTsetup.port = val;
          bJumptoMQTTmenuRoot = true;
        }
      }
      else {
        bJumptoMQTTmenuRoot = true;
      }
      break;
    case 3:  // enter MQTT broker username
      if(getMQTTstring(rxVal, 31, _MQTTsetup.username)) {
        bJumptoMQTTmenuRoot = true;
      }
      break;
    case 4:  // enter MQTT broker username
      if(getMQTTstring(rxVal, 31, _MQTTsetup.password)) {
        bJumptoMQTTmenuRoot = true;
      }
      break;
    case 5:  // enter root topic name
      if(getMQTTstring(rxVal, 31, _MQTTsetup.topicPrefix)) {
        bJumptoMQTTmenuRoot = true;
      }
      break;
    case 6:
      if(rxVal >= '0' && rxVal <= '2') {
        _MQTTsetup.qos = rxVal - '0';  
      }
      bJumptoMQTTmenuRoot = true;
      break;
    case 7:
      if(tolower(rxVal) == 'y')
        _MQTTsetup.enabled = true;
      if(tolower(rxVal) == 'n')
        _MQTTsetup.enabled = false;
      bJumptoMQTTmenuRoot = true;
      break;
  }
  if(bJumptoMQTTmenuRoot) {
    _mode = 0;
    showMQTTmenu();
  }
  return true;
}

bool 
CMQTTsetup::getMQTTstring(char rxVal, int maxidx, char* pTargetString)
{
  if(rxVal < ' ') {
    if(_idx == 0) strcpy(_buffer, pTargetString);
    if(rxVal == '\r')
      return false;
    if(rxVal == '\n') {
      strncpy(pTargetString, _buffer, maxidx);
      pTargetString[maxidx] = 0;
      return true;
    }
    if(rxVal == 0x1b) {
      return true;
    }
  }
  else {
    if(_idx == 0) memset(_buffer, 0, sizeof(_buffer));
    DebugPort.print(rxVal);
    _buffer[_idx++] = rxVal;
    if(_idx == maxidx) {
      strncpy(pTargetString, _buffer, maxidx);
      pTargetString[maxidx] = 0;
      return true;
    }
  }
  return false;
}

