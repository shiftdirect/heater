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

#include <Arduino.h>
#include "BTCWifi.h"
#include "BTCWebServer.h"
#include "BTCota.h"
#include "../Utility/DebugPort.h"
#include "../Protocol/TxManage.h"
#include "../Utility/helpers.h"
#include "../cfg/pins.h"
#include "../cfg/BTCConfig.h"
#include "../Utility/BTC_JSON.h"
#include "../Utility/Moderator.h"
#include "../Libraries/WiFiManager-dev/WiFiManager.h"
#include <SPIFFS.h>
#include "../Utility/NVStorage.h"
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

extern WiFiManager wm;
extern const char* stdHeader;
extern const char* formatIndex;
extern const char* updateIndex;
extern const char* formatDoneContent;

File fsUploadFile;              // a File object to temporarily store the received file
int SPIFFSupload = 0;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

bool bRxWebData = false;   // flags for OLED animation
bool bTxWebData = false;
bool bUpdateAccessed = false;  // flag used to ensure web update always starts via /update. direct accesses to /updatenow will FAIL
bool bFormatAccessed = false;
uint16_t fileCRC = 0;
int  nUploadSize = 0;
long _SuppliedFileSize = 0;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
bool checkFile(File &file);
void addTableData(String& HTML, String dta);
void rootRedirect();
String getContentType(String filename);
bool handleFileRead(String path);
void onNotFound();
void onErase();
void onFormatSPIFFS();
void onFormatNow();
void onFormatDone();
void onWMConfig();
void onResetWifi();
void onUploadBegin();
void onUploadCompletion();
void onUploadProgression();
void build404Response(String& content, String file);
void build500Response(String& content, String file);

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
	
	server.on("/wmconfig", onWMConfig);
	server.on("/resetwifi", onResetWifi);
  server.on("/erase", HTTP_POST, onErase);  // erase file from SPIFFS

  // Magical code originally shamelessly lifted from Arduino WebUpdate example, then greatly modified
  // This allows pushing new firmware to the ESP from a WEB BROWSER!
  // Added authentication and a sequencing flag to ensure this is not bypassed
  // You can also upload files to SPIFFS via this same portal
  //
  // Initial launch page
  server.on("/update", HTTP_GET, onUploadBegin);
  // handle attempts to browse the /updatenow path - force redirect to root
  server.on("/updatenow", HTTP_GET, []() {  
    DebugPort.println("WEB: GET /updatenow - ILLEGAL - root redirect");
    rootRedirect();
  });
  // valid upload attempts must use post, AND they must have also passed thru /update (bUpdateAccessed = true)
  server.on("/updatenow", HTTP_POST, onUploadCompletion, onUploadProgression);

  // SPIFFS formatting
  server.on("/formatspiffs", HTTP_GET, onFormatSPIFFS);
  server.on("/formatnow", HTTP_GET, []() {     // deny browse access
    DebugPort.println("WEB: GET /formatnow - ILLEGAL - root redirect");
    rootRedirect();
  });
	server.on("/formatnow", HTTP_POST, onFormatNow);  // access via POST is legal, but only if bFormatAccess == true

  // NOTE: this serves the default home page, and favicon.ico
  server.onNotFound([]() 
  {                                                      // If the client requests any URI
    if (!handleFileRead(server.uri())) {                  // send it if it exists
      onNotFound();
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
      file.close();                                     // if not, close the file
    }
    if(!file) {
      DebugPort.println("\tFile exists, but could not be read?");  // dodgy file - throw error back to client

      String content;
      build500Response(content, path);
      server.send(500, "text/html", content);
      return false;                                     // If the file is broken, return false
    }
    else {
      server.streamFile(file, contentType);             // File good, send it to the client
      file.close();                                     // Then close the file 
      return true;
    }
  }
  DebugPort.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}

const char* stdHeader = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="CACHE-CONTROL" content="NO-CACHE">
</head>
<style>
body { font-family: Arial, Helvetica, sans-serif; }
th { text-align: left; }
.throb { animation: throbber 1s linear infinite; }
@keyframes throbber { 50% { opacity: 0; } }
</style>
)=====";

const char* updateIndex = R"=====(
<script>
// globals
var sendSize;
var ws;

function _(el) {
 return document.getElementById(el);
}

function onWebSocket(event) {
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

function init() {
 ws = new WebSocket('ws://' + window.location.hostname + ':81/');
 ws.onmessage = onWebSocket;
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
  setTimeout( function() { window.location.assign('/'); }, 5000);    
 }
 else {
  setTimeout( function() { location.assign('/update'); }, 500);    
 }
}

function errorHandler(event) {
 _("status").innerHTML = "Upload Failed";
}

