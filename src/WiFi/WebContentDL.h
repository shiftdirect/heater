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
public:
  CWebContentDL();
  ~CWebContentDL();
  void get(const char* filename);
  void process();
  // callback handlers
  int16_t queueDLdata(int size, asyncHTTPrequest* request);
  void finalise(); 
  bool busy() const;
};

void WebPageRequestCB(void* optParm, asyncHTTPrequest* request, int readyState);
void WebPageDataCB(void* optParm, asyncHTTPrequest*, size_t available);


