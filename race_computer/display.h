#ifndef __DISPLAY__
#define __DISPLAY__
#define U8G2_USE_DYNAMIC_ALLOC
#include <U8g2lib.h>
#include <CK_MAX.h>
#include "bsp.h"
#include "helpers.h"
#include <list>
#include "race.h"

typedef class displayContent displayContent_t;
typedef class screen screen_t;


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

enum OLEDRaceDisplayTypedef {
  GPSSpeedLarge=0,
  AvgSpeedLarge,
  DistanceLarge,
  DistRemainLarge,
  DeltaTimeLarge,
  DeltaSpeedLarge,
  TurnPoints,
  GPSInfo
};

typedef enum OLEDRaceDisplayTypedef OLEDRraceDisplayTypedef_t;

enum LEDRaceDisplayTypedef {
  LegTime=0,
  RaceTime,
  LegDeltaSpeed,
  RaceDeltaSpeed,
  Dashes
};
typedef enum LEDRaceDisplayTypedef LEDRaceDisplayTypedef_t;


extern std::list<displayContent_t*> displayList;
extern uint8_t menuItem;
extern race_t *dispRace;
extern raceLeg_t *dispRaceLeg;
extern bool raceSelectHighlight;
extern bool raceLegSelectHighlight;
extern void (*ledDispFunc)(void);
extern int OLEDDisplayActive[4];
extern int OLEDDisplaySelect[4];
extern int LEDDisplayActive;
extern int LEDDisplaySelect;
extern const char *OLEDDisplayDescr[8];
extern const char *LEDDisplayDescr[5];
extern event_t *displayUpdateEvent;
extern event_t *displayUpdateFastEvent;

class displayContent {
  public:
    displayContent(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &argScreen, 
      dispPos_t argPosX, 
      dispPos_t argPosY, 
      void (*argShowFunc)(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &, dispPos_t, dispPos_t));
    void display(void);
  private:
    dispPos_t posX;
    dispPos_t posY;
    U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &screen;
    void (*showFunc)(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
};

void displaySetup(void);
void displayUpdate(void);
void displayUpdateFast(void);

void displayGpsSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayAvgSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDistanceMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDistRemainMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDeltaTimeMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDeltaSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);

void displayGpsSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayAvgSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDistanceLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDistRemainLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDeltaTimeLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDeltaSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);

void displayGPSInfo(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayRaceInfo(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayMenu(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayMenuTitle(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDisplayConfigTitle(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDisplayConfig(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayRaceSelectTitle(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayRaceLegSelectTitle(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayLegSummaryActual(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayLegSummaryAdjusted(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayRaceSummary1(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayRaceSummary2(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);

void displayPoint(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);

void ledDispLegTime(void);
void ledDispRaceTime(void);
void ledDispLegDeltaSpeed(void);
void ledDispRaceDeltaSpeed(void);
void ledDispDashes(void);

void displayError(const char *err);


#endif