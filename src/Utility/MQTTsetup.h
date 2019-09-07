#include "NVStorage.h"


class CMQTTsetup {
  char _buffer[128];
  int _idx;
  int _mode;
  bool _active;
  sMQTTparams _MQTTsetup;
  bool HandleMQTTsetup(char rxVal);
  void showMQTTmenu(bool init = false);
  bool getMQTTstring(char rxVal, int maxidx, char* pTargetString);
public:
  CMQTTsetup();
  bool Handle(char& rxVal);
  void setActive();
};
