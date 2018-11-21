#ifndef __BTC_KEYPAD_H__
#define __BTC_KEYPAD_H__

#include "stdint.h"

//void initKeyPad();
//uint8_t readKeys();

const uint8_t keyPress_Left   = 0x01;
const uint8_t keyPress_Right  = 0x02;
const uint8_t keyPress_Centre = 0x04;
const uint8_t keyPress_Up     = 0x08;
const uint8_t keyPress_Down   = 0x10;
const uint8_t keyPressed      = 0x20;   // action flag
const uint8_t keyReleased     = 0x40;   // action flag
const uint8_t keyRepeat       = 0x80;   // action flag

class CKeyPad {
private:
  void (*_keyCallback)(uint8_t event);
  uint8_t _pins[5];  //0:L, 1:R, 2:C, 3:U, 4:D
  // pin scanning usage
  uint8_t _prevPins;
  uint8_t _debouncedPins;
  // handler usage
  uint8_t _lastKey;
  unsigned long _lastHoldTime;
  unsigned long _holdTimeout;
  uint8_t scanPins();
  unsigned long _lastDebounceTime;
  unsigned long _debounceDelay;
public:
  CKeyPad();
  void init(int Lkey, int Rkey, int Ckey, int Ukey, int Dkey);
  uint8_t update();
	void setCallback(void (*Callback)(uint8_t event));
};

#endif