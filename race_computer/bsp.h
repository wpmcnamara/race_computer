#ifndef __BSP__
#define __BSP__

#include <wiring.h>
//#include <cstdint>
#include <stdint.h>

//these are the pin asignements for the prototype PCB
//pin assignments on the Teensy
#define LED_DISP_LOAD 22

#define OLED_DISP1_CS 10
#define OLED_DISP2_CS 7
#define OLED_DISP3_CS 5
#define OLED_DISP4_CS 3
#define OLED_DISP_DC 16
#define OLED_DISP1_RESET 8
#define OLED_DISP2_RESET 6
#define OLED_DISP3_RESET 4
#define OLED_DISP4_RESET 2

#define GPS_PPS 9
#define GPS_RESET 23
#define GPS_INT 20
#define GPS_CS 17

#define KEYPAD_INT 0
#define KEYPAD_START 15
//Neopixel line for the start/stop button
#define KEYPAD_LED 14

#define SDCARD_CS 1

extern bool SPILock;
extern "C" uint32_t firmwareSize;
extern "C" uint32_t firmwareProgress;
extern "C" uint8_t firmwareUpdateState;

void bsp_setup(void);
inline void doSPILock (void) {SPILock=true;}
inline void doSPIUnlock (void) {SPILock=false;}
bool checkForUpdate(void);
bool doFirmwareUpdate(void);
void doReboot(void);

#define SPI_SPEED 4000000



#endif