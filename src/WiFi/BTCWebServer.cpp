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
#include "../../lib/WiFiManager-dev/WiFiManager.h"
#include <SPIFFS.h>
#include "../Utility/NVStorage.h"
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "BrowserUpload.h"
#include <Update.h>
#include "WebContentDL.h"

#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>
#include <HTTPBodyParser.hpp>
#include <HTTPMultipartBodyParser.hpp>
#include <HTTPURLEncodedBodyParser.hpp>
#include <WebsocketHandler.hpp>
#include <FreeRTOS.h>
#include "../OLED/ScreenManager.h"
// #include "../Utility/ABpreferences.h"

extern CScreenManager ScreenManager;

using namespace httpsserver;

extern WiFiManager wm;
extern const char* stdHeader;
extern const char* formatIndex;
extern const char* updateIndex;
extern const char* formatDoneContent;
extern const char* rebootIndex;

extern void checkSplashScreenUpdate();

size_t streamFileSSL(fs::File &file, const String& contentType, httpsserver::HTTPResponse* pSSL);
void streamFileCoreSSL(const size_t fileSize, const String & fileName, const String & contentType);

SSLCert* pCert;
HTTPSServer * secureServer;
HTTPServer * insecureServer;
HTTPServer * WSserver;
#if USE_SSL_LOOP_TASK == 1
void SSLloopTask(void *);
#endif

sBrowserUpload BrowserUpload;
#ifdef OLD_SERVER
// WebServer server(80);
// WebSocketsServer webSocket = WebSocketsServer(81);
#endif
CGetWebContent GetWebContent;

bool bRxWebData = false;   // flags for OLED animation
bool bTxWebData = false;
bool bUpdateAccessed = false;  // flag used to ensure browser update always starts via GET /update. direct accesses to POST /update will FAIL
bool bFormatAccessed = false;
bool bFormatPerformed = false;
long _SuppliedFileSize = 0;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
bool checkFile(File &file);
void addTableData(String& HTML, String dta);
String getContentType(String filename);
bool handleFileRead(String path, HTTPResponse* res=NULL);
void onNotFound();
#ifdef OLD_SERVER
void onFormatSPIFFS();
void onFormatDone();
void rootRedirect();
void onRename();
void onWMConfig();
void onResetWifi();
#else
void onReboot(HTTPRequest * req, HTTPResponse * res);
// void onDoReboot(HTTPRequest * req, HTTPResponse * res);
void onFormatSPIFFS(HTTPRequest * req, HTTPResponse * res);
void onFormatNow(HTTPRequest * req, HTTPResponse * res);
void onFormatDone(HTTPRequest * req, HTTPResponse * res);
void rootRedirect(HTTPRequest * req, HTTPResponse * res);
void onErase(HTTPRequest * req, HTTPResponse * res);
void onUpload(HTTPRequest* req, HTTPResponse* res);
void onUploadBegin(HTTPRequest* req, HTTPResponse* res);
void onUploadProgression(HTTPRequest* req, HTTPResponse* res);
void onUploadCompletion(HTTPRequest* req, HTTPResponse* res);
void onRename(HTTPRequest* req, HTTPResponse* res);
void onWMConfig(HTTPRequest* req, HTTPResponse* res);
void onResetWifi(HTTPRequest* req, HTTPResponse* res);
void doDefaultWebHandler(HTTPRequest * req, HTTPResponse * res);
#endif
void build404Response(HTTPRequest * req, String& content, String file);
void build500Response(String& content, String file);
bool checkAuthentication(HTTPRequest * req, HTTPResponse * res, int credID=0);


// As websockets are more complex, they need a custom class that is derived from WebsocketHandler
class JSONHandler : public WebsocketHandler {
public:
  // This method is called by the webserver to instantiate a new handler for each
  // client that connects to the websocket endpoint
  static WebsocketHandler* create();

  // This method is called when a message arrives
  void onMessage(WebsocketInputStreambuf * input);

  // Handler function on connection close
  void onClose();
};

// Max clients to be connected to the JSON handler
#define MAX_CLIENTS 4


// Simple array to store the active clients:
JSONHandler* activeClients[MAX_CLIENTS];


// In the create function of the handler, we create a new Handler and keep track
// of it using the activeClients array
WebsocketHandler * JSONHandler::create() {
  DebugPort.println("Creating new JSON client!");
  JSONHandler * handler = new JSONHandler();
  for(int i = 0; i < MAX_CLIENTS; i++) {
    if (activeClients[i] == nullptr) {
      activeClients[i] = handler;
      break;
    }
  }
  return handler;
}

// When the websocket is closing, we remove the client from the array
void JSONHandler::onClose() {
  for(int i = 0; i < MAX_CLIENTS; i++) {
    if (activeClients[i] == this) {
      activeClients[i] = nullptr;
    }
  }
}

// Finally, passing messages around. If we receive something, we send it to all
// other clients
void JSONHandler::onMessage(WebsocketInputStreambuf * inbuf) {
  // Get the input message
  std::ostringstream ss;
  std::string msg;
  ss << inbuf;
  msg = ss.str();

/*  // Send it back to every client
  for(int i = 0; i < MAX_CLIENTS; i++) {
    if (activeClients[i] != nullptr) {
      activeClients[i]->send(msg, SEND_TYPE_TEXT);
    }
  }*/

  bRxWebData = true;
/*    char cmd[256];
    memset(cmd, 0, 256);
    for (int i = 0; i < length && i < 256; i++) {
      cmd[i] = payload[i];
    }
    interpretJsonCommand(cmd);  // send to the main heater controller decode routine
*/
  char cmd[256];
  memset(cmd, 0, 256);
  if(msg.length() < 256) {
    strcpy(cmd, msg.c_str());
    interpretJsonCommand(cmd);  // send to the main heater controller decode routine
  }
  
}




const char* getWebContent(bool start) {
  
  if(isWifiSTAConnected() && start) {
    GetWebContent.start();
  }

  return GetWebContent.getFilename();
}

SemaphoreHandle_t SSLSemaphore = NULL;

