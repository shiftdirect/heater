/*
   esp32 firmware OTA
   Date: December 2018
   Author: Chris Joyce <https://chrisjoyce911/esp32FOTA>
   Purpose: Perform an OTA update from a bin located on a webserver (HTTP Only)
*/

#include <Arduino.h>
#include "esp32fota.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include "../../ArduinoJson/ArduinoJson.h"
#include "../../asyncHTTPrequest/src/asyncHTTPrequest.h"

extern void forceBootInit();

esp32FOTA::esp32FOTA(String firwmareType, int firwmareVersion)
{
    _firwmareType = firwmareType;
    _firwmareVersion = firwmareVersion;
    useDeviceID = false;
//    _endCallback = NULL;
}

// Utility to extract header value from headers
String esp32FOTA::getHeaderValue(String header, String headerName)
{
    return header.substring(strlen(headerName.c_str()));
}

// OTA Logic
void esp32FOTA::execOTA()
{

    WiFiClient client;
    int contentLength = 0;
    bool isValidContentType = false;

    Serial.println("Connecting to: " + String(_host));
    // Connect to Webserver
    if (client.connect(_host.c_str(), _port))
    {
        // Connection Succeed.
        // Fecthing the bin
        Serial.println("Fetching Bin: " + String(_bin));

        // Get the contents of the bin file
        client.print(String("GET ") + _bin + " HTTP/1.1\r\n" +
                     "Host: " + _host + "\r\n" +
                     "Cache-Control: no-cache\r\n" +
                     "Connection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (client.available() == 0)
        {
            if (millis() - timeout > 5000) 
            {
                Serial.println("Client Timeout !");
                client.stop();
                return;
            }
        }

        while (client.available())
        {
            // read line till /n
            String line = client.readStringUntil('\n');
            // remove space, to check if the line is end of headers
            line.trim();

            if (!line.length())
            {
                //headers ended
                break; // and get the OTA started
            }

            // Check if the HTTP Response is 200
            // else break and Exit Update
            if (line.startsWith("HTTP/1.1"))
            {
                if (line.indexOf("200") < 0)
                {
                    Serial.println("Got a non 200 status code from server. Exiting OTA Update.");
                    break;
                }
            }

            // extract headers here
            // Start with content length
            if (line.startsWith("Content-Length: "))
            {
                contentLength = atoi((getHeaderValue(line, "Content-Length: ")).c_str());
                Serial.println("Got " + String(contentLength) + " bytes from server");
            }

            // Next, the content type
            if (line.startsWith("Content-Type: "))
            {
                String contentType = getHeaderValue(line, "Content-Type: ");
                Serial.println("Got " + contentType + " payload.");
                if (contentType == "application/octet-stream")
                {
                    isValidContentType = true;
                }
            }
        }
    }
    else
    {
        // Connect to webserver failed
        // May be try?
        // Probably a choppy network?
        Serial.println("Connection to " + String(_host) + " failed. Please check your setup");
        // retry??
        // execOTA();
    }

    // Check what is the contentLength and if content type is `application/octet-stream`
    Serial.println("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType));

    // check contentLength and content type
    if (contentLength && isValidContentType)
    {
        // Check if there is enough to OTA Update
        bool canBegin = Update.begin(contentLength);

        // If yes, begin
        if (canBegin)
        {
            Serial.println("Begin OTA. This may take 2 - 5 mins to complete. Things might be quite for a while.. Patience!");
            // No activity would appear on the Serial monitor
            // So be patient. This may take 2 - 5mins to complete
            size_t written = Update.writeStream(client);

            if (written == contentLength)
            {
                Serial.println("Written : " + String(written) + " successfully");
            }
            else
            {
                Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
                // retry??
                // execOTA();
            }

            if ( _onComplete != NULL) {
              if(!_onComplete(contentLength)) {
                Serial.println("ESP32FOTA: OnComplete handler returned false");
                Update.abort();
              }
            }

            if (Update.end())
            {
                Serial.println("OTA done!");
                if (Update.isFinished())
                {
                    Serial.println("Update successfully completed. Rebooting.");
                    if(_onSuccess != NULL) {
                      _onSuccess();
                    }
                    ESP.restart();
                }
                else
                {
                    if(_onFail != NULL) {
                      _onFail();
                    }
                    Serial.println("Update not finished? Something went wrong!");
                }
            }
            else
            {
                if(_onFail != NULL) {
                  _onFail();
                }
                Serial.println("Error Occurred. Error #: " + String(Update.getError()));
            }
        }
        else
        {
            // not enough space to begin OTA
            // Understand the partitions and
            // space availability
            Serial.println("Not enough space to begin OTA");
            client.flush();
        }
    }
    else
    {
        Serial.println("There was no content in the response");
        client.flush();
    }
}

bool esp32FOTA::execHTTPcheck()
{

    String useURL;

    if (useDeviceID)
    {
        // String deviceID = getDeviceID() ;
        useURL = checkURL + "?id=" + getDeviceID();
    }
    else
    {
        useURL = checkURL;
    }

    WiFiClient client;
    _port = 80;

    Serial.println("Getting HTTP");
    Serial.println(useURL);
    Serial.println("------");
    if ((WiFi.status() == WL_CONNECTED))
    { //Check the current connection status

        HTTPClient http;

        http.begin(useURL);        //Specify the URL
        int httpCode = http.GET(); //Make the request

        if (httpCode == 200)
        { //Check is a file was returned
            String payload = http.getString();

            int str_len = payload.length() + 1;
            char* JSONMessage = new char[str_len];
            payload.toCharArray(JSONMessage, str_len);

            StaticJsonBuffer<300> JSONBuffer;                         //Memory pool
            JsonObject &parsed = JSONBuffer.parseObject(JSONMessage); //Parse message

            if (!parsed.success())
            { //Check for errors in parsing
                delete[] JSONMessage;
                Serial.println("Parsing failed");
                delay(5000);
                return false;
            }

            const char *pltype = parsed["type"];
            int plversion = parsed["version"];
            const char *plhost = parsed["host"];
            _port = parsed["port"];
            const char *plbin = parsed["bin"];

            String jshost(plhost);
            String jsbin(plbin);

            _host = jshost;
            _bin = jsbin;

            String fwtype(pltype);

            delete[] JSONMessage;

            if (plversion > _firwmareVersion && fwtype == _firwmareType)
            {
                _newVersion = plversion;
                return true;
            }
            else
            {
                _newVersion = 0;
                return false;
            }
            
        }

        else
        {
            Serial.println("Error on HTTP request");
            return false;
        }

        http.end(); //Free the resources
    }
    return false;
 }

String esp32FOTA::getDeviceID()
{
    char deviceid[21];
    uint64_t chipid;
    chipid = ESP.getEfuseMac();
    sprintf(deviceid, "%" PRIu64, chipid);
    String thisID(deviceid);
    return thisID;
}

/**
 * onProgress, set a callback called during upload, passed directly thru to OTA Updater class
 * @access public 
 * @param {[type]} void (*func)(size_t, size_t)
 */
void esp32FOTA::onProgress( std::function<void(size_t, size_t)> func ) {
  Update.onProgress(func);
}


/**
 * onComplete, set a callback after upload is completed, but not yet verified
 * @access public 
 * @param {[type]} void (*func)(void)
 */
void esp32FOTA::onComplete( std::function<bool(int)> func ) {
  _onComplete = func;
}

/**
 * onSuccess, set a callback after upload is fully completed and verified
 * @access public 
 * @param {[type]} void (*func)(void)
 */
void esp32FOTA::onSuccess( std::function<void()> func ) {
  _onSuccess = func;
}

/**
 * onComplete, set a callback after upload is completed, but not yet verified
 * @access public 
 * @param {[type]} void (*func)(void)
 */
void esp32FOTA::onFail( std::function<void()> func ) {
  _onFail = func;
}
















void FOTAconnectCallback(void* arg, AsyncClient * c) 
{
  esp32FOTA* pInstance = (esp32FOTA*)arg;
  if(pInstance) {
    Serial.printf("ESPFOTA Connected. Sending Data\r\n");
    String resp;

    resp = "GET ";
    resp += pInstance->getURI(); 
    resp += " HTTP/1.1\r\n"

            "Host: ";
    resp += pInstance->getHost();
    resp += "\r\n"
//            "www.mrjones.id.au"  "\r\n"
            "Connection: close\r\n\r\n";

/*    c->write(("GET " + pInstance->getURI() + " HTTP/1.1\r\n"
            "Host: " + pInstance->getHost() + "\r\n"
//            "www.mrjones.id.au"  "\r\n"
            "Connection: close\r\n\r\n")
            .c_str());*/
    c->write(resp.c_str());
  }
}

void FOTAdataCallback(void* arg, AsyncClient* c, void* data, size_t len) 
{
  esp32FOTA* pInstance = (esp32FOTA*)arg;
  if(pInstance == NULL) {
    c->close();
    return;
  }

  Serial.printf("FOTA received with length: %d\r\n", len);

  char JSONMessage[1500];
  memset(JSONMessage, 0, 1500);
  char* pData = (char*)data;
  for (int i = 0; i < len && i < 1499; i++) {
    JSONMessage[i] = pData[i];
  }

  Serial.printf("%s\r\n", JSONMessage);

  pInstance->decodeResponse(JSONMessage);

  c->close();
}

bool esp32FOTA::decodeResponse(String payload) 
{
  int str_len = payload.length() + 1;
  char* JSONMessage = new char[str_len];
  payload.toCharArray(JSONMessage, str_len);
  
  bool retval = decodeResponse(JSONMessage);

  delete[] JSONMessage;

  return retval;
}

bool esp32FOTA::decodeResponse(char* resp)
{
  StaticJsonBuffer<300> JSONBuffer;                         //Memory pool
  JsonObject &parsed = JSONBuffer.parseObject(resp); //Parse message

  if (!parsed.success())
  { //Check for errors in parsing
//      delete[] JSONMessage;
    Serial.println("FOTA Parsing failed\r\n");
    // delay(5000);
    //   return false;
  }

  const char *pltype = parsed["type"];
  int plversion = parsed["version"];
  const char *plhost = parsed["host"];
  _port = parsed["port"];
  const char *plbin = parsed["bin"];

  String jshost(plhost);
  String jsbin(plbin);

  _host = jshost;
  _bin = jsbin;

  String fwtype(pltype);

  if (plversion > _firwmareVersion && fwtype == _firwmareType)
  {
      _newVersion = plversion;
      return true;
  }
  else
  {
      _newVersion = 0;
      return false;
  }
}


bool esp32FOTA::setURL(const char* URL, const char* expectedProtocol)
{
  String url(URL);

  // check for : (http: or https:
  int index = url.indexOf(':');
  if(index < 0) {
    log_e("failed to parse protocol");
    return false;
  }

  _webprotocol = url.substring(0, index);
  if (_webprotocol != expectedProtocol) {
    log_w("unexpected protocol: %s, expected %s", _protocol.c_str(), expectedProtocol);
    return false;
  }

  url.remove(0, (index + 3)); // remove http:// or https://

  index = url.indexOf('/');
  String host = url.substring(0, index);
  url.remove(0, index); // remove host part

/*  // get Authorization
  index = host.indexOf('@');
  if(index >= 0) {
    // auth info
    String auth = host.substring(0, index);
    host.remove(0, index + 1); // remove auth part including @
    _base64Authorization = base64::encode(auth);
  }*/

  // get port
  _webport = 80;
  index = host.indexOf(':');
  if(index >= 0) {
    _webhost = host.substring(0, index); // hostname
    host.remove(0, (index + 1)); // remove hostname + :
    _webport = host.toInt(); // get port
  } else {
    _webhost = host;
  }
  
  _webURI = url;
  log_d("host: %s port: %d url: %s", _webhost.c_str(), _webport, _webURI.c_str());
  Serial.printf("host: %s port: %d url: %s", _webhost.c_str(), _webport, _webURI.c_str());
  return true;
}

/*void esp32FOTA::setupAsync(const char* host)
{
  setURL(host, "http");

  static const char* shost = host;
  _webclient.onError([](void* arg, AsyncClient * c, int8_t error) {
    Serial.printf("ESPFOTA Error %s\r\n", c->errorToString(error));
    c->close();
  });
  _webclient.onTimeout([](void* arg, AsyncClient * c, uint32_t tm) {
    Serial.printf("ESPFOTA Timeout\r\n");
  });
//   _webclient.onConnect([](void* arg, AsyncClient * c) {
//     Serial.printf("ESPFOTA Connected. Sending Data\r\n");
//     c->write(("GET /?format=json HTTP/1.1\r\n"
//               "Host: " + 
//               String(shost) + "\r\n"
//               "Connection: close\r\n\r\n")
//               .c_str());
//   });
  // _webclient.onData([](void* arg, AsyncClient* c, void* data, size_t len) {
  //   Serial.printf("FOTA received with length: %d\r\n", len);
  //   Serial.printf("%s\r\n", (char*)data);
  //   c->close();
  // });
  _webclient.onConnect(FOTAconnectCallback, this);
  _webclient.onData(FOTAdataCallback, this);
}*/

// void esp32FOTA::poll(const char* host) 
// {
//   _webclient.connect(host, 80);
// }


asyncHTTPrequest request;

void FOTArequestCB(void* optParm, asyncHTTPrequest* pRequest, int readyState){
    if(readyState == 4){  // resposnse
        String JSONinfo(pRequest->responseText());
        Serial.println(JSONinfo);
        Serial.println();
//        pRequest->close();
        esp32FOTA* pFOTA = (esp32FOTA*) optParm;
        if(pFOTA) pFOTA->decodeResponse(JSONinfo);
//        delete pRequest;
    }
    if(readyState == 1) {  // connected
      pRequest->send();
    }
}
    
void esp32FOTA::setupAsync(const char* host) 
{
  request.setDebug(true);
  request.onReadyStateChange(FOTArequestCB, this);
}

void esp32FOTA::poll(const char* host) 
{
  asyncHTTPrequest* pRequest = &request/*new asyncHTTPrequest*/;
  // pRequest->setDebug(true);
  // pRequest->onReadyStateChange(FOTArequestCB, this);
  if(pRequest->readyState() == 0 || pRequest->readyState() == 4) {
    bool bOK = pRequest->open("GET", host);
  }
}
