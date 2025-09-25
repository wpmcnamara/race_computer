#ifndef __KEYPAD__
#define __KEYPAD__
#include "bsp.h"
#include "helpers.h"
#include "timer.h"
#include "Adafruit_NeoKey_1x4.h"
#include <Adafruit_NeoPixel.h>

#define KEY_START_STOP (1 << 4)
#define KEY_ENTER (1 << 0)
#define KEY_DOWN (1 << 1)
#define KEY_UP (1 << 2)
#define KEY_ESC (1 << 3)

//keypad
extern Adafruit_NeoKey_1x4 keypad;
extern volatile bool keyPress;
extern volatile bool startDebounce;
extern int debounceCount;

void keyPressInt(void);
void keypadSetup(void);
void startPressInt();
void readKeypad(void);
void keypadUpdate(void);
void startStopStopBreath(void);
void startStopStartBreath(void);
bool startStopIsBreathing(void);
void startStopStopBlink(void);
void startStopStartBlink(void);
bool startStopIsBlinking(void);
uint8_t getKeyPress(void);
#endif