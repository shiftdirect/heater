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
#include "../WiFi/ABMQTT.h"

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
  DebugPort.enable(true);
  if(init)
    _MQTTsetup = NVstore.getMQTTinfo();

  DebugPort.print("\014");
  DebugPort.println("MQTT broker configuration");
  DebugPort.println("");
  DebugPort.printf("  <1> - set IP address, currently \"%s\"\r\n", _MQTTsetup.host);
  DebugPort.printf("  <2> - set port, currently %d\r\n", _MQTTsetup.port);
  DebugPort.printf("  <3> - set username, currently \"%s\"\r\n", _MQTTsetup.username);
  DebugPort.printf("  <4> - set password, currently \"%s\"\r\n", _MQTTsetup.password);
#ifdef ALLOW_USER_TOPIC
  DebugPort.printf("  <5> - set root topic, currently \"%s\"\r\n", _MQTTsetup.topicPrefix);
#else
  DebugPort.printf("        Fixed unique topic prefix: \"%s\"\r\n", getTopicPrefix());
#endif
  DebugPort.printf("  <6> - set QoS, currently %d\r\n", _MQTTsetup.qos);
  DebugPort.printf("  <7> - set enabled, currently %s\r\n", _MQTTsetup.enabled ? "ON" : "OFF");
  DebugPort.printf("  <ENTER> - save and exit\r\n");
  DebugPort.printf("  <ESC> - abort\r\n");

  DebugPort.enable(false);  // suppress sundry debug whilst MQTT menu is active
}

