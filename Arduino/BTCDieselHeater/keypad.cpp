#include <Arduino.h>
#include "keypad.h"
#include "pins.h"

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