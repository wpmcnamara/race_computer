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

#define SDCARD_CS 28


#define HWVERSTRUCTVER 1
#define HWVERSTRUCTADDR 0

struct hardwareVersionStruct {
    uint8_t structVer;
    uint8_t pcbMajor;
    uint8_t pcbMinor;
    uint8_t pad;
    char pcbOther[16];
    uint32_t serialNo;
    char serialOther[16];
    uint32_t chkSum;
};

typedef struct hardwareVersionStruct hardwareVersionStruct_t;

extern bool SPILock;
extern "C" uint32_t firmwareSize;
extern "C" uint32_t firmwareProgress;
extern "C" uint8_t firmwareUpdateState;
extern hardwareVersionStruct_t hardwareVersion;
extern bool initHwVer;



void bsp_setup(void);
inline void doSPILock (void) {SPILock=true;}
inline void doSPIUnlock (void) {SPILock=false;}
bool checkForUpdate(void);
bool doFirmwareUpdate(void);
void doReboot(void);
uint8_t loadHardwareVersionStruct(void);
bool storeHardwareVersionStruct(hardwareVersionStruct_t* hwVerPtr);

#define SPI_SPEED 5000000




#endif