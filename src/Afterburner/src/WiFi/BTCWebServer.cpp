/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2018  Ray Jones <ray@mrjones.id.au>
 * Copyright (C) 2018  James Clark
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

#define USE_EMBEDDED_WEBUPDATECODE    

#include "BTCWebServer.h"
#include "../Utility/DebugPort.h"
#include "../Protocol/TxManage.h"
#include "../Utility/helpers.h"
#include "../cfg/pins.h"
#include "../cfg/BTCConfig.h"
//#include "Index.h"
#include "../Utility/BTC_JSON.h"
#include "../Utility/Moderator.h"
#include "../Libraries/WiFiManager-dev/WiFiManager.h"
#include <SPIFFS.h>
#include "../Utility/NVStorage.h"

extern WiFiManager wm;

File fsUploadFile;              // a File object to temporarily store the received file
int SPIFFSupload = 0;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

bool bRxWebData = false;   // flags for OLED animation
bool bTxWebData = false;
bool bUpdateAccessed = false;  // flag used to ensure web update always starts via /update. direct accesses to /updatenow will FAIL
long _SuppliedFileSize = 0;

void handleBTCNotFound();
bool checkFile(File &file);


String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".bin")) return "application/octet-stream";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  DebugPort.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    if(!checkFile(file)) {                              // check it is readable
      file.close();                                     // Then close the file again
    }
    if(!file) {
      DebugPort.println("\tFile exists, but could not be read?");
      String SPIFFSfmtpath = "http://" + server.client().localIP().toString() + "/formatspiffs";
      String Updatepath = "http://" + server.client().localIP().toString() + "/update";
    	String message = "<h1>Internal Server Error</h1>";
      message += "<h3>Sorry, cannot open file</h3>";
      message += "<p><b><i>" + path + "</i></b> exists, but cannot be opened?<br>";
      message += "Recommended remedy is to re-format SPIFFS, then reload the web content.";
      message += "<p><b>Use:<br><i><a href=\"" + SPIFFSfmtpath + "\" target=\"_blank\">" + SPIFFSfmtpath + "</a></b></i>  to format SPIFFS.";
      message += "<p><b>Then:<br><i>" + Updatepath + "</b></i>  to upload each file of the web content.<br>";
      message += "<p>Latest web content can be downloaded from <a href=\"http://www.mrjones.id.au/afterburner/firmware.html\" target=\"_blank\">http://www.mrjones.id.au/afterburner/firmware.html</a>";
      message += "<p><b>Please ensure you unzip the web page content, then upload all the contained files.</b>";
      server.send(500, "text/html", message);
    }
    else {
      server.streamFile(file, contentType); // And send it to the client
      file.close();                                       // Then close the file again
    }
    return true;
  }
  DebugPort.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}

void handleWMConfig() 
{
  DebugPort.println("WEB: GET /wmconfig");
	server.send(200, "text/plain", "Start Config Portal - Retaining credential");
	DebugPort.println("Starting web portal for wifi config");
  delay(500);
  wifiEnterConfigPortal(true, false, 3000);
}

void handleReset() 
{
  DebugPort.println("WEB: GET /resetwifi");
	server.send(200, "text/plain", "Start Config Portal - Resetting Wifi credentials!");
	DebugPort.println("diconnecting client and wifi, then rebooting");
  delay(500);
  wifiEnterConfigPortal(true, true, 3000);
}

void handleFormat() 
{
  DebugPort.println("WEB: GET /formatspiffs");
  String Updatepath = "http://" + server.client().localIP().toString() + "/update";
	String message = "<h1>SPIFFS partition formatted</h1>";
  message += "<h3>You must now upload the web content.</h3>";
  message += "<p>Latest web content can be downloaded from <a href=\"http://www.mrjones.id.au/afterburner/firmware.html\" target=\"_blank\">http://www.mrjones.id.au/afterburner/firmware.html</a>";
  message += "<p><b>Use:<br><i><a href=\"" + Updatepath + "\">" + Updatepath + "</a></b></i>  to then upload each file of the web content.<br>";
  message += "<p><b>Please ensure you unzip the web page content, then upload all the contained files.</b>";
	server.send(200, "text/html", message);

	DebugPort.println("Formatting SPIFFS partition");
  delay(500);
  SPIFFS.format();
}

void handleSpiffs() 
{
  String report;
  String message;
  listDir(SPIFFS, "/", 2, report, true);
  char usage[128];
  sprintf(usage, "<p>Usage: %d/%d <p>", SPIFFS.usedBytes(), SPIFFS.totalBytes());
  message += "<h1>Current SPIFFS contents:</h1>";
  message += report;
  message += usage;
  message += "<p><a href=\"/update\">Add more files</a><br>";
  message += "<p><a href=\"/index.html\">Home</a>";

	server.send(200, "text/html", message);
}

