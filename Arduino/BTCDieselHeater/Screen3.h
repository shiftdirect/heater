#include "stdint.h"

class C128x64_OLED;
class CScreenManager;
class CProtocol;

class CScreen3 : public CScreen {
  unsigned long _PrimeStop;
  unsigned long _PrimeCheck;
  int _rowSel;
  int _colSel;
  void stopPump();
public:
  CScreen3(C128x64_OLED& display, CScreenManager& mgr);
  void show(const CProtocol& CtlFrame, const CProtocol& HtrFrame);
  void animate() {};
  void keyHandler(uint8_t event);
};