function abortHandler(event) {
 _("status").innerHTML = "Upload Aborted";
}

function onErase(fn) {
 if(confirm('Do you really want to erase ' + fn +' ?')) {
  var formdata = new FormData();
  formdata.append("filename", fn);
  var ajax = new XMLHttpRequest();
  ajax.open("POST", "/erase");
  ajax.send(formdata);
  setTimeout(function () { location.reload(); }, 500);    
 }
}

function onBrowseChange() {
  _("upload").hidden = false;
  _("progressBar").hidden = false;
  _("status").hidden = false;
  _("loaded_n_total").hidden = false;
  _("spacer").hidden = false;
}
</script>

<title>Afterburner update</title>
<body onload="javascript:init()">
 <h1>Afterburner update</h1>
 <form id='upload_form' method="POST" enctype="multipart/form-data" autocomplete="off">
  <label for='file1'>Select a file to upload:<br></label>
  <input type="file" name="file1" id="file1" size="50" onchange="onBrowseChange()"><p>
  <input id="upload" type='button' onclick='uploadFile()' value='Upload' hidden>
  <progress id='progressBar' value='0' max='100' style='width:300px;' hidden></progress>
  <p id='spacer' hidden> </p>
  <input type='button' onclick=window.location.assign('/') id='cancel' value='Cancel'>
  <h3 id='status' hidden></h3>
  <div id='loaded_n_total' hidden></div>
 </form>
)=====";


void onWMConfig() 
{
  DebugPort.println("WEB: GET /wmconfig");
	server.send(200, "text/plain", "Start Config Portal - Retaining credential");
	DebugPort.println("Starting web portal for wifi config");
  delay(500);
  wifiEnterConfigPortal(true, false, 3000);
}

void onResetWifi() 
{
  DebugPort.println("WEB: GET /resetwifi");
	server.send(200, "text/plain", "Start Config Portal - Resetting Wifi credentials!");
	DebugPort.println("diconnecting client and wifi, then rebooting");
  delay(500);
  wifiEnterConfigPortal(true, true, 3000);
}


//<p><a href="/update"> <button type="button">Add</button></a>
//<p><a href="/"><button type="button">Home</button></a>

void onNotFound() 
{
  String path = server.uri();
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file

  String message;
  build404Response(message, path);

	server.send(404, "text/html", message);
}

void rootRedirect()
{
  server.sendHeader("Location","/");      // reselect the update page
  server.send(303);
}

