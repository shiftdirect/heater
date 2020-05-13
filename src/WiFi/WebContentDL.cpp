/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2020  Ray Jones
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

// seek a web page update from the afterburner web server

#include "WebContentDL.h"
#include "../Utility/DebugPort.h"
#include "../Utility/helpers.h"
#include "../Utility/BTC_JSON.h"

// #define DUMP_WEB_BYTES          

// callback function for bulk of file download.
// It appears the callback may well be running at an elevated priority, perhaps IRQL
// and that causes random core crashes when accessing SPIFFS.
// Instead me marshall the data via FreeRTOS queue to be handled at a definite user level later.
void WebPageDataCB(void* pClass, asyncHTTPrequest* request, size_t available) 
{  
  CWebContentDL* pParent = (CWebContentDL*)pClass;
  while(available) {
    int read = pParent->queueDLdata(available, request);
    if(read >= 0)
      available -= read;
    else 
      break;
  }
}

   
// callback function for completion of the Async TCP "GET" request.
// Not all file data may be processed yet...
void WebPageRequestCB(void* pClass, asyncHTTPrequest* request, int readyState) 
{
  CWebContentDL* pParent = (CWebContentDL*)pClass;
  if(readyState == 4) {
    while(request->available()) {
      pParent->queueDLdata(request->available(), request);
    }
    pParent->finalise(request->responseHTTPcode() == 200);  // mark end of file data, causes SPIFFS file to be closed.

    request->close();   // graceful close the Async TCP connection
  }
}



CWebContentDL::CWebContentDL()
{
#ifdef DEBUG_WEBDL
  _request.setDebug(true);
#endif
  _request.onReadyStateChange(WebPageRequestCB, this);
  _request.onData(WebPageDataCB, this);
  _queue = NULL;
  _fileActive = false;
  _bytecount = 0;
  _queuecount = 0;
}

CWebContentDL::~CWebContentDL()
{
  _closequeue();
}

bool
CWebContentDL::busy() const
{
  if(_fileActive)  
    return true;
  return _request.readyState() != 0 && _request.readyState() != 4;
}

bool
CWebContentDL::OK() const
{
  return _bOK;
}

void CWebContentDL::get(const char* filename) 
{
  if(_request.readyState() == 0 || _request.readyState() == 4){
    _openqueue();
    _openfile(filename);
    _bytecount = 0;
    _bOK = false;

    std::string URL = "http://afterburner.mrjones.id.au/fota/web";
    URL += _filename;
    _request.open("GET", URL.c_str());
    _request.send();
  }
}

// routine called regualrly by the "loop" task - ie not IRQL
// it is not safe to write to SPIFFS in the AsyncTCP callbacks!
void 
CWebContentDL::process() 
{
  sQueueEntry entry;

  while(_queue != NULL && xQueueReceive(_queue, &entry, 0)) {
    int16_t len = entry.len;
    if(len == -1) {
      _closefile();
      _closequeue();
      _bOK = true;
      DebugPort.printf("Downloaded %s (%d bytes) - CLOSED OK\r\n", _filename.c_str(), _bytecount);
    }
    else if(len == -2) {
      _closefile();
      SPIFFS.remove(_filename.c_str());  // remove the bad file from SPIFFS
      _closequeue();
      DebugPort.printf("HTTP ERROR ENCOUNTERED: %s\r\n", _filename.c_str());
    }
    else if(len > 0) {
      if(_file) {
        if(_file.write(entry.data, len) != len) {   // Write the received bytes to the file
          _closefile();
          DebugPort.printf("Web content download - FILE_WRITE error: removing %s\r\n", _filename.c_str());
          SPIFFS.remove(_filename.c_str());  // remove the bad file from SPIFFS
        }
        else {
#ifdef DUMP_WEB_BYTES          
          for(int i=0; i< len;) {
            for(int j=0; j< 32 && i<len; j++) {
              DebugPort.printf("%02X ", entry.data[i++]);
              if((i & 0xf) == 0)
                DebugPort.print(" ");
              if((i & 0x7) == 0)
                DebugPort.print(" ");
            }
            DebugPort.print("\r\n");
          }
#endif
          _bytecount += len;
        }
      }
    }
  }
}

