#include <Arduino.h>
#include "keypad.h"
#include "pins.h"
/*
const unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void initKeyPad()
{
  pinMode(keyLeft_pin, INPUT);
  pinMode(keyRight_pin, INPUT);
  pinMode(keyCentre_pin, INPUT);
  pinMode(keyUp_pin, INPUT);
  pinMode(keyDown_pin, INPUT);
}

uint8_t readKeys()
{ 
  static uint8_t debouncedKey = 0;
  static unsigned long lastDebounceTime = 0;
  
  uint8_t newKey = 0;
  if(digitalRead(keyLeft_pin) == LOW)   newKey |= keyPress_Left;
  if(digitalRead(keyRight_pin) == LOW)  newKey |= keyPress_Right;
  if(digitalRead(keyCentre_pin) == LOW) newKey |= keyPress_Centre;
  if(digitalRead(keyUp_pin) == LOW)     newKey |= keyPress_Up;
  if(digitalRead(keyDown_pin) == LOW)   newKey |= keyPress_Down;

  static uint8_t prevKey = 0;
  if(newKey != prevKey) {
    lastDebounceTime = millis();
    prevKey = newKey;
  }

  unsigned long elapsed = millis() - lastDebounceTime;
  if (elapsed > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
//    Serial.println("debounce");

    // if the button state has changed:
    if (newKey != debouncedKey) {
      debouncedKey = newKey;
    }
  }

  return debouncedKey;
}
*/
CKeyPad::CKeyPad()
{
  // pin scanning
  _debouncedPins = 0;
  _prevPins = 0;
  _lastDebounceTime = millis();
  _debounceDelay = 50;
  // handler
  _lastKey = 0;
  _lastHoldTime = 0;
  _holdTimeout = 0;
  _keyCallback = NULL;
}

void
CKeyPad::init(int Lkey, int Rkey, int Ckey, int Ukey, int Dkey)
{
  _pins[0] = Lkey;
  _pins[1] = Rkey;
  _pins[2] = Ckey;
  _pins[3] = Ukey;
  _pins[4] = Dkey;
  for(int i=0; i<5; i++) 
    pinMode(_pins[i], INPUT);
}

uint8_t 
CKeyPad::scanPins()
{ 
  
  uint8_t newPins = 0;
  if(digitalRead(_pins[0]) == LOW)  newPins |= keyPress_Left;
  if(digitalRead(_pins[1]) == LOW)  newPins |= keyPress_Right;
  if(digitalRead(_pins[2]) == LOW)  newPins |= keyPress_Centre;
  if(digitalRead(_pins[3]) == LOW)  newPins |= keyPress_Up;
  if(digitalRead(_pins[4]) == LOW)  newPins |= keyPress_Down;

  if(newPins != _prevPins) {
    _lastDebounceTime = millis();
    _prevPins = newPins;
  }

  unsigned long elapsed = millis() - _lastDebounceTime;
  if (elapsed > _debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    _debouncedPins = newPins;
  }

  return _debouncedPins;
}

void
CKeyPad::setCallback(void (*callback)(uint8_t event))
{
  _keyCallback = callback;
}

uint8_t
CKeyPad::update()
{
  uint8_t newKey = scanPins();

  // determine edge events
  uint8_t keyChange = newKey ^ _lastKey;
  uint8_t Press = keyChange & newKey;     // bits set upon intial press, ONLY
  uint8_t Release = keyChange & ~newKey;  // bits set upon intial release, ONLY
  uint8_t Repeat = 0;  

  _lastKey = newKey;

  if(Press) {
#ifdef DBG_KEYPAD
    DebugPort.println("PRESS");
#endif
    _lastHoldTime = millis();
    _holdTimeout = 350;                 // initial hold delay
  }

  if(Release) {
#ifdef DBG_KEYPAD
    DebugPort.println("RELEASE");
#endif
    _holdTimeout = 0;                   // cancel repeat
  }

  if(_holdTimeout && ((millis() - _lastHoldTime) > _holdTimeout)) {
#ifdef DBG_KEYPAD
    DebugPort.println("REPEAT");
#endif
    _holdTimeout = 150;                 // repeat delay
    _lastHoldTime += _holdTimeout;
    Repeat = newKey;
  }

  if(Press) {
    if(_keyCallback != NULL) 
      _keyCallback(keyPressed | Press);
    return keyPressed | Press;
  }

  if(Release) {
    if(_keyCallback != NULL) 
      _keyCallback(keyReleased | Release);
    return (keyReleased | Release);
  }

  if(Repeat) {
    if(_keyCallback != NULL) 
      _keyCallback(keyRepeat | Repeat);
    return (keyRepeat | Repeat);
  }

  return 0;
}