void SSLkeyTask(void *) {
  DebugPort.println("SSL creation starting");

  pCert = new SSLCert();

  ABpreferences  SSLkeyStore;
  SSLkeyStore.begin("SSLkeys");

  if(SSLkeyStore.hasBytes("Certificate")) {
    ScreenManager.showBootMsg("Loading SSL cert.");

    DebugPort.println("Using stored SSL certificate");
    int len;

    len = SSLkeyStore.getBytesLength("Certificate");
    unsigned char* pCertData = new unsigned char[len];              // POTENTIAL LEAK HERE DUE TO LAME SSL LIBARY POINTER COPY
    SSLkeyStore.getBytes("Certificate", pCertData, len);
    pCert->setCert(pCertData, len);

    len = SSLkeyStore.getBytesLength("PrivateKey");
    unsigned char* pPKData = new unsigned char[len];              // POTENTIAL LEAK HERE DUE TO LAME SSL LIBARY POINTER COPY
    SSLkeyStore.getBytes("PrivateKey", pPKData, len);
    pCert->setPK(pPKData, len);

    // vTaskDelay(10000);  // TEST
  }
  else {
    DebugPort.println("Creating SSL certificate - this may take a while...");
    ScreenManager.showBootMsg("Creating SSL cert.");

    int createCertResult = createSelfSignedCert(
                          *pCert,
                          KEYSIZE_2048,
                          "CN=myesp.local,O=acme,C=US");
 
    DebugPort.println("SSL certificate created");
    if (createCertResult != 0) {
      DebugPort.printf("Error generating certificate");
    }
    else {
      SSLkeyStore.putBytes("Certificate", pCert->getCertData(), pCert->getCertLength());
      SSLkeyStore.putBytes("PrivateKey", pCert->getPKData(), pCert->getPKLength());
    }
  }
  SSLkeyStore.end();
      DebugPort.printf("Certificate: length = %d\r\n", pCert->getCertLength());
      for(int i=0; i<pCert->getCertLength(); i++ ) {
        for(int j=0; (j < 32) && (i<pCert->getCertLength()); j++) {
          DebugPort.printf("%02X ", pCert->getCertData()[i++]);
        }
        DebugPort.println("");
      }
      DebugPort.println("");

      DebugPort.printf("Private key: length = %d\r\n", pCert->getPKLength());
      for(int i=0; i<pCert->getPKLength(); i++ ) {
        for(int j=0; (j < 32) && (i < pCert->getPKLength()); j++) {
          DebugPort.printf("%02X ", pCert->getPKData()[i++]);
        }
        DebugPort.println("");
      }
      DebugPort.println("");

  xSemaphoreGive(SSLSemaphore);
  vTaskDelete(NULL);
}


void initWebServer(void) {

  if (MDNS.begin("Afterburner")) {
    DebugPort.println("MDNS responder started");
  }

  // ScreenManager.showBootMsg("Preparing SSL cert.");

  // create SSL certificate, but off load to a task with a BIG stack;
  SSLSemaphore = xSemaphoreCreateBinary();

  TaskHandle_t SSLTask;
  xTaskCreate(SSLkeyTask,
             "SSLkeyTask",
             16384,
             NULL,
             TASKPRIORITY_SSL_CERT,   // low priority as this blocks BIG time
             &SSLTask);

  // int tick = 0;
  // char animation[4] = { '-', '\\', '|', '/'};
  while(!xSemaphoreTake(SSLSemaphore, 250)) {
    ScreenManager.showBootWait(1);
  }
  ScreenManager.showBootWait(0);
  ScreenManager.showBootMsg("Starting web server");
    
  vSemaphoreDelete(SSLSemaphore);

  WSserver = new HTTPServer(81);
  insecureServer = new HTTPServer();
  secureServer = new HTTPSServer(pCert);
  // secureServer->registerNode(SJSONNode);

  DebugPort.println("HTTPS server created");

  ResourceNode * WSnodeRoot = new ResourceNode("/", "GET", [](HTTPRequest * req, HTTPResponse * res){
    res->print("Insecure websocket lives here!!!");
  });  
  WSserver->registerNode(WSnodeRoot);
  WebsocketNode * WebsktNode = new WebsocketNode("/", &JSONHandler::create);
  WSserver->registerNode(WebsktNode);
  insecureServer->registerNode(WebsktNode);
  secureServer->registerNode(WebsktNode);       // associated secure websocket

  ResourceNode * rebootNode = new ResourceNode("/reboot", "", &onReboot);
  // ResourceNode * formatspiffsGet = new ResourceNode("/formatspiffs", "GET", &onFormatSPIFFS);
  ResourceNode * formatNode = new ResourceNode("/formatspiffs", "", &onFormatNow);
  // // handle attempts to browse the /formatnow path - force redirect to root
  // ResourceNode * formatnowGet = new ResourceNode("/formatnow", "GET", [](HTTPRequest * req, HTTPResponse * res){
  //   DebugPort.println("WEB: GET /formatnow - ILLEGAL - root redirect");
  //   rootRedirect(req, res);
  // });
  // ResourceNode * formatnowPost = new ResourceNode("/formatnow", "POST", &onFormatNow);
  // ResourceNode * formatnowNode = new ResourceNode("/formatnow", "", &onFormatNow);  // POST/GET handled by onFormatNow
  // ResourceNode * updateGet = new ResourceNode("/update", "GET", &onUploadBegin);
  // // handle attempts to browse the /updatenow path - force redirect to root
  // ResourceNode * updatenowGet = new ResourceNode("/updatenow", "GET", [](HTTPRequest * req, httpsserver::HTTPResponse * res){
  //   DebugPort.println("WEB: GET /updatenow - ILLEGAL - root redirect");
  //   rootRedirect(req, res);
  // });
  // // // valid upload attempts must use post, AND they must have also passed thru /update (bUpdateAccessed = true)
  // ResourceNode * updatenowPost = new ResourceNode("/updatenow", "POST", &onUploadProgression);
  ResourceNode * updateNode = new ResourceNode("/update", "", &onUpload);
  ResourceNode * renameNode = new ResourceNode("/rename", "POST", &onRename);
  ResourceNode * eraseNode = new ResourceNode("/erase", "POST", &onErase);
  ResourceNode * wmconfigNode = new ResourceNode("/wmconfig", "GET", &onWMConfig);
  ResourceNode * resetwifiNode = new ResourceNode("/resetwifi", "GET", &onResetWifi);
  

  insecureServer->registerNode(rebootNode);     
  insecureServer->registerNode(formatNode);
  insecureServer->registerNode(updateNode);
  insecureServer->registerNode(renameNode);
  insecureServer->registerNode(eraseNode);
  insecureServer->registerNode(resetwifiNode);
  insecureServer->registerNode(wmconfigNode);
  // insecureServer->registerNode(formatspiffsGet);
/*  
  // insecureServer->registerNode(rebootGet);
  // insecureServer->registerNode(rebootPost);  
  // insecureServer->registerNode(formatspiffsGet);
  // insecureServer->registerNode(formatnowGet);
  // insecureServer->registerNode(formatnowPost);
  // insecureServer->registerNode(updateGet);
  // insecureServer->registerNode(updatenowGet);
  // insecureServer->registerNode(updatenowPost);
  insecureServer->registerNode(renamePost);
  insecureServer->registerNode(wmconfigGet);
  insecureServer->registerNode(resetwifiGet);*/

  ResourceNode * defaultGet = new ResourceNode("/", "GET", &doDefaultWebHandler);
  
  insecureServer->setDefaultNode(defaultGet);
  secureServer->setDefaultNode(defaultGet);

  WSserver->start();
  insecureServer->start();
  secureServer->start();
  DebugPort.println("HTTPS started");

#ifdef OLD_SERVER

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

  server.on("/reboot", HTTP_GET, onReboot);  // access via POST is legal, but only if bFormatAccess == true
  server.on("/reboot", HTTP_POST, onDoReboot);  // access via POST is legal, but only if bFormatAccess == true

  server.on("/rename", HTTP_POST, onRename);  // access via POST is legal, but only if bFormatAccess == true

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
#endif

#if USE_SSL_LOOP_TASK == 1
  xTaskCreate(SSLloopTask,
             "SSLloopTask",
             16384,
             NULL,
             TASKPRIORITY_SSL_LOOP,   // low priority as this blocks BIG time
             &SSLTask);

  DebugPort.println("HTTP task started");
#endif
  // initWebPageUpdate();
}