bool 
CMQTTsetup::Handle(char& rxVal)
{
  if(_active) {
    DebugPort.enable(true);
    _active = HandleMQTTsetup(rxVal);
    if(_active)
      DebugPort.enable(false);
    else
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
        DebugPort.print("\014");
        switch(_mode) {
          case 1: 
            DebugPort.printf("Enter MQTT broker's IP address (%s)", _MQTTsetup.host); 
            _lineInput.reset(_MQTTsetup.host, 31); 
            break;
          case 2: 
            DebugPort.printf("Enter MQTT broker's port (%d)", _MQTTsetup.port); 
            _lineInput.reset(_MQTTsetup.port);
            break;
          case 3: 
            DebugPort.printf("Enter MQTT broker's username (currently '%s', CTRL-X to erase)", _MQTTsetup.username); 
            _lineInput.reset(_MQTTsetup.username, 31);
            break;
          case 4: 
            DebugPort.printf("Enter MQTT broker's password (currently '%s', CTRL-X to erase)", _MQTTsetup.password); 
            _lineInput.reset(_MQTTsetup.password, 31); 
            break;
          case 5: 
#ifdef ALLOW_USER_TOPIC
            DebugPort.printf("Enter root topic name (%s)", _MQTTsetup.topicPrefix); 
            _lineInput.reset(_MQTTsetup.topicPrefix, 31); 
#else
            _mode = 0;  // topic prefix is now fixed, based upon our STA MAC
            showMQTTmenu();
#endif
            return true;
          case 6: 
            DebugPort.printf("Enter QoS level (%d)", _MQTTsetup.qos); 
            break;
          case 7: 
            DebugPort.printf("Enable MQTT? (Y)es / (N)o (%s)", _MQTTsetup.enabled ? "YES" : "NO"); 
            break;
        }
        DebugPort.print("... ");
      }
      else {
        showMQTTmenu();
      }
      return true;
    case 1:  // enter MQTT broker IP
      if(_lineInput.handle(rxVal)) {
        bJumptoMQTTmenuRoot = true;
      }
      break;
    case 2:  // enter MQTT broker port
      if(_lineInput.handle(rxVal)) {
        _MQTTsetup.port = _lineInput.getNumeric();
        bJumptoMQTTmenuRoot = true;
      }
      break;
    case 3:  // enter MQTT broker username
      if(_lineInput.handle(rxVal)) {
        bJumptoMQTTmenuRoot = true;
      }
      break;
    case 4:  // enter MQTT broker username
      if(_lineInput.handle(rxVal)) {
        bJumptoMQTTmenuRoot = true;
      }
      break;
    case 5:  // enter root topic name
      if(_lineInput.handle(rxVal)) {
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


CSecuritySetup::CSecuritySetup()
{
  _active = false;
  _password.Idx = 0;
  _password.State = 0;
}

void 
CSecuritySetup::setActive()
{
  _active = true;
  _showMenu(true);
}

void insertDummy(int len);

void 
CSecuritySetup::_showMenu(bool init)
{
  _mode = 0;
  _password.Idx = 0;
  _password.State = 0;

  DebugPort.enable(true);
  if(init)
    _credsSetup = NVstore.getCredentials();

  int len;

  DebugPort.print("\014");
  DebugPort.println("Security configuration");
  DebugPort.println("");
  DebugPort.println("  Access Point credentials");
  DebugPort.printf("    <1> - set SSID, currently \"%s\"\r\n", _credsSetup.APSSID);
  DebugPort.print("    <2> - set password");
  len = strlen(_credsSetup.APpassword);
  if(len == 0)
    DebugPort.print(", currently UNRESTRICTED\r\n");
  else {
    insertDummy(len);
  }
  DebugPort.println("");

  DebugPort.println("  Web page credentials");
  DebugPort.printf("    <3> - set username, currently \"%s\"\r\n", _credsSetup.webUsername);
  DebugPort.print("    <4> - set password");
  len = strlen(_credsSetup.webPassword);
  if(len == 0)
    DebugPort.printf(", currently UNRESTRICTED\r\n");
  else {
    insertDummy(len);
  }
  DebugPort.println("");

  DebugPort.println("  /update web page credentials");
  DebugPort.printf("    <5> - set username, currently \"%s\"\r\n", _credsSetup.webUpdateUsername);
  DebugPort.printf("    <6> - set password");
  len = strlen(_credsSetup.webUpdatePassword);
  if(len == 0)
    DebugPort.printf(", UNRESTRICTED!\r\n");
  else {
    insertDummy(len);
  }
  DebugPort.println("");
  DebugPort.printf("  <ENTER> - save and exit\r\n");
  DebugPort.printf("  <ESC> - abort\r\n");

  DebugPort.enable(false);  // suppress sundry debug whilst Security menu is active
}

void insertDummy(int len) {
  char dummy[32];
  memset(dummy, 0, 32);
  if(len > 31) 
    len = 31;
  for(int i = 0; i < len; i++)
    dummy[i] = '*';
  DebugPort.printf(" (%s)\r\n", dummy);
}


bool 
CSecuritySetup::Handle(char& rxVal)
{
  if(_active) {
    DebugPort.enable(true);
    _active = _handle(rxVal);
    if(_active)
      DebugPort.enable(false);
    else
      rxVal = 0;
    return true;
  }
  return false;
}

bool 
CSecuritySetup::_handle(char rxVal)
{
  bool bJumptoMenuRoot = false;
  if(_getPassword()) {
    if(_handlePassword(rxVal)) {
      if(!_getPassword()) 
        _showMenu();
    }
    return true;
  }
  switch(_mode) {
    case 0:  // initial menu entry selection
      if(rxVal == 0x1b) {
        _credsSetup = NVstore.getCredentials();
        return false;
      }
      if(rxVal == '\n') {
        NVstore.setCredentials(_credsSetup);
        NVstore.save();
        return false;
      }
      if(rxVal >= '1' && rxVal <= '6') {
        _mode = rxVal - '0';
        DebugPort.print("\014");
        switch(_mode) {
          case 1: 
            DebugPort.printf("Enter new AP SSID (%s)", _credsSetup.APSSID); 
            _lineInput.reset(_credsSetup.APSSID, 31); 
            break;
          case 2: 
            DebugPort.print("Enter current AP password"); 
            _initPassword(0);
            break;
          case 3: 
            DebugPort.printf("Enter new Web page access username (currently '%s', CTRL-X to erase)", _credsSetup.webUsername); 
            _lineInput.reset(_credsSetup.webUsername, 31);
            break;
          case 4: 
            DebugPort.print("Enter current web page access password"); 
            _initPassword(1);
            break;
          case 5: 
            DebugPort.printf("Enter new /update web page access username (currently '%s', CTRL-X to erase)", _credsSetup.webUpdateUsername); 
            _lineInput.reset(_credsSetup.webUpdateUsername, 31);
            break;
          case 6: 
            DebugPort.print("Enter current /update web page access password"); 
            _initPassword(2);
            break;
        }
        DebugPort.print("... ");
      }
      else {
        _showMenu();
      }
      return true;
    case 1:  // enter AP SSID
    case 2:  // enter AP password 
    case 3:  // enter web page username
    case 4:  // enter web page password
    case 5:  // enter /update username
    case 6:  // enter /update password
      if(_lineInput.handle(rxVal)) {
        bJumptoMenuRoot = true;
      }
      break;
  }
  if(bJumptoMenuRoot) {
    _showMenu();
  }
  return true;
}

void 
CSecuritySetup::_initPassword(int idx)
{
  _lineInput.reset();
  _lineInput.maskEntry();
  _password.Idx = idx;
  _password.State = 1;
}

bool
CSecuritySetup::_getPassword()
{
  return _password.State != 0;
}

bool
CSecuritySetup::_handlePassword(char rxVal)
{
  switch(_password.State) {
    case 1:
      if(_lineInput.handle(rxVal)) {
        _password.str1 = _lineInput.getString();
        _password.str2 = _getCurrentPassword();
        if(_password.str1 != _password.str2) {
          DebugPort.println("\r\nPassword does not match existing - ABORTING");
          _password.State = 0;
        }
        else {
          _password.State = 2;
          DebugPort.print("\r\nPlease enter new password - ");
          DebugPort.enable(false);  // block other debug msgs whilst we get the password
        }
        _lineInput.reset();
        _lineInput.maskEntry();
      }
      return true;
    case 2:
      if(_lineInput.handle(rxVal)) {
        _password.str1 = _lineInput.getString();
        if(_lineInput.getLen() < 8) {
          // ABORT - too short
          DebugPort.println("\r\nNew password must be at least 8 characters - ABORTING");
          _password.State = 0;
        }
        else if(_lineInput.getLen() > 31) {
          // ABORT - too long!
          DebugPort.println("\r\nNew password is longer than 31 characters - ABORTING");
          _password.State = 0;
        }
        else {
          _password.State = 3;
          DebugPort.print("\r\nPlease confirm new password - ");
          DebugPort.enable(false);  // block other debug msgs whilst we get the password
        }
        _lineInput.reset();
        _lineInput.maskEntry();
      }
      return true;
    case 3:
      if(_lineInput.handle(rxVal)) {
        _password.str2 = _lineInput.getString();
        _lineInput.reset();
        if(_password.str1 != _password.str2) {
          DebugPort.println("\r\nNew passwords do not match - ABORTING");
          _password.State = 0;
        }
        else {
          _password.State = 4;
          DebugPort.print("\r\nSet new password (y/n) - ");
        }
      }
      return true;
    case 4:
      if(rxVal == 'y' || rxVal == 'Y') {
        _setPassword(_password.str2.c_str());
      }
      _password.State = 0;
      return true;
  }
  return false;
}

const char*
CSecuritySetup::_getCurrentPassword() {
  switch(_password.Idx) {
    case 0: return _credsSetup.APpassword;
    case 1: return _credsSetup.webPassword;
    case 2: return _credsSetup.webUpdatePassword;
  }
  return "";
}      

void
CSecuritySetup::_setPassword(const char* newPW)
{
  switch(_password.Idx) {
    case 0: strcpy(_credsSetup.APpassword, newPW); break;
    case 1: strcpy(_credsSetup.webPassword, newPW); break;
    case 2: strcpy(_credsSetup.webUpdatePassword, newPW); break;
  }
}