void handleBTCNotFound() 
{
  String path = server.uri();
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String Updatepath = "http://" + server.client().localIP().toString() + "/update";

	String message = "<h1>404: File Not Found</h1>";
	message += "<p>URI: <b><i>" + path + "</i></b>";
	message += "<br>Method: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "<br>Arguments: ";
	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "<br>";
	}
  message += "<hr>";
  message += "<p>Please try uploading the file from the web content.";
  message += "<p>Latest web content can be downloaded from <a href=\"http://www.mrjones.id.au/afterburner/firmware.html\" target=\"_blank\">http://www.mrjones.id.au/afterburner/firmware.html</a>";
  message += "<p><b>Use:<br><i><a href=\"/update\">" + Updatepath + "</a></b></i>  to upload the web content.<br>";
  message += "<p><b>Please ensure you unzip the web page content, then upload all the contained files.</b>";

  char usage[128];
  sprintf(usage, "<p>Usage: %d/%d<p>", SPIFFS.usedBytes(), SPIFFS.totalBytes());
  String report;
  listDir(SPIFFS, "/", 2, report);
  message += "<hr><h3>Current SPIFFS contents:</h3>";
  message += report;
  message += usage;

	server.send(404, "text/html", message);
}

// embedded HTML & Javascript to perform browser based updates of firmware or SPIFFS
const char* updateIndex = R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
  <meta charset="utf-8"/>
  <meta http-equiv="Pragma" content="no-cache">
  <meta http-equiv="Expires" content="-1">
  <meta http-equiv="CACHE-CONTROL" content="NO-CACHE">
<script>
  // global variables
  var sendSize;
  var ws;

  function _(el) {
    return document.getElementById(el);
  }
  function init() {
    ws = new WebSocket('ws://' + window.location.hostname + ':81/');

    ws.onmessage = function(event){
      var response = JSON.parse(event.data);
      var key;
      for(key in response) {
        switch(key) {
          case "progress":
            // actual data bytes received as fed back via web socket
            var bytes = response[key];
            _("loaded_n_total").innerHTML = "Uploaded " + bytes + " bytes of " + sendSize;
            var percent = Math.round( 100 * (bytes / sendSize));
            _("progressBar").value = percent;
            _("status").innerHTML = percent+"% uploaded.. please wait";
            break;
        }
      }
    }
  }
  function uploadFile() {
    _("cancel").hidden = true;
    var file = _("file1").files[0];
    sendSize = file.size;

    var JSONmsg = {};
    JSONmsg['UploadSize'] = sendSize;
    var str = JSON.stringify(JSONmsg);
    console.log("JSON Tx:", str);
    ws.send(str);

    var formdata = new FormData();
    formdata.append("update", file);
    var ajax = new XMLHttpRequest();
    // progress is handled via websocket JSON sent from controller
    // using server side progress only shows the buffer filling, not actual delivery.
    ajax.addEventListener("load", completeHandler, false);
    ajax.addEventListener("error", errorHandler, false);
    ajax.addEventListener("abort", abortHandler, false);
    ajax.open("POST", "/updatenow");
    ajax.send(formdata);
  }
  function completeHandler(event) {
   _("status").innerHTML = event.target.responseText;
   _("progressBar").value = 0;
   _("loaded_n_total").innerHTML = "Uploaded " + sendSize + " bytes of " + sendSize;
    var file = _("file1").files[0];
    if(file.name.endsWith(".bin")) {
      setTimeout(function () { 
        window.location.assign("/"); 
      }, 5000);    
    }
    else {
      setTimeout(function () { 
        window.location.reload(); 
      }, 1000);    
    }
  }
  function errorHandler(event) {
    _("status").innerHTML = "Upload Failed";
  }
  function abortHandler(event) {
    _("status").innerHTML = "Upload Aborted";
  }
</script>
<style>
  body {font-family: Arial, Helvetica, sans-serif;}
</style>
  <title>Afterburner firmware update</title>
</head>
<body onload="javascript:init()">
  <h1>Afterburner firmware update</h1>
  <form id="upload_form" method="POST" enctype="multipart/form-data" autocomplete="off">
    <input type="file" name="file1" id="file1"> <BR>
    <input type="button" value="Update" onclick="uploadFile()"> 
    <progress id="progressBar" value="0" max="100" style="width:300px;"></progress><BR>
    <h3 id="status"></h3>
    <p id="loaded_n_total"></p>
    <BR>
    <input type="button" onclick=window.location.assign("/") value="Cancel" id="cancel">
  </form>
</body>
</html>
)=====";


void rootRedirect()
{
  server.sendHeader("Location","/");      // reselect the update page
  server.send(303);
}


