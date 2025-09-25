#ifndef __DISPLAY__
#define __DISPLAY__
#include <U8g2lib.h>
#include <CK_MAX.h>
#include "bsp.h"
#include "helpers.h"

//8 digit LED display
extern CK_MAX ledDisp;

//OLED displays
//New Haven Displays 256x64 is a good enough match for the displays on the dev board.  U8G2_R0 sets rotation
//to 0 degrees.  Boards are configured for four wire SPI
extern U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oledDisp1;
extern U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oledDisp2;
extern U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oledDisp3;
extern U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oledDisp4;

enum dispPos {
  dispTop,
  dispMiddle,
  dispBottom,
  dispLeft,
  dispRight,
  dispNA
};
typedef enum dispPos dispPos_t;

void displaySetup(void);
void displayUpdate(void);
void displayUpdateFast(void);

void displayGpsSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayGpsAvgSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDistanceMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDistRemainMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDeltaTimeMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDeltaSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayGPSInfo(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
#endif