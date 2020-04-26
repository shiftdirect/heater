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


#include "BrowserUpload.h"
#include "../Utility/DebugPort.h"
#include <WiFi.h>
#include <Update.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include "BTCota.h"
#include "../Utility/helpers.h"

void
sBrowserUpload::init()
{
  Update
  .onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress / (total / 100));
		DebugPort.printf("Browser progress: %u%%\r\n", percent);
    DebugPort.handle();    // keep telnet spy alive
    ShowOTAScreen(percent, eOTAWWW);  // WWW update in place
    DebugPort.print("^");
	});
} 

int 
sBrowserUpload::begin(String& filename, int filesize)
{
  bUploadActive = true;
  SrcFile.name = filename;
  SrcFile.size = -1;   //start with max available size
  if(filesize) {
    SrcFile.size = filesize; // adapt to websocket supplied size
  }

  if(filename.endsWith(".bin")) {
    // FIRMWARE UPLOAD START
    DebugPort.printf("Starting firmware update: %s\r\n", SrcFile.name.c_str());  // name without leading slash

    if (!Update.begin(SrcFile.size)) { 
      Update.printError(DebugPort);
      bUploadActive = false;
      return -1;
    }
  }
  else {
    SrcFile.name.replace("%20", " ");   // convert HTML spaces to real spaces

    // SPIFFS UPLOAD START
    DebugPort.printf("Starting SPIFFS upload: %s\r\n", SrcFile.name.c_str());

    if(SPIFFS.exists(SrcFile.name)) {
      DebugPort.println("Removing existing file from SPIFFS");
      SPIFFS.remove(SrcFile.name);
    }

    // calculate a *very wild* guess of max size me *may* be able to cope with
    int freebytes = SPIFFS.totalBytes() - SPIFFS.usedBytes();
    DebugPort.printf("SPIFFS freespace test = %d %d\r\n", freebytes, SrcFile.size);  // report our space estimate
    freebytes -= 8192;  // at least 2 blocks must be kept free, each being 4k
//    int pageestimate = (SrcFile.size / 256) + 1 + 1;  // +1 for shortfall, +1 for metadata
//    freebytes -= pageestimate * 256;
    int pageestimate = (SrcFile.size / 4096) + 1 + 1;  // +1 for shortfall, +1 for metadata
    freebytes -= pageestimate * 4096;
    DebugPort.printf("SPIFFS freespace test = %d\r\n", freebytes);  // report our space estimate

    DstFile.state = -1;  // assume SPIFFS error for now...
    bUploadActive = false;
    if(freebytes > 0) {
      // *may* have enough space, open a file
      DstFile.file = SPIFFS.open(SrcFile.name, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
      if(DstFile.file) {
        DstFile.state = 1;   // opened OK, mark as SPIFFS upload in progress
        bUploadActive = true;
      }
    }
    // not enough space - return error to browser via web socket, javascript will report error
    if(DstFile.state == -1) {
      DebugPort.println("Aborting SPIFFS upload, insufficient space");
      return -1;
    }
  }
  return 0;
}

int 
sBrowserUpload::fragment(HTTPUpload& upload, HTTPResponse * res)
{
  if(isSPIFFSupload()) {
    // SPIFFS update (may be error state)
    if(DstFile.file) {
      // file is open, add new fragment of data to file opened for writing
      if(DstFile.file.write(upload.buf, upload.currentSize) != upload.currentSize) { // Write the received bytes to the file
        // ERROR! write operation failed if length does not match!
        DstFile.file.close();         // close the file (fsUploadFile becomes NULL)
        DstFile.state = -1;  // flag SPIFFS error!
        bUploadActive = false;
        DebugPort.printf("UPLOAD_FILE_WRITE error: removing %s\r\n", SrcFile.name.c_str());
        ::SPIFFS.remove(SrcFile.name.c_str());  // remove the bad file from SPIFFS
        return -2;
      }
      if(res)
        upload.totalSize += upload.currentSize;
    }
  }
  else {
    // Firmware update, add new fragment to OTA partition
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      // ERROR !
      Update.printError(DebugPort);
      return -3;
    }
    if(res)
      upload.totalSize += upload.currentSize;
  }
  return upload.totalSize;
}

int 
sBrowserUpload::end(HTTPUpload& upload)
{
  int retval = upload.totalSize;
  if(isSPIFFSupload()) {
    // any form of SPIFFS attempt (may be error)
    // Close the file if still open (did not encounter write error)
    if(DstFile.file) {
      DstFile.file.close();    
      DebugPort.printf("\r\nFinal SPIFFS upload Size: %d\r\n", upload.totalSize);
    }
    else {
      // file already closed indicates we encountered a write error
      retval = -2; 
    }
  }
  else {
    // completion of firmware update
    // check the added CRC we genertaed matches 
    // - this helps guard against malicious, badly formatted bin file attempts.
    if(!CheckFirmwareCRC(SrcFile.size)) {
      DebugPort.printf("\r\nCRC fail: %s, %d bytes\r\nRebooting...\r\n", SrcFile.name.c_str(), upload.totalSize);
      Update.abort();
      retval = -4;
    }

    if (Update.end()) { 
      DebugPort.printf("\r\nUpdate Success: %s, %d bytes\r\nRebooting...\r\n", SrcFile.name.c_str(), upload.totalSize);
    } else {
      Update.printError(DebugPort);
      if(retval == upload.totalSize) {
        retval = -3;
      }
    }
  }
  return retval;
}

bool 
sBrowserUpload::isOK() const 
{
  if(isSPIFFSupload())
    return DstFile.state == 1; 
  else
    return !Update.hasError(); 
}
