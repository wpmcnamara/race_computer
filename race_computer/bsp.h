#ifndef __BSP__
#define __BSP__

#include <wiring.h>
#include <cstdint>

//these are the pin asignements for the prototype PCB
//pin assignments on the Teensy
#define LED_DISP_LOAD 22

#define OLED_DISP1_CS 10
#define OLED_DISP2_CS 8
#define OLED_DISP3_CS 6
#define OLED_DISP4_CS 4
#define OLED_DISP_DC 16
#define OLED_DISP1_RESET 0
#define OLED_DISP2_RESET 7
#define OLED_DISP3_RESET 5
#define OLED_DISP4_RESET 3

#define GPS_PPS 9
#define GPS_RESET 23
#define GPS_INT 20
#define GPS_CS 17

#define KEYPAD_INT 1
#define KEYPAD_START 14
//Neopixel line for the start/stop button
#define KEYPAD_LED 15

//rev 2 of the proto board swaps the START and LED pins
//this is for routing and so isn't in the manual reworks on the
//rev 1 boards
//#define KEYPAD_START 15
//#define KEYPAD_LED 14

#define SDCARD_CS 2

#endif