int16_t 
CWebContentDL::queueDLdata(int size, asyncHTTPrequest* request) 
{
  sQueueEntry entry;
  
  if(size > sizeof(entry.data)) 
    size = sizeof(entry.data);

  int16_t read = request->responseRead(entry.data, size);

  if(_queue == NULL) {
    DebugPort.println("CWebContentDL::queueDLdata - no queue!");
  }
  else if(read <= 0) {
    DebugPort.println("CWebContentDL::queueDLdata - read error?");
  }
  else {
    // available -= read;
    if(request->responseHTTPcode() == 200) {  // only push to queue if HTTP OK
      entry.len = read;
      entry.count = ++_queuecount;

      BaseType_t awoken;
      xQueueSendFromISR(_queue, &entry, &awoken);
    }
  }

  return read;
}

void 
CWebContentDL::finalise(bool OK) 
{
  if(_queue != NULL) {
    sQueueEntry entry;

    entry.len = OK ? -1 : -2;
    entry.count = -1;
    BaseType_t awoken;
    xQueueSendFromISR(_queue, &entry, &awoken);
  }
}


void 
CWebContentDL::abort()
{
  _request.close();

  _closefile();
}

void 
CWebContentDL::_closefile()
{
  if(_file) {
    _file.close();
    _fileActive = false;
  }
}

void
CWebContentDL::_openqueue()
{
  if(_queue == NULL) {
    _queue = xQueueCreate(10, sizeof(sQueueEntry));
    _queuecount = 0;
  }
}

void
CWebContentDL::_closequeue()
{
  if(_queue)
    vQueueDelete(_queue);
  _queue = NULL;
}

void
CWebContentDL::_setfilename(const char* filename)
{
  // ensure leading forward slash, required for SPIFFS
  _filename = "";
  if(filename[0] != '/') _filename = "/"; 
  _filename += filename;
}

void 
CWebContentDL::_openfile(const char* filename)
{
  _setfilename(filename);
  filename = _filename.c_str();  // replace with sanitised name

  DebugPort.printf("Loading file to SPIFFS: '%s'\r\n", filename);
  if(SPIFFS.exists(filename)) {
    DebugPort.println("  Already exists! - removing");
    SPIFFS.remove(filename);
  }

  _file = SPIFFS.open(filename, "w");  // Open the file for writing in SPIFFS 
  _fileActive = true;
}

const char* 
CWebContentDL::getState()
{
  return _filename.c_str();
}


CGetWebContent::CGetWebContent()
{
  _state = 0;
  _holdoff = 0;
}

void
CGetWebContent::start() {
  _state = 1;
  manage();
}

void
CGetWebContent::_get(const char* filename)
{
  _filename = filename;
  DebugPort.printf("Requesting %s from Afterburner web site\r\n", _filename.c_str());
  handler.get(_filename.c_str()); 
  setHoldoff(_holdoff, 15000);
}

bool 
CGetWebContent::_done()
{
  if(!handler.busy()) {
    DebugPort.printf("Completed %s from Afterburner web site\r\n", _filename.c_str());
    return true;
  }
  return false;
}

void 
CGetWebContent::manage() 
{
  switch(_state) {
    case 1:
      _get("index.html.gz");
      _state++;
      break;   
    case 2:
      handler.process();
      if(_timeout()) {
        handler.abort();
        _state = -1;
      }
      if(_done()) {
        if(!handler.OK()) {
          setHoldoff(_holdoff, 1000);
          _state = -2;
        }
        else {
          _sendJSON();
          _state++;
        }
      }
      break;
    case 3:
      _get("favicon.ico");
      _state++;
      break;
    case 4:
      handler.process();
      if(_timeout()) {
        handler.abort();
        _state = -1;
      }
      if(_done()) {
        setHoldoff(_holdoff, 1000);
        if(!handler.OK()) {
          setHoldoff(_holdoff, 1000);
          _state = -2;
        }
        else {
          _state++;
          _sendJSON();
        }
      }
      break;
    case 5:
      if(_timeout()) {
        _state = 0;
        _sendJSON("DONE");
      }
      break;
    case -2:
      if(_timeout()) {
        _state = -1;
        _sendJSON("ERROR");
      }
      break;
  }
}

bool
CGetWebContent::_timeout() {
  if(_holdoff) {
    long tDelta = millis() - _holdoff;
    if(tDelta > 0) {
      _holdoff = 0;
      return true;
    }
    else 
      return false;
  }
  return true;
}

const char* 
CGetWebContent::getFilename()
{
  if(_state == -1) 
    return "ERROR";
  if(_state == 0)
    return "DONE";
  return _filename.c_str();
}

void
CGetWebContent::_sendJSON(const char* name)
{
  std::string JSONmsg;

  JSONmsg = "{\"LoadWebContent\":\"";
  if(name == NULL)
    JSONmsg += _filename;
  else
    JSONmsg += name;  
  JSONmsg += "\"}";
  sendJSONtext(JSONmsg.c_str(), false);
}