void initWebServer(void) {

  Update
  .onProgress([](unsigned int progress, unsigned int total) {
    int percent = (progress / (total / 100));
		DebugPort.printf("Progress: %u%%\r", percent);
    DebugPort.handle();    // keep telnet spy alive
    ShowOTAScreen(percent, eOTAWWW);  // WWW update in place
    DebugPort.print("^");
	});
  
	if (MDNS.begin("Afterburner")) {
		DebugPort.println("MDNS responder started");
	}
	
	server.on("/wmconfig", handleWMConfig);
	server.on("/resetwifi", handleReset);
	server.on("/formatspiffs", handleFormat);
	server.on("/spiffs", handleSpiffs);

  server.on("/tst", HTTP_GET, []() {
    DebugPort.println("WEB: GET /tst");
    server.sendHeader("Location","/");      // reselect the update page
    server.send(303);
  });

  // Magical code originally shamelessly lifted from Arduino WebUpdate example, then modified
  // This allows pushing new firmware to the ESP from a WEB BROWSER!
  // Added authentication and a sequencing flag to ensure this is not bypassed
  //
  // Initial launch page
  server.on("/update", HTTP_GET, []() {
    DebugPort.println("WEB: GET /update");
    sCredentials creds = NVstore.getCredentials();
    if (!server.authenticate(creds.webUpdateUsername, creds.webUpdatePassword)) {
      return server.requestAuthentication();
    }
    bUpdateAccessed = true;
#ifdef USE_EMBEDDED_WEBUPDATECODE    
    server.send(200, "text/html", updateIndex);
#else
     handleFileRead("/uploadfirmware.html");
#endif
  });

  // handle attempts to just browse the /updatenow path - force redirect to root
  server.on("/updatenow", HTTP_GET, []() {  
    DebugPort.println("WEB: GET /updatenow - ILLEGAL - root redirect");
    rootRedirect();
  });

  // actual guts that manages the new firmware upload
  server.on("/updatenow", HTTP_POST, []() {
    DebugPort.println("WEB: POST /updatenow completion");
    // completion functionality
    if(SPIFFSupload) {
      if(SPIFFSupload == 1) {
        DebugPort.println("WEB: SPIFFS OK");
        server.send(200, "text/plain", "OK - File uploaded to SPIFFS");
        // javascript reselects the /update page!
      }
      else {
        DebugPort.println("WEB: SPIFFS FAIL");
        server.send(500, "text/plain", "500: couldn't create file");
      }
      SPIFFSupload = 0;
    }
    else {
      if(Update.hasError()) {
        DebugPort.println("WEB: UPDATE FAIL");
        server.send(200, "text/plain", "FAIL - Afterburner will reboot shortly");
      }
      else {
        DebugPort.println("WEB: UPDATE OK");
        server.send(200, "text/plain", "OK - Afterburner will reboot shortly");
      }
      delay(1000);
      // javascript redirects to root page so we go there after reboot!
      forceBootInit();
      ESP.restart();                             // reboot
    }
  }, []() {
    if(bUpdateAccessed) {  // only allow progression via /update, attempts to directly access /updatenow will fail
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        String filename = upload.filename;
        DebugPort.setDebugOutput(true);
        if(filename.endsWith(".bin")) {
          DebugPort.printf("Update: %s %d\r\n", filename.c_str(), upload.totalSize);
          if (!Update.begin()) { //start with max available size
            Update.printError(DebugPort);
          }
        }
        else {
          if(!filename.startsWith("/")) filename = "/"+filename;
          DebugPort.printf("handleFileUpload Name: %s\r\n", filename.c_str());
          fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
          SPIFFSupload = fsUploadFile ? 1 : 2;
          //filename = String();
        }
      } 

      // handle file segments
      else if (upload.status == UPLOAD_FILE_WRITE) {
#if USE_SW_WATCHDOG == 1
        feedWatchdog();   // we get stuck here for a while, don't let the watchdog bite!
#endif
        if(upload.totalSize) {
          char JSON[64];
          sprintf(JSON, "{\"progress\":%d}", upload.totalSize);
          sendWebServerString(JSON);  // feedback proper byte count of update to browser via websocket
        }
        int percent = 0;
        if(_SuppliedFileSize) 
          percent = 100 * upload.totalSize / _SuppliedFileSize;
        ShowOTAScreen(percent, eOTAbrowser);  // browser update 

        DebugPort.print(".");
        if(fsUploadFile) {
          fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
        }
        else {
          if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(DebugPort);
          }
        }
      } 
      
      // handle end of upload
      else if (upload.status == UPLOAD_FILE_END) {
        if(SPIFFSupload) {
          if(fsUploadFile) {
            fsUploadFile.close();                               // Close the file again
            DebugPort.printf("handleFileUpload Size: %d\r\n", upload.totalSize);
          }
        }
        else {
          if (Update.end(true)) { //true to set the size to the current progress
            DebugPort.printf("Update Success: %u\r\nRebooting...\r\n", upload.totalSize);
          } else {
            Update.printError(DebugPort);
          }
        }
        DebugPort.setDebugOutput(false);
        bUpdateAccessed = false;
      } else {
        DebugPort.printf("Update Failed Unexpectedly (likely broken connection): status=%d\r\n", upload.status);
        bUpdateAccessed = false;
      }
    }
    else {
      // attempt to POST without using /update - forced redirect to root
      DebugPort.println("WEB: POST /updatenow forbidden entry");
      rootRedirect();
    }
  });

  // NOTE: this serves the default home page, and favicon.ico
  server.onNotFound([]() 
  {                                                      // If the client requests any URI
    if (!handleFileRead(server.uri())) {                  // send it if it exists
      handleBTCNotFound();
    }
  });

	server.begin();
  
	webSocket.begin();
	webSocket.onEvent(webSocketEvent);

	DebugPort.println("HTTP server started");

}