#if USE_SSL_LOOP_TASK == 1
void SSLloopTask(void *) {
  for(;;) {
    WSserver->loop();
    insecureServer->loop();
    secureServer->loop();
    vTaskDelay(1);   
  }
}
#endif

// called by main sketch loop()
bool doWebServer(void) 
{
#if USE_SSL_LOOP_TASK != 1
  WSserver->loop();
  insecureServer->loop();
  secureServer->loop();
#endif
    
#ifdef OLD_SERVER
  webSocket.loop();
  server.handleClient();
#endif
  GetWebContent.manage();
  return true;
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".bin")) return "application/octet-stream";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}


bool
findFormArg(HTTPRequest * req, const char* Arg, std::string& value)
{
  HTTPBodyParser *pParser = new HTTPMultipartBodyParser(req);
  while(pParser->nextField()) {
    std::string name = pParser->getFieldName();
    DebugPort.printf("findArg: %s\r\n", name.c_str());
    if (name == Arg) {
      DebugPort.println("found desired Arg");
      char buf[512];
      size_t readLength = pParser->read((byte *)buf, 512);
      value = std::string(buf, readLength);
      delete pParser;
      return true;
    }
  }
  delete pParser;
  return false;
}

void
doDefaultWebHandler(HTTPRequest * req, HTTPResponse * res)
{
  String path = req->getRequestString().c_str();
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  if(path.indexOf("index.html") >= 0) {
    if(!checkAuthentication(req, res, 1)) {
      return;
    }
  }
  if (!handleFileRead(req->getRequestString().c_str(), res)) {                  // send it if it exists

    String message;
    build404Response(req, message, path);

    res->setStatusCode(404);
    res->setStatusText("Not found");
    res->print(message.c_str());
  }
}

bool checkAuthentication(HTTPRequest * req, HTTPResponse * res, int credID) {
  // Get login information from request
  // If you use HTTP Basic Auth, you can retrieve the values from the request.
  // The return values will be empty strings if the user did not provide any data,
  // or if the format of the Authorization header is invalid (eg. no Basic Method
  // for Authorization, or an invalid Base64 token)
  std::string reqUsername = req->getBasicAuthUser();
  std::string reqPassword = req->getBasicAuthPassword();

  // If the user entered login information, we will check it
  if (reqUsername.length() > 0 && reqPassword.length() > 0) {

    sCredentials creds = NVstore.getCredentials();
    if (credID == 0 && reqUsername == creds.webUpdateUsername && reqPassword == creds.webUpdatePassword) {
      return true;
    }
    if (credID == 1 && reqUsername == creds.webUsername && reqPassword == creds.webPassword) {
      return true;
    }
  }
  res->setStatusCode(401);
  res->setStatusText("Unauthorized");
  res->setHeader("WWW-Authenticate", "Basic realm=\"Login Required\"");
  res->setHeader("Content-Type", "text/html");
  res->println("401. Not authorized");
  return false;
}

