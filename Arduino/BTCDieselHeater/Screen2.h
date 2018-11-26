#include "stdint.h"
#include "display.h"

class C128x64_OLED;
class CProtocol;
class CScreenManager;

class CScreen2 : public CScreen
{
  unsigned long _showSetMode;
  unsigned long _showMode;
  unsigned char _nModeSel;
  void showRunState();
public:
  CScreen2(C128x64_OLED& display, CScreenManager& mgr);
  void show(const CProtocol& CtlFrame, const CProtocol& HtrFrame);
  void animate() {};
  void keyHandler(uint8_t event);
};
