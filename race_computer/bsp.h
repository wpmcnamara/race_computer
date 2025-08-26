#ifndef __BSP__
#define __BSP__

#include <wiring.h>
#include <cstdint>

#ifdef __BREADBOARD__
//These are the pin assignments for the breadboard version
//pin assignments on the Teensy
#define LED_DISP_LOAD 14

#define OLED_DISP1_CS 10
#define OLED_DISP2_CS 8
#define OLED_DISP3_CS 7
#define OLED_DISP4_CS 6
#define OLED_DISP_DC 16
#define OLED_DISP1_RESET 22
#define OLED_DISP2_RESET 21
#define OLED_DISP3_RESET 20
#define OLED_DISP4_RESET 15

#define GPS_PPS 9
#define GPS_RESET 23
#define GPS_INT 4
#define GPS_CS 17

#define KEYPAD_INT 0
#define KEYPAD_START 5
//Neopixel line for the start/stop button
#define KEYPAD_LED ?

#define SDCARD_CS 1

#else
//these are the pin asignements for the prototype PCB
//pin assignments on the Teensy
#define LED_DISP_LOAD 22

#define OLED_DISP1_CS 10
#define OLED_DISP2_CS 8
#define OLED_DISP3_CS 6
#define OLED_DISP4_CS 4
#define OLED_DISP_DC 16
#define OLED_DISP1_RESET 9
#define OLED_DISP2_RESET 7
#define OLED_DISP3_RESET 5
#define OLED_DISP4_RESET 3

#define GPS_PPS 0
#define GPS_RESET 23
#define GPS_INT 20
#define GPS_CS 17

#define KEYPAD_INT 1
#define KEYPAD_START 14
//Neopixel line for the start/stop button
#define KEYPAD_LED 15

#define SDCARD_CS 2

#endif

#endif