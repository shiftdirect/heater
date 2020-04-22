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

#ifndef BTC_BROWSERUPLOAD_H_
#define BTC_BROWSERUPLOAD_H_

#include <Arduino.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <HTTPResponse.hpp>

struct sBrowserUpload{
  struct {
    String name;
    int size;
  } SrcFile;
  struct {
    File file;              // a File object to store the received file into SPIFFS
    int state;
  } DstFile;
  bool bUploadActive;
  //methods
  sBrowserUpload() {
    reset();
  }
  void reset() {
    if(DstFile.file) {
      DstFile.file.close();
    }
    DstFile.state = 0;
    bUploadActive = false;
  }
  void init();
  int begin(String& filename, int filesize = -1);
  int fragment(HTTPUpload& upload, httpsserver::HTTPResponse * res = NULL);
  int end(HTTPUpload& upload);
  bool isSPIFFSupload() const { return DstFile.state != 0; };
  bool isOK() const; 
};



#endif