// called by main sketch loop()
bool doWebServer(void) 
{
	webSocket.loop();
	server.handleClient();
  return true;
}

bool isWebServerClientChange() 
{
	static int prevNumClients = -1;

	int numClients = webSocket.connectedClients();
	if(numClients != prevNumClients) {
		prevNumClients = numClients;
		DebugPort.println("Changed number of web clients, should reset JSON moderator");
    return true;
	}
  return false;
}

bool sendWebServerString(const char* Str)
{
  CProfile profile;
	if(webSocket.connectedClients()) {
    unsigned long tCon = profile.elapsed(true);
    bTxWebData = true;              // OLED tx data animation flag
    webSocket.broadcastTXT(Str);
    unsigned long tWeb = profile.elapsed(true);
    DebugPort.printf("Websend times : %ld,%ld\r\n", tCon, tWeb); 
		return true;
	}
  return false;
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) 
{
	if (type == WStype_TEXT) {
		bRxWebData = true;
		char cmd[256];
		memset(cmd, 0, 256);
		for (int i = 0; i < length && i < 256; i++) {
			cmd[i] = payload[i];
		}
		interpretJsonCommand(cmd);  // send to the main heater controller decode routine
  }
}

bool hasWebClientSpoken(bool reset)
{
	bool retval = bRxWebData;
	if(reset)
  	bRxWebData = false;
	return retval;
}

bool hasWebServerSpoken(bool reset)
{
	bool retval = bTxWebData;
	if(reset)
		bTxWebData = false;
	return retval;
}

void setUploadSize(long val)
{
  _SuppliedFileSize = val;
};

// Sometimes SPIFFS gets corrupted (WTF?)
// When this happens, you can see the files exist, but you cannot read them
// This routine checks the file is readable.
// Typical failure mechanism is read returns 0, and the WifiClient upload never progresses
// The software watchdog then steps in after 15 seconds of that nonsense
bool checkFile(File &file) 
{
  uint8_t buf[128];
  bool bOK = true;

  size_t available = file.available();
  while(available) {
    int toRead = (available > 128) ? 128 : available;
    int Read = file.read(buf, toRead);
    if(Read != toRead) {
      bOK = false;
      DebugPort.printf("SPIFFS precautionary file check failed for %s\r\n", file.name());
      break;
    }
    available = file.available();
  }
  file.seek(0);
  return bOK;
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels, String& HTMLreport, bool withHTMLanchors) 
{
  char msg[128];
  File root = fs.open(dirname);
  if (!root) {
    sprintf(msg, "Failed to open directory \"%s\"", dirname);
    DebugPort.println(msg);
    HTMLreport += msg; HTMLreport += "<br>";
    return;
  }
  if (!root.isDirectory()) {
    sprintf(msg, "\"%s\" is not a directory", dirname);
    DebugPort.println(msg);
    HTMLreport += msg; HTMLreport += "<br>";
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      sprintf(msg, "  DIR : %s", file.name());
      DebugPort.println(msg);
      HTMLreport += msg; HTMLreport += "<br>";
      if (levels) {
        listDir(fs, file.name(), levels - 1, HTMLreport);
      }
    } else {
      String fn = file.name();
      if(withHTMLanchors) {
        if(fn.endsWith(".html") || fn.endsWith(".htm")) {
          String fn2(fn);
          fn = "<a href=\"" + fn2 + "\">" + fn2 + "</a>";
        }
      }
      sprintf(msg, "  FILE: %s SIZE: %d", fn.c_str(), file.size());
      DebugPort.println(msg);
      HTMLreport += msg; HTMLreport += "<br>";
    }
    file = root.openNextFile();
  }
}    
