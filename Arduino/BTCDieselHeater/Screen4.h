#include "stdint.h"

class C128x64_OLED;
class CScreenManager;
class CProtocol;

class CScreen4 : public CScreen {
public:
  CScreen4(C128x64_OLED& display, CScreenManager& mgr);
  void show(const CProtocol& CtlFrame, const CProtocol& HtrFrame);
  void keyHandler(uint8_t event);
};
