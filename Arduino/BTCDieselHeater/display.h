#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <Arduino.h>


class CProtocol;
class C128x64_OLED;
class CScreen;

class CScreenManager {
  static const int _maxScreens = 4;
  CScreen* _pScreen[_maxScreens];
  CScreen* _pActiveScreen;
  C128x64_OLED* _pDisplay;
  int _currentScreen;
  void _switchScreen();
public:
  CScreenManager();
  ~CScreenManager();
  void init();
  void update(const CProtocol& CtlFrame, const CProtocol& HtrFrame);
  void animate();
  void nextScreen();
  void prevScreen();
  void keyHandler(uint8_t event);
};

class CScreen {
protected:
  C128x64_OLED& _display;
  CScreenManager& _Manager;
  void showBTicon();
  void showWifiIcon();
  void showBatteryIcon(float voltage);
public:
  CScreen(C128x64_OLED& disp, CScreenManager& mgr); 
  virtual ~CScreen(); 
  virtual void animate();
  virtual void show(const CProtocol& CtlFrame, const CProtocol& HtrFrame);
  virtual void keyHandler(uint8_t event) {};
};

#endif // __DISPLAY_H__