bool handleFileRead(String path, HTTPResponse *res) { // send the right file to the client (if it exists)
  DebugPort.println("handleFileRead original request: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  path.replace("%20", " ");                             // convert HTML spaces to normal spaces
  String contentType = getContentType(path);            // Get the MIME type
  String pathWithGz = path + ".gz";
  DebugPort.println("handleFileRead conditioned request: " + path);
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {  // If the file exists as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz)) {                     // If the compressed file exists
      path += ".gz";
      DebugPort.println("handleFileRead now .gz request: " + path);
    }
    File file = SPIFFS.open(path, "r");                 // Open it
    if(!checkFile(file)) {                              // check it is readable
      file.close();                                     // if not, close the file
    }
    if(!file) {
      DebugPort.println("\tFile exists, but could not be read?");  // dodgy file - throw error back to client

      String content;
      build500Response(content, path);
      if(res) {
        res->setStatusCode(500);
        res->setStatusText("Internal server error");
        res->print(content.c_str());
      }
#ifdef OLD_SERVER
      else {
        server.send(500, "text/html", content);
      }
#endif
      return false;                                     // If the file is broken, return false
    }
    else {
      if(res) {
        streamFileSSL(file, contentType, res);
      }
#ifdef OLD_SERVER
      else {
        server.streamFile(file, contentType);             // File good, send it to the client
      }
#endif
      file.close();                                     // Then close the file 
      return true;
    }
  }
  DebugPort.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}

  size_t streamFileSSL(fs::File &file, const String& contentType, HTTPResponse* res) {
    String filename = file.name();
    if (filename.endsWith("gz") &&
        contentType != String("application/x-gzip") &&
        contentType != String("application/octet-stream")) {
      res->setHeader("Content-Encoding", "gzip");
    }
    res->setHeader("Content-Type", contentType.c_str());
    res->setHeader("Content-Length", httpsserver::intToString(file.size()));

    DebugPort.print("Streaming");

  // Read the file and write it to the response
    uint8_t buffer[256];
    size_t progressdot = 0;
    size_t done = 0;
    size_t length = file.read(buffer, 256);
    while(length > 0) {

      size_t wr = res->write(buffer, length);
      if(wr > 0) {
        done += wr;
        if(done > progressdot) {
#if USE_SSL_LOOP_TASK != 1          
          feedWatchdog();
#endif
          DebugPort.print(".");
          progressdot += 1024;
        }
      }
      if(wr <= 0) break;

      length = file.read(buffer, 256);
    } 
    return done;
  }


const char* stdHeader = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="CACHE-CONTROL" content="NO-CACHE">
<style>
body { 
 font-family: Arial, Helvetica, sans-serif; 
 zoom: 200%;
}
button {
 background-color: #016ABC;
 color: #fff;
 border-radius: 25px;
 height: 30px;
}
.del {
 color: white;
 font-weight: bold;
 background-color: red;
 border-radius: 50%;
 height: 30px;
 width: 30px;
}
.redbutton {
 color: white;
 font-weight: bold;
 background-color: red;
}
th {
 text-align: left;
}
.throb { 
 animation: throbber 1s linear infinite;
}
@keyframes throbber { 
 50% { 
  opacity: 0; 
 }
}
</style>

<script>
function _(el) {
 return document.getElementById(el);
}
</script>
)=====";

const char* updateIndex = R"=====(
<style>
body {
 background-color: yellowgreen;
}
.inputfile {
 width: 0.1px;
 height: 0.1px;
 opacity: 0;
 overflow: hidden;
 position: absolute;
 z-index: -1;
}
.inputfile + label {
 color: #fff;
 background-color: #016ABC;
 display: inline-block;
 border-style: solid;
 border-radius: 25px;
 border-width: medium;
 border-top-color: #E3E3E3;
 border-left-color: #E3E3E3;
 border-right-color: #979797;
 border-bottom-color: #979797;
}
#filename {
  font-weight: bold;
  font-style: italic;
}
#upload {
  font-weight: bold;
  font-style: italic;
}
</style>
<script>
// globals
var sendSize;
var ws;
var timeDown;
var timeUp;
var ajax;
var uploadErr;

function onWebSocket(event) {
  var response = JSON.parse(event.data);
  var key;
  for(key in response) {
   switch(key) {
    case 'done':
     setTimeout( function() { location.assign('/'); }, 10000);    
     break;
    case 'progress':
     // actual data bytes received as fed back via web socket
     var progress = response[key];
     if(progress >= 0) {
      // normal progression
      _('loaded_n_total').innerHTML = 'Uploaded ' + progress + ' bytes of ' + sendSize;
      var percent = Math.round( 100 * (progress / sendSize));
      _('progressBar').value = percent;
      _('status').innerHTML = percent+'% uploaded.. please wait';
      uploadErr = '';
     }
     else {
      // upload failure
      _('progressBar').value = 0;
      switch(progress) {
        case -1:  uploadErr = 'File too large - SPIFFS upload ABORTED'; break;
        case -2:  uploadErr = 'Write error - SPIFFS upload ABORTED'; break;
        case -3:  uploadErr = 'Update error - Firmware upload ABORTED'; break;
        case -4:  uploadErr = 'Invalid file - Firmware upload ABORTED'; break;
      }
      ajax.abort();
     }
     break;
   }
  }
}

function init() {
 ws = new WebSocket('ws://' + window.location.hostname + ':81/');
 ws.onmessage = onWebSocket;
}

function uploadFile() {
 _('upload_form').hidden = true;
 _('cancel').hidden = true;
 _('upload').hidden = true;
 _('progressBar').hidden = false;
 var file = _('file1').files[0];
 sendSize = file.size;
 console.log(file);
 var JSONmsg = {};
 JSONmsg['UploadSize'] = sendSize;
 var str = JSON.stringify(JSONmsg);
 console.log('JSON Tx:', str);
 ws.send(str);
 var formdata = new FormData();
 formdata.append('update', file);
 ajax = new XMLHttpRequest();
 // progress feedback is handled via websocket JSON sent from controller
 // using server side progress only shows the buffer filling, not actual delivery.
 ajax.addEventListener('load', completeHandler, false);
 ajax.addEventListener('error', errorHandler, false);
 ajax.addEventListener('abort', abortHandler, false);
 ajax.open('POST', '/update');
 ajax.send(formdata);
}

function completeHandler(event) {
 _('status').innerHTML = event.target.responseText;
 _('progressBar').hidden = true;
 _('progressBar').value = 0;
 _('loaded_n_total').innerHTML = 'Uploaded ' + sendSize + ' bytes of ' + sendSize;
 var file = _('file1').files[0];
 if(file.name.endsWith('.bin')) {
  _('status').innerHTML='Rebooting NOW';
  setTimeout(function () { _('status').innerHTML='Rebooted'; }, 2000);    
  setTimeout(function () { _('status').innerHTML='Initialising...'; }, 4000);    
  setTimeout(function () { _('status').innerHTML='Loading /index.html...'; location.assign('/'); }, 7500);    
 }
 else {
  setTimeout( function() { location.assign('/update'); }, 500);    
 }
}

