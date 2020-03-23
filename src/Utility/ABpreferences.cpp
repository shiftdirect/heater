#include "ABpreferences.h"

#include "nvs.h"

const char * nvs_errors2[] = { "OTHER", "NOT_INITIALIZED", "NOT_FOUND", "TYPE_MISMATCH", "READ_ONLY", "NOT_ENOUGH_SPACE", "INVALID_NAME", "INVALID_HANDLE", "REMOVE_FAILED", "KEY_TOO_LONG", "PAGE_FULL", "INVALID_STATE", "INVALID_LENGHT"};
#define nvs_error(e) (((e)>ESP_ERR_NVS_BASE)?nvs_errors2[(e)&~(ESP_ERR_NVS_BASE)]:nvs_errors2[0])

bool
ABpreferences::hasBytes(const char *key)
{
  size_t len = 0;
  if(!_started || !key){
    return false;
  }
  esp_err_t err = nvs_get_blob(_handle, key, NULL, &len);
  if(err == ESP_ERR_NVS_NOT_FOUND) {  // NOT FOUND - expected!
    return false;
  }
  if(err) { // remaining errors - print it
    log_e("nvs_get_blob len fail: %s %s", key, nvs_error(err));
    return false;
  }
  return len != 0;
}

bool
ABpreferences::hasString(const char *key)
{
  size_t len = 0;
  if(!_started || !key){
    return false;
  }
  esp_err_t err = nvs_get_str(_handle, key, NULL, &len);
  if(err == ESP_ERR_NVS_NOT_FOUND) {  // NOT FOUND - expected!
    return false;
  }
  if(err) { // remaining errors - print it
    log_e("nvs_get_str len fail: %s %s", key, nvs_error(err));
    return false;
  }
  return len != 0;
}