#ifndef __KEYPAD__
#define __KEYPAD__
#include "bsp.h"
#include "helpers.h"
#include "timer.h"
#include "Adafruit_NeoKey_1x4.h"
#include <Adafruit_NeoPixel.h>

#define KEYPAD_KEY_START_STOP (1 << 4)
#define KEYPAD_KEY_ENTER (1 << 0)
#define KEYPAD_KEY_DOWN (1 << 1)
#define KEYPAD_KEY_UP (1 << 2)
#define KEYPAD_KEY_ESC (1 << 3)

#define COLOR_RED 0xFF0000
#define COLOR_GREEN 0x00FF00
#define COLOR_BLUE 0x0000FF
#define COLOR_YELLOW 0xFFFF00
#define COLOR_CYAN 0x00FFFF
#define COLOR_PURPLE 0xFF00FF
#define COLOR_WHITE 0xFFFFFF
#define COLOR_BLACK 0x000000

//keypad
extern Adafruit_NeoKey_1x4 keypad;
extern volatile bool keyPress;
extern volatile bool startDebounce;
extern int debounceCount;
extern bool keysLocked;
extern bool startStopStartsRace;
extern int speedBandSource;

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
void startStopOn(uint32_t color);
void startStopOff(void);
void keypadBreath(void);
void keypadStopBreath(void);
void keypadStartBreath(void);
uint8_t getKeyPress(void);
void setAllButtonColor(uint32_t color);
void setButtonColor(uint8_t button, uint32_t color);
#endif