function errorHandler(event) {
 console.log('Error Handler');
 _('status').innerHTML = 'Upload Error?';
 _('status').style.color = 'red';
 setTimeout( function() { location.reload(); }, 2000);    
}

function abortHandler(event) {
 console.log('Abort Handler' + event);
 _('status').innerHTML = uploadErr;
 _('status').style.color = 'red';
 setTimeout( function() { location.reload(); }, 2000);    
}

function onErase(fn) {
 if(confirm('Do you really want to erase ' + fn +' ?')) {
  var formdata = new FormData();
  formdata.append('filename', fn);
  var ajax = new XMLHttpRequest();
  ajax.open('POST', '/erase');
  ajax.send(formdata);
  setTimeout(function () { location.reload(); }, 500);    
 }
}

function onRename(fn) {
  var newname = prompt('Enter new file name', fn);
  if(newname != null && newname != '') {
    var formdata = new FormData();
    formdata.append('oldname', fn);
    formdata.append('newname', newname);
    var ajax = new XMLHttpRequest();
    ajax.open('POST', '/rename');
    ajax.send(formdata);
    setTimeout(function () { location.reload(); }, 500);    
  }
}

function onBrowseChange() {
  _('uploaddiv').hidden = false;
  _('upload').hidden = false;
  _('status').hidden = false;
  _('loaded_n_total').hidden = false;
  _('spacer').hidden = false;
  var file = _('file1').files[0];
  _('filename').innerHTML = file.name;
}

function onformatClick() {
    location.assign('/formatspiffs');
}

</script>

<title>Afterburner update</title>
</head>
<body onload='javascript:init()'>
 <h1>Afterburner update</h1>
 <form id='upload_form' method='POST' enctype='multipart/form-data' autocomplete='off'>
  <input type='file' name='file1' id='file1' class='inputfile' onchange='onBrowseChange()'/>
  <label for='file1'>&nbsp;&nbsp;Select a file to upload&nbsp;&nbsp;</label>
 </form>
 <p>
 <div id='uploaddiv' hidden><span id='filename'></span>&nbsp;<button id='upload' class='throb' onclick='uploadFile()' hidden>Upload</button>
 <progress id='progressBar' value='0' max='100' style='width:300px;' hidden></progress><p></div>
 <p id='spacer' hidden> </p> 
 <div><button onclick=location.assign('/') id='cancel'>Cancel</button></div>
 <h3 id='status' hidden></h3>
 <div id='loaded_n_total' hidden></div>
)=====";


void onWMConfig(HTTPRequest * req, httpsserver::HTTPResponse * res) 
{
  DebugPort.println("WEB: GET /wmconfig");
  // server.send(200, "text/plain", "Start Config Portal - Retaining credential");
  // res->setStatusCode(200);
  // res->setHeader("Content-Type", "text/plain");
  res->print("Start Config Portal - Retaining credential");
  DebugPort.println("Starting web portal for wifi config");
  delay(500);
  wifiEnterConfigPortal(true, false, 3000);
}


void onResetWifi(HTTPRequest * req, httpsserver::HTTPResponse * res) 
{
  DebugPort.println("WEB: GET /resetwifi");
  // server.send(200, "text/plain", "Start Config Portal - Resetting Wifi credentials!");
  // res->setStatusCode(200);
  // res->setHeader("Content-Type", "text/plain");
  res->print("Start Config Portal - Resetting Wifi credentials!");
  DebugPort.println("diconnecting client and wifi, then rebooting");
  delay(500);
  wifiEnterConfigPortal(true, true, 3000);
}


void rootRedirect(HTTPRequest * req, httpsserver::HTTPResponse * res)
{
  res->setHeader("Location","/");      // reselect the update page
  res->setStatusCode(303);
}


