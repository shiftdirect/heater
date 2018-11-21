#ifndef __BTC_KEYPAD_H__
#define __BTC_KEYPAD_H__

#include "stdint.h"

void initKeyPad();
uint8_t readKeys();

const uint8_t keyPress_Left   = 0x01;
const uint8_t keyPress_Right  = 0x02;
const uint8_t keyPress_Centre = 0x04;
const uint8_t keyPress_Up     = 0x08;
const uint8_t keyPress_Down   = 0x10;

#endif