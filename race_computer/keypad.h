#ifndef __KEYPAD__
#define __KEYPAD__
#include "bsp.h"
#include "helpers.h"
#include "timer.h"
#include "Adafruit_NeoKey_1x4.h"

//keypad
extern Adafruit_NeoKey_1x4 keypad;
extern volatile bool keyPress;
extern volatile bool startDebounce;
extern int debounceCount;

void keyPressInt(void);
void keypadSetup(void);
void startPressInt();
int readKeypad(void);
void keypadUpdate(void);
#endif