bool sendWebSocketString(const char* Str)
{
#ifdef OLD_SERVER
#ifdef WEBTIMES
  CProfile profile;
#endif

  if(webSocket.connectedClients()) {

#ifdef WEBTIMES
    unsigned long tCon = profile.elapsed(true);
#endif

    bTxWebData = true;              // OLED tx data animation flag
    webSocket.broadcastTXT(Str);

#ifdef WEBTIMES
    unsigned long tWeb = profile.elapsed(true);
    DebugPort.printf("Websend times : %ld,%ld\r\n", tCon, tWeb); 
#endif

    return true;
  }
  return false;
#else
#ifdef WEBTIMES
  CProfile profile;
#endif

#ifdef WEBTIMES
    unsigned long tCon = profile.elapsed(true);
#endif

  bool sent = false;
  for(int i=0; i< MAX_CLIENTS; i++) {
    if(activeClients[i]) {
      bTxWebData = true;              // OLED tx data animation flag
      sent = true;
      activeClients[i]->send(Str, WebsocketHandler::SEND_TYPE_TEXT);
    }
  }


#ifdef WEBTIMES
    unsigned long tWeb = profile.elapsed(true);
    DebugPort.printf("Websend times : %ld,%ld\r\n", tCon, tWeb); 
#endif

  return sent;
#endif
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

#ifdef OLD_SERVER
  int numClients = webSocket.connectedClients();
#else
  int numClients = 0;
  for(int i=0; i< MAX_CLIENTS; i++) {
    if(activeClients[i]) numClients++;
  }
#endif
  if(numClients != prevNumClients) {
    bool retval = numClients > prevNumClients;
    prevNumClients = numClients;
    if(retval) {
      DebugPort.println("Increased number of web socket clients, should reset JSON moderator");
      return true;
    }
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
      addTableData(HTMLreport, "");

      sprintf(msg, "  DIR : %s", file.name());
      DebugPort.println(msg);

      if (levels) {
        listSPIFFS(file.name(), levels - 1, HTMLreport);
      }
    } else {
      String fn = file.name();
      String ers;
      String rename;
      if(withHTMLanchors == 2) {
        String htmlNm = fn;
        htmlNm.replace(" ", "%20");
        rename = "<button class='rename' onClick=onRename('" + htmlNm + "')>Rename</button>";
        ers = "<input class='del' type='button' value='X' onClick=onErase('" + htmlNm + "')>";
      }
      if(withHTMLanchors) {
        String fn2;
        if(fn.endsWith(".html")) {
          // can hyperlink .html files
          fn2 = fn;
        }
        else if(fn.endsWith(".html.gz")) {
          // we can hyperlink .html.gz files but we must strip .gz extension for
          // the hyperlink otherwise you get asked if you want to download the .gz, not view web page!
          fn2 = fn;
          fn2.remove(fn2.length()-3, 3);  // strip trailing ".gz"
        }
        if(fn2.length() != 0) {
          fn2.replace(" ", "%20");
          // create hyperlink if web page file
          fn = "<a href=\"" + fn2 + "\">" + file.name() + "</a>";
        }
      }
      String sz( int(file.size()));
      addTableData(HTMLreport, "");
      addTableData(HTMLreport, fn);
      addTableData(HTMLreport, sz);
      addTableData(HTMLreport, rename);
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
void onErase(HTTPRequest * req, HTTPResponse * res)
{
  std::string sfilename;
  findFormArg(req, "filename", sfilename);
  String filename(sfilename.c_str());
  filename.replace("%20", " ");   // convert HTML spaces to real spaces

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
void onUploadCompletion(HTTPRequest * req, HTTPResponse * res)
{
  _SuppliedFileSize = 0;
  DebugPort.println("WEB: POST /updatenow completion");
  // completion functionality
  if(BrowserUpload.isSPIFFSupload()) {
    if(BrowserUpload.isOK()) {
      checkSplashScreenUpdate();
      DebugPort.println("WEB: SPIFFS OK");
      // server.send(200, "text/plain", "OK - File uploaded to SPIFFS");
      res->setStatusCode(200);
      res->setHeader("Content-Type", "text/plain");
      res->print("OK - File uploaded to SPIFFS");
      // javascript reselects the /update page!
    }
    else {
      DebugPort.println("WEB: SPIFFS FAIL");
      // server.send(500, "text/plain", "500: couldn't create file");
      res->setStatusCode(500);
      res->setHeader("Content-Type", "text/plain");
      res->print("500: couldn't create file");
    }
    BrowserUpload.reset();
#if USE_SSL_LOOP_TASK != 1          
    ShowOTAScreen(-1, eOTAbrowser);  // browser update 
#endif
  }
  else {
    if(BrowserUpload.isOK()) {
      DebugPort.println("WEB: FIRMWARE UPDATE OK");
      // server.send(200, "text/plain", "OK - Afterburner will reboot shortly");
      res->setStatusCode(200);
      res->setHeader("Content-Type", "text/plain");
      res->print("OK - Afterburner will reboot shortly");
    }
    else {
      DebugPort.println("WEB: FIRMWARE UPDATE FAIL");
      // server.send(200, "text/plain", "FAIL - Afterburner will reboot shortly");
      res->setStatusCode(200);
      res->setHeader("Content-Type", "text/plain");
      res->print("FAIL - Afterburner will reboot shortly");
    }
    // rootRedirect(req, res);

    forceBootInit();
    delay(1000);
    // javascript redirects to root page so we go there after reboot!
    ESP.restart();                             // reboot
  }
}


void onUpload(HTTPRequest* req, HTTPResponse* res) 
{
  if(req->getMethod() == "GET") {
    onUploadBegin(req, res);
  }
  if(req->getMethod() == "POST") {
    onUploadProgression(req, res);
  }
}

void onUploadBegin(HTTPRequest* req, HTTPResponse* res)
{
  DebugPort.println("WEB: GET /update");
  if(!checkAuthentication(req, res))
    return;
  // if (!server.authenticate(creds.webUpdateUsername, creds.webUpdatePassword)) {
  //   return server.requestAuthentication();
  // }
  bUpdateAccessed = true;
  bFormatAccessed = false;
  bFormatPerformed = false;
#ifdef USE_EMBEDDED_WEBUPDATECODE    
  String SPIFFSinfo;
  listSPIFFS("/", 2, SPIFFSinfo, 2);
  String content = stdHeader;
  content += updateIndex + SPIFFSinfo;
  content += "<p><button class='redbutton' onclick='onformatClick()'>Format SPIFFS</button>";
  content += "</body></html>";
  res->setStatusCode(200);
  res->setHeader("Content-Type", "text/html");
  res->print( content );

#else
    handleFileRead("/uploadfirmware.html");
#endif
}

void onUploadProgression(HTTPRequest * req, httpsserver::HTTPResponse * res)
{
  char JSON[64];

  if(!bUpdateAccessed) {  // only allow progression via /update, attempts to directly access /updatenow will fail
    DebugPort.println("WEB: POST /updatenow forbidden entry");
    rootRedirect(req, res);
  }
  else {

    if(!checkAuthentication(req, res)) {
      // attempt to POST without using /update - forced redirect to root
      bUpdateAccessed = false;
      return;
    }

    HTTPUpload upload;
    HTTPBodyParser *parser;
    std::string contentType = req->getHeader("Content-Type");
    size_t semicolonPos = contentType.find(";");
    if (semicolonPos != std::string::npos) {
      contentType = contentType.substr(0, semicolonPos);
    }
    if (contentType == "multipart/form-data") {
      parser = new HTTPMultipartBodyParser(req);
    } else {
      Serial.printf("Unknown POST Content-Type: %s\n", contentType.c_str());
      return;
    }

    // We iterate over the fields. Any field with a filename is uploaded
    while(parser->nextField()) {
      std::string name = parser->getFieldName();
      std::string sfilename = parser->getFieldFilename();
      std::string mimeType = parser->getFieldMimeType();
      DebugPort.printf("onUploadProgression: field name='%s', filename='%s', mimetype='%s'\n", name.c_str(), sfilename.c_str(), mimeType.c_str());
      // Double check that it is what we expect
      if (name != "update") {
        Serial.println("Skipping unexpected field");
        break;
      }
      // Should check file name validity and all that, but we skip that.
      String filename = sfilename.c_str();
      if(filename[0] != '/')
        filename = "/" + filename;

      upload.filename = filename;
      upload.name = name.c_str();
      upload.type = mimeType.c_str();
      upload.totalSize = 0;
      upload.currentSize = 0;

      int sts = BrowserUpload.begin(filename, _SuppliedFileSize);   // _SuppliedFileSize come in via websocket
      sprintf(JSON, "{\"progress\":%d}", sts);
      sendWebSocketString(JSON);  // feedback proper byte count of update to browser via websocket

      while (!parser->endOfField()) {
#if USE_SSL_LOOP_TASK != 1          
        feedWatchdog();   // we get stuck here for a while, don't let the watchdog bite!
#endif
        upload.currentSize = parser->read(upload.buf, HTTP_UPLOAD_BUFLEN);
        int sts = BrowserUpload.fragment(upload, res);
        if(sts < 0) {
          sprintf(JSON, "{\"progress\":%d}", sts);
          sendWebSocketString(JSON);  // feedback -ve byte count of update to browser via websocket - write error
          break;
        }
        else {
          // upload still in progress?
          if(BrowserUpload.bUploadActive) {  // show progress unless a write error has occured
            DebugPort.print(".");
            if(upload.totalSize) {
              // feed back bytes received over web socket for progressbar update on browser (via javascript)
              sprintf(JSON, "{\"progress\":%d}", upload.totalSize);
              sendWebSocketString(JSON);  // feedback proper byte count of update to browser via websocket
            }
            // show percentage on OLED
            int percent = 0;
            if(_SuppliedFileSize) 
              percent = 100 * upload.totalSize / _SuppliedFileSize;
#if USE_SSL_LOOP_TASK != 1          
            ShowOTAScreen(percent, eOTAbrowser);  // browser update 
#endif
          }
        }
      }
    }

    int sts = BrowserUpload.end(upload);
    sprintf(JSON, "{\"progress\":%d", sts);
    sendWebSocketString(JSON);  // feedback proper byte count of update to browser via websocket
    if(!BrowserUpload.isSPIFFSupload()) {
      sprintf(JSON, "{\"done\":1}");
      sendWebSocketString(JSON);  // feedback proper byte count of update to browser via websocket
    }
    // WSserver->loop();
    delay(2000);

    bUpdateAccessed = false;  // close gate on POST to /updatenow

    delete parser;

    onUploadCompletion(req, res);

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

#ifdef OLD_SERVER
void onFormatSPIFFS()
{
  DebugPort.println("WEB: GET /formatspiffs");
  bUpdateAccessed = false;
  String content = stdHeader;
  if(!bFormatPerformed) {
    sCredentials creds = NVstore.getCredentials();
    if (!server.authenticate(creds.webUpdateUsername, creds.webUpdatePassword)) {
      return server.requestAuthentication();
    }
    bFormatAccessed = true;   // only set after we pass authentication

    content += formatIndex;
  }
  else {
    bFormatAccessed = false;
    bFormatPerformed = false;

    content += formatDoneContent;
  }
  server.send(200, "text/html", content );
}
#else


void onFormatSPIFFS(HTTPRequest * req, HTTPResponse * res)
{
  DebugPort.println("WEB: GET /formatspiffs");
  bUpdateAccessed = false;
  String content = stdHeader;
  if(!bFormatPerformed) {
    if(checkAuthentication(req, res)) {
      bFormatAccessed = true;   // only set after we pass authentication

      content += formatIndex;
      res->setStatusCode(200);
      res->setHeader("Content-Type", "text/html");
      res->print( content );
    }
  }
  else {
    bFormatAccessed = false;
    bFormatPerformed = false;

    content += formatDoneContent;

    res->setStatusCode(200);
    res->setHeader("Content-Type", "text/html");
    res->print( content );
  }
}
#endif

const char* formatDoneContent = R"=====(
<style>
body {
 background-color: yellow;
}
</style>
</head>
<body>
<h1>SPIFFS partition has been formatted</h1>
<h3>You must now upload the web content.</h3>
<p>Latest web content can be downloaded from <a href='http://afterburner.mrjones.id.au/firmware.html' target='_blank'>http://afterburner.mrjones.id.au/firmware.html</a>
<h4 class="throb">Please ensure you unzip the web page content, then upload all the files contained.</h4>
<p><button onclick=location.assign('/update')>Upload web content</button> 
</body>
</html>
)=====";

const char* formatIndex = R"=====(
<style>
body {
 background-color: orangered;
}
</style>
<script>
function init() {
}
function onFormat() {
 var formdata = new FormData();
 if(confirm('Do you really want to reformat the SPIFFS partition ?')) {
  _('throb').innerHTML = 'FORMATTING - Please wait';
  formdata.append('confirm', 'yes');
  setTimeout(function () { location.reload(); }, 200);    
 }
 else {
  formdata.append('confirm', 'no');
  setTimeout(function () { location.assign('/update'); }, 20);    
 }
 var ajax = new XMLHttpRequest();
//  ajax.open('POST', '/formatnow');
 ajax.open('POST', '/formatspiffs');
 ajax.send(formdata);
}
</script>
<title>Afterburner SPIFFS format</title>
</head>
<body onload='javascript:init()'>
<h1>Format SPIFFS partition</h1>
<h3 class='throb' id='throb'>CAUTION!  This will erase all web content</h1>
<p><button class='redbutton' onClick='onFormat()'>Format</button><br>
<p><a href='/update'><button>Cancel</button></a>
</body>
</html>
)=====";


void onFormatNow(HTTPRequest * req, httpsserver::HTTPResponse * res)
{
  if(req->getMethod() == "GET") {
    onFormatSPIFFS(req, res);
    // DebugPort.println("WEB: GET /formatnow - ILLEGAL - root redirect");
    // rootRedirect(req, res);
  }

  if(req->getMethod() == "POST") {
    // HTTP POST handler, do not need to return a web page!
    DebugPort.println("WEB: POST /formatnow");
    std::string value;
    findFormArg(req, "confirm", value);
    if(value == "yes" && bFormatAccessed) {      // confirm user agrees, and we did pass thru /formatspiffs first
      DebugPort.println("Formatting SPIFFS partition");
      SPIFFS.format();                             // re-format the SPIFFS partition
      bFormatPerformed = true;
    }
    else {
      bFormatAccessed = false;                     // user cancelled upon last confirm popup, or not authenticated access
      bFormatPerformed = false;
      rootRedirect(req, res);
    }
  }
}


void onReboot(HTTPRequest * req, httpsserver::HTTPResponse * res)
{  
  if (req->getMethod() == "GET") {
    DebugPort.println("WEB: GET /reboot");
    String content = stdHeader;
    content += rebootIndex;
    res->print(content);
  }
  if (req->getMethod() == "POST") {
    // HTTP POST handler, do not need to return a web page!
    DebugPort.println("WEB: POST /reboot");
    // First, we need to check the encoding of the form that we have received.
    // The browser will set the Content-Type request header, so we can use it for that purpose.
    std::string value;
    if(findFormArg(req, "reboot", value)) {
      if(value == "yes") {      // confirm user agrees, and we did pass thru /formatspiffs first
        DebugPort.println("Rebooting via /reboot");
        ESP.restart();
      }
    }
  }

}

/*void onDoReboot(HTTPRequest * req, HTTPResponse * res) 
{
  // HTTP POST handler, do not need to return a web page!
  DebugPort.println("WEB: POST /reboot");
  // First, we need to check the encoding of the form that we have received.
  // The browser will set the Content-Type request header, so we can use it for that purpose.
  std::string value;
  if(findFormArg(req, "reboot", value)) {
    if(value == "yes") {      // confirm user agrees, and we did pass thru /formatspiffs first
      DebugPort.println("Rebooting via /reboot");
      ESP.restart();
    }
  }
}*/


const char* rebootIndex = R"=====(
<style>
body {
 background-color: orangered;
}
</style>
<script>
function onReboot() {
 if(confirm('Do you really want to reboot the Afterburner ?')) {
   _('info').innerHTML='Rebooting NOW';
  setTimeout(function () { _('info').innerHTML='Rebooted'; }, 2000);    
  setTimeout(function () { _('info').innerHTML='Initialising...'; }, 4000);    
  setTimeout(function () { _('info').innerHTML='Loading /index.html...'; location.assign('/'); }, 7500);    
  var formdata = new FormData();
  formdata.append('reboot', 'yes');
  var ajax = new XMLHttpRequest();
  ajax.open('POST', '/reboot');
  ajax.send(formdata);
  _('info').hidden = false;
 }
 else {
   location.assign('/');
 }
}
</script>
<title>Afterburner Reboot</title>
</head>
<body>
<h1>Reboot Afterburner</h1>
<p>
<h3 class='throb' id='info' hidden>Rebooting - will re-direct to root index</h3>
<button class='redbutton' onClick='onReboot()'>Reboot</button>
&nbsp;&nbsp;&nbsp;&nbsp;<a href='/'><button>Cancel</button></a>
</body>
</html>
)=====";



void onRename(HTTPRequest * req, httpsserver::HTTPResponse * res) 
{
  // HTTP POST handler, do not need to return a web page!
  DebugPort.println("WEB: POST /rename");
  std::string value;
  findFormArg(req, "oldname", value);  // get request argument value by name
  String oldname = value.c_str();   
  findFormArg(req, "newname", value);  // get request argument value by name
  String newname = value.c_str();   

  newname.replace("%20", " ");               // convert html spaces to real spaces
  oldname.replace("%20", " ");
  if(oldname != "" && newname != "") {      
    DebugPort.printf("Renaming %s to %s\r\n", oldname.c_str(), newname.c_str());
    SPIFFS.rename(oldname.c_str(), newname.c_str());
    checkSplashScreenUpdate();
  }
}


/***************************************************************************************
 * HTTP RESPONSE 404 - FILE NOT FOUND HANDLING
 */
void build404Response(HTTPRequest * req, String& content, String file)
{
content += stdHeader;
content += R"=====(</head>
<body>
<h1>404: File Not Found</h1>
<p>URI: <b><i>)=====";
content +=  file;
content += R"=====(</i></b><br>
Method: )=====";
#ifdef OLD_SERVER
content += (server.method() == HTTP_GET) ? "GET" : "POST";
content += "<br>Arguments: ";
for (uint8_t i = 0; i < server.args(); i++) {
	content += " " + server.argName(i) + ": " + server.arg(i) + "<br>";
}
#else
content += req->getMethod().c_str();
content += "<br>Arguments: ";
for(auto it = req->getParams()->beginQueryParameters(); it != req->getParams()->endQueryParameters(); ++it) {
  std::string nm(it->first);
  std::string val(it->second);
	content += " ";
  content += nm.c_str();
  content += ": ";
  content += val.c_str();
  content += "<br>";
}
#endif
content += R"=====(<hr>
<p>Please check the URL.<br>
If OK please try uploading the file from the web content.
<p>Latest web content can be downloaded from <a href="http://afterburner.mrjones.id.au/firmware.html" target="_blank">http://afterburner.mrjones.id.au/firmware.html</a>
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
content += R"=====(</head>
<body>
<h1>500: Internal Server Error</h1>
<h3 class="throb">Sorry, cannot open file</h3>
<p><b><i> ")=====";
content += file;
content += R"=====(" </i></b> exists, but cannot be streamed?
<hr>
<p>Recommended remedy is to re-format the SPIFFS partition, then reload the web content files.
<br>Latest web content can be downloaded from <a href="http://afterburner.mrjones.id.au/firmware.html" target="_blank">http://afterburner.mrjones.id.au/firmware.html</a> <i>(opens in new page)</i>.
<p>To format the SPIFFS partition, press <button class='redbutton' onClick=location.assign('/formatspiffs')>Format SPIFFS</button>
<p>You will then need to upload each file of the web content by using the subsequent "<b>Upload</b>" button.
<hr>
<h4 class="throb">Please ensure you unzip the web page content, then upload all the files contained.</h4>
</body>
</html>
)=====";
}


