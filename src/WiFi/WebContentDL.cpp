// seek a web page update from the afterburner web server

#include "WebContentDL.h"
#include "../Utility/DebugPort.h"


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

   
void WebPageRequestCB(void* pClass, asyncHTTPrequest* request, int readyState) 
{
  CWebContentDL* pParent = (CWebContentDL*)pClass;
  if(readyState == 4){
    while(request->available()) {
      pParent->queueDLdata(request->available(), request);
    }
    pParent->finalise();

    request->close();
  }
}

CWebContentDL::CWebContentDL()
{
  // _request.setDebug(true);
  _request.onReadyStateChange(WebPageRequestCB, this);
  _request.onData(WebPageDataCB, this);
  _queue = xQueueCreate(10, sizeof(sQueueEntry));
  _fileActive = false;
  _bytecount = 0;
  _queuecount = 0;
}

CWebContentDL::~CWebContentDL()
{
  vQueueDelete(_queue);
}

bool
CWebContentDL::busy() const
{
  return _fileActive || (_request.readyState() != 0 && _request.readyState() != 4) ;
}


void CWebContentDL::get(const char* filename) 
{
  if(_request.readyState() == 0 || _request.readyState() == 4){
    // ensure leading forward slash, required for SPIFFS
    _filename = "";
    if(filename[0] != '/') _filename = "/"; 
    _filename += filename;
    // replace with sanitised name
    filename = _filename.c_str();

    DebugPort.printf("Loading file to SPIFFS: '%s'\r\n", filename);
    if(SPIFFS.exists(filename)) {
      DebugPort.println("Removing existing file from SPIFFS");
      SPIFFS.remove(filename);
    }

    _file = SPIFFS.open(filename, "w");  // Open the file for writing in SPIFFS (create if it doesn't exist)
    _fileActive = true;
    _bytecount = 0;
    _queuecount = 0;

    String URL = "http://afterburner.mrjones.id.au/fota/web";
    URL += filename;
    _request.open("GET", URL.c_str());
    _request.send();
  }
}

void CWebContentDL::process() 
{
  sQueueEntry entry;
  while(xQueueReceive(_queue, &entry, 0)) {
    int16_t len = entry.len;
    if(len == -1) {
      if(_file) {
        _file.close();
        _fileActive = false;
      }
      DebugPort.printf("Downloaded %s (%d bytes) - CLOSED OK\r\n", _filename.c_str(), _bytecount);

    }
    else if(len > 0) {
      if(_file) {
        if(_file.write(entry.data, len) != len) {   // Write the received bytes to the file
          _file.close();
          _fileActive = false;
          DebugPort.printf("Web content downlod - FILE_WRITE error: removing %s\r\n", _filename.c_str());
          SPIFFS.remove(_filename.c_str());  // remove the bad file from SPIFFS
        }
        else {
          _bytecount += len;
        }
      }
    }
    // DebugPort.printf("Len=%d Queuecount=%d/%d total=%d\r\n", entry.len, entry.count, queuecount, webpagecount);
  }
}

int16_t 
CWebContentDL::queueDLdata(int size, asyncHTTPrequest* request) 
{
  sQueueEntry entry;
  
  if(size > sizeof(entry.data)) 
    size = sizeof(entry.data);

  int16_t read = request->responseRead(entry.data, size);

  if(read > 0) {
    // available -= read;
    entry.len = read;
    entry.count = ++_queuecount;

    BaseType_t awoken;
    xQueueSendFromISR(_queue, &entry, &awoken);
  }
  else {
    DebugPort.println(" page read error?");
  }

  return read;
}

void 
CWebContentDL::finalise() 
{
  sQueueEntry entry;

  entry.len = -1;
  entry.count = -1;
  BaseType_t awoken;
  xQueueSendFromISR(_queue, &entry, &awoken);
}