bool sendWebSocketString(const char* Str)
{
//  CProfile profile;
	if(webSocket.connectedClients()) {
//    unsigned long tCon = profile.elapsed(true);
    bTxWebData = true;              // OLED tx data animation flag
    webSocket.broadcastTXT(Str);
//    unsigned long tWeb = profile.elapsed(true);
//    DebugPort.printf("Websend times : %ld,%ld\r\n", tCon, tWeb); 
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

bool isWebSocketClientChange() 
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

void listSPIFFS(const char * dirname, uint8_t levels, String& HTMLreport, int withHTMLanchors) 
{
  char msg[128];
  File root = SPIFFS.open(dirname);  
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

  HTMLreport += "<hr><h3>Current SPIFFS contents:</h3>";

  // create HTML table header
  HTMLreport += R"=====(<table>
<tr>
<th></th>
<th style="width:200px">Name</th>
<th style="width:60px">Size</th>
<th></th>
</tr>
)=====";
  File file = root.openNextFile();
  while (file) {
    HTMLreport += "<tr>\n";
    if (file.isDirectory()) {
      addTableData(HTMLreport, "DIR");
      addTableData(HTMLreport, file.name());
      addTableData(HTMLreport, "");
      addTableData(HTMLreport, "");

      sprintf(msg, "  DIR : %s", file.name());
      DebugPort.println(msg);

      if (levels) {
        listSPIFFS(file.name(), levels - 1, HTMLreport);
      }
    } else {
      String fn = file.name();
      String ers;
      if(withHTMLanchors == 2)
        ers = "<input type=\"button\" value=\"X\" onClick=\"onErase('" + fn + "')\">";
      if(withHTMLanchors) {
        if(fn.endsWith(".html") || fn.endsWith(".htm")) {
          String fn2(fn);
          fn = "<a href=\"" + fn2 + "\">" + fn2 + "</a>";
        }
      }
      String sz; sz += int(file.size());
      addTableData(HTMLreport, "");
      addTableData(HTMLreport, fn);
      addTableData(HTMLreport, sz);
      addTableData(HTMLreport, ers);

      sprintf(msg, "  FILE: %s  SIZE: %d", fn.c_str(), file.size());
      DebugPort.println(msg);
    }
    HTMLreport += "</tr>\n";
    file = root.openNextFile();
  }
  HTMLreport += "</table>\n";

  if(withHTMLanchors) {
    char usage[128];
    int used = SPIFFS.usedBytes();
    int total = SPIFFS.totalBytes();
    float percent = used * 100. / total;
    sprintf(usage, "<p><b>Usage</b><br> %d / %d bytes  (%.1f%%)\n<p>", used, total, percent);
    HTMLreport += usage;
  }    
}    

void addTableData(String& HTML, String dta) 
{
  HTML += "<td>";
  HTML += dta;
  HTML += "</td>\n";
}

// erase a file from SPIFFS partition
void onErase()
{
  String filename = server.arg("filename");        // get request argument value by name

  if(filename.length() != 0)  {
    DebugPort.printf("onErase: %s ", filename.c_str());
    if(SPIFFS.exists(filename.c_str())) {
      SPIFFS.remove(filename.c_str());
      DebugPort.println("ERASED\r\n");
    }
    else
      DebugPort.println("NOT FOUND\r\n");
  }
}

// function called upon completion of file (form) upload
void onUploadCompletion()
{
  _SuppliedFileSize = 0;
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
}

void onUploadBegin()
{
  DebugPort.println("WEB: GET /update");
  sCredentials creds = NVstore.getCredentials();
  if (!server.authenticate(creds.webUpdateUsername, creds.webUpdatePassword)) {
    return server.requestAuthentication();
  }
  bUpdateAccessed = true;
  bFormatAccessed = false;
#ifdef USE_EMBEDDED_WEBUPDATECODE    
  String SPIFFSinfo;
  listSPIFFS("/", 2, SPIFFSinfo, 2);
  String content = stdHeader;
  content += updateIndex + SPIFFSinfo;
  content += "<p><button onclick=window.location.assign('/formatspiffs')>Format SPIFFS</button>";
  content += "</body></html>";
  server.send(200, "text/html", content );
#else
    handleFileRead("/uploadfirmware.html");
#endif
}

void onUploadProgression()
{
  if(bUpdateAccessed) {  // only allow progression via /update, attempts to directly access /updatenow will fail
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      String filename = upload.filename;
      // CRC checking
      nUploadSize = upload.totalSize;

      DebugPort.setDebugOutput(true);
      if(filename.endsWith(".bin")) {
        DebugPort.printf("Update: %s\r\n", filename.c_str());
        int sizetouse = -1;   //start with max available size
        if(_SuppliedFileSize) {
          sizetouse = _SuppliedFileSize; // adapt to websocket supplied size
        }
        if (!Update.begin(sizetouse)) { 
          Update.printError(DebugPort);
        }
      }
      else {
        if(!filename.startsWith("/")) filename = "/"+filename;
        DebugPort.printf("handleFileUpload Name: %s\r\n", filename.c_str());
        fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
        SPIFFSupload = fsUploadFile ? 1 : 2;
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
        sendWebSocketString(JSON);  // feedback proper byte count of update to browser via websocket
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
        delay(2000);
      if(SPIFFSupload) {
        if(fsUploadFile) {
          fsUploadFile.close();                               // Close the file again
          DebugPort.printf("handleFileUpload Size: %d\r\n", upload.totalSize);
        }
      }
      else {
        if(!CheckFirmwareCRC(_SuppliedFileSize))
          Update.abort();

        if (Update.end()) { 
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
}

/***************************************************************************************
 * FORMAT SPIFFS HANDLING
 *
 * User must first access /formatspiffs.
 * If not already authenticated, an Username/Password challenge is presented
 * If that passes, bFormatAccessed is set, unlocking access to the /formatnow path
 * The presneted web page offers Format and Cancel button.
 * Cancel will immediatly return to the file upload path '/update'
 * Format will then present a confirmation dialog, user must press Yes to proceed.
 * 
 * Assuming Yes was pressed, a HTTP POST to /format now with the payload 'confirm'='yes' is performed
 * The /formatnow handler will check that confirm does equal yes, and that bFormatAccessed was set
 * If all good SPIFFS is re-formatted - no response is set.
 * The javascript though from the /formatspiffs page performs a reload shortly after the post (200ms timeout)
 * 
 * As bFormatAccessed is still set, a confimration page is the presented advising files now need to be uploaded
 * A button allows direct access to /update
 */

void onFormatSPIFFS()
{
  DebugPort.println("WEB: GET /formatspiffs");
  bUpdateAccessed = false;
  String content = stdHeader;
  if(!bFormatAccessed) {
    sCredentials creds = NVstore.getCredentials();
    if (!server.authenticate(creds.webUpdateUsername, creds.webUpdatePassword)) {
      return server.requestAuthentication();
    }
    bFormatAccessed = true;   // only set after we pass authentication

    content += formatIndex;
  }
  else {
    bFormatAccessed = false;

    content += formatDoneContent;
  }
  server.send(200, "text/html", content );
}

const char* formatDoneContent = R"=====(<body>
<h1>SPIFFS partition has been formatted</h1>
<h3>You must now upload the web content.</h3>
<p>Latest web content can be downloaded from <a href=\"http://www.mrjones.id.au/afterburner/firmware.html\" target=\"_blank\">http://www.mrjones.id.au/afterburner/firmware.html</a>
<h4 class="throb">Please ensure you unzip the web page content, then upload all the files contained.</h4>
<p><button onclick=window.location.assign('/update')>Upload web content</button> 
</body>
</html>
)=====";

const char* formatIndex = R"=====(
<script>
function init() {
}
function onFormat() {
 var formdata = new FormData();
 if(confirm('Do you really want to reformat the SPIFFS partition ?')) {
  formdata.append("confirm", "yes");
  setTimeout(function () { location.reload(); }, 200);    
 }
 else {
  formdata.append("confirm", "no");
  setTimeout(function () { location.assign('/update'); }, 200);    
 }
 var ajax = new XMLHttpRequest();
 ajax.open("POST", "/formatnow");
 ajax.send(formdata);
}
</script>
<title>Afterburner SPIFFS format</title>
<body onload="javascript:init()">
<h1>Format SPIFFS partition</h1>
<h3 class="throb">CAUTION!  This will erase all web content</h1>
<p><button onClick='onFormat()'>Format</button><br>
<p><a href="/update"><button>Cancel</button></a>
</body>
</html>
)=====";


void onFormatNow() 
{
  // HTTP POST handler, do not need to return a web page!
  DebugPort.println("WEB: POST /formatnow");
  String confirm = server.arg("confirm");        // get request argument value by name
  if(confirm == "yes" && bFormatAccessed) {      // confirm user agrees, and we did pass thru /formatspiffs first
	  DebugPort.println("Formatting SPIFFS partition");
    SPIFFS.format();                             // re-format the SPIFFS partition
  }
  else {
    bFormatAccessed = false;                     // user cancelled upon last confirm popup, or not authenticated access
    rootRedirect();
  }
}



/***************************************************************************************
 * HTTP RESPONSE 404 - FILE NOT FOUND HANDLING
 */
void build404Response(String& content, String file)
{
content += stdHeader;
content += R"=====(<body>
<h1>404: File Not Found</h1>
<p>URI: <b><i>)=====";
content +=  file;
content += R"=====(</i></b><br>
Method: )=====";
content += (server.method() == HTTP_GET) ? "GET" : "POST";
content += "<br>Arguments: ";
for (uint8_t i = 0; i < server.args(); i++) {
	content += " " + server.argName(i) + ": " + server.arg(i) + "<br>";
}
content += R"=====(<hr>
<p>Please check the URL.<br>
If OK please try uploading the file from the web content.
<p>Latest web content can be downloaded from <a href="http://www.mrjones.id.au/afterburner/firmware.html" target="_blank">http://www.mrjones.id.au/afterburner/firmware.html</a>
<h4 class="throb">Please ensure you unzip the web page content, then upload all the files contained.</h4>
<p><a href="/update"><button>Upload web content</button></a><br>
)=====";

String SPIFFSinfo;
listSPIFFS("/", 2, SPIFFSinfo, 1);
content += SPIFFSinfo;
content += "</body>";
content += "</html>";
}

/***************************************************************************************
 * HTTP RESPONSE 500 - SERVER ERROR HANDLING
 */
void build500Response(String& content, String file)
{
content = stdHeader;
content += R"=====(<body>
<h1>500: Internal Server Error</h1>
<h3 class="throb">Sorry, cannot open file</h3>
<p><b><i> ")=====";
content += file;
content += R"=====(" </i></b> exists, but cannot be streamed?
<hr>
<p>Recommended remedy is to re-format the SPIFFS partition, then reload the web content files.
<br>Latest web content can be downloaded from <a href="http://www.mrjones.id.au/afterburner/firmware.html" target="_blank">http://www.mrjones.id.au/afterburner/firmware.html</a> <i>(opens in new page)</i>.
<p>To format the SPIFFS partition, press <button onClick=location.assign('/formatspiffs')>Format SPIFFS</button>
<p>You will then need to upload each file of the web content by using the subsequent "<b>Upload</b>" button.
<hr>
<h4 class="throb">Please ensure you unzip the web page content, then upload all the files contained.</h4>
</body>
</html>
)=====";
}

