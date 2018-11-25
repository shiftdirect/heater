#include "stdint.h"

class C128x64_OLED;
class CProtocol;
class CScreenManager;

class CScreen1 : public CScreen
{
  bool _animatePump;
  bool _animateRPM;
  bool _animateGlow;
  int  _fanAnimationState;
  int  _dripAnimationState;
  int  _heatAnimationState;
  int  _keyRepeatCount;

  unsigned long _showTarget;

  void showRunState();
  void showThermometer(float desired, float actual);
  void showBodyThermometer(int actual);
  void showGlowPlug(int power);
  void showFan(int RPM);
  void showFuel(float rate);
  void showRunState(int state, int errstate);
public:
  CScreen1(C128x64_OLED& display, CScreenManager& mgr);
  void show(const CProtocol& CtlFrame, const CProtocol& HtrFrame);
  void animate();
  void keyHandler(uint8_t event);
};
