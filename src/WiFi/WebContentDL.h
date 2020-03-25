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

#include "../../asyncHTTPrequest/src/asyncHTTPrequest.h"
#include <SPIFFS.h>
#include "freertos/queue.h"
#include <string>

#pragma pack(push, 1)
struct sQueueEntry {
  int16_t len;
  int16_t count;
  uint8_t  data[2044];
};
#pragma pack(pop)

class CWebContentDL {
  std::string _filename;
  asyncHTTPrequest _request;
  bool _fileActive;
  File _file;
  int _bytecount;
  int _queuecount;
  QueueHandle_t _queue;
  bool _bOK;
  void _closefile();
  void _openfile(const char* filename);
  void _openqueue();
  void _closequeue();
  void _setfilename(const char* filename);
public:
  CWebContentDL();
  ~CWebContentDL();
  void get(const char* filename);
  void process();
  // callback handlers
  int16_t queueDLdata(int size, asyncHTTPrequest* request);
  void finalise(bool OK); 
  bool busy() const;
  bool OK() const;
  void abort();
  const char* getState();
};

class CGetWebContent 
{
  unsigned long _holdoff;
  int _state;
  std::string _filename;
  void _get(const char* filename);
  bool _done();
  bool _timeout();
  void _sendJSON(const char* filename=NULL);
  CWebContentDL handler;
public:
  CGetWebContent();
  void start();
  void manage();
  const char* getFilename();
};


void WebPageRequestCB(void* optParm, asyncHTTPrequest* request, int readyState);
void WebPageDataCB(void* optParm, asyncHTTPrequest*, size_t available);


