/*
   esp32 firmware OTA
   Date: December 2018
   Author: Chris Joyce <https://chrisjoyce911/esp32FOTA>
   Purpose: Perform an OTA update from a bin located on a webserver (HTTP Only)
*/

#ifndef esp32fota_h
#define esp32fota_h

//#include <Arduino.h>
#include <functional>
#include "../../AsyncTCP/src/AsyncTCP.h"
#include "../../asyncHTTPrequest/src/asyncHTTPrequest.h"

class esp32FOTA
{
public:
  esp32FOTA(String firwmareType, int firwmareVersion);
  void execOTA();
  bool execHTTPcheck();
  bool useDeviceID;
  String checkURL;
  void onProgress( std::function<void(size_t, size_t)> func );
  void onComplete( std::function<bool(int)> func );
  void onSuccess( std::function<void()> func );
  void onFail( std::function<void()> func );
  int  getNewVersion() { return _newVersion; };

  bool setURL(const char* URL, const char* expectedProtocol);
  void setupAsync(const char* host);
  void poll(const char* host);
  bool decodeResponse(String payload);
  bool decodeResponse(char* resp);

  const char* getURI() { return _webURI.c_str(); };
  const char* getHost() { return _webhost.c_str(); };

private:
  String getHeaderValue(String header, String headerName);
  String getDeviceID();
  String _firwmareType;
  int _firwmareVersion;
  int _newVersion;
  String _host;
  String _bin;
  int _port;
  std::function<bool(int)> _onComplete;
  std::function<void()> _onSuccess;
  std::function<void()> _onFail;
  AsyncClient _webclient;
  String _webhost;
  String _webURI;
  String _webprotocol;
  int    _webport;
};

#endif