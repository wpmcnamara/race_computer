#ifndef __DISPLAY__
#define __DISPLAY__
#define U8G2_USE_DYNAMIC_ALLOC
#include <U8g2lib.h>
#include <CK_MAX.h>
#include "bsp.h"
#include "helpers.h"
#include <list>
#include "raceData.h"

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
  LegAvgSpeedLarge,
  LegDistanceLarge,
  LegDistRemainLarge,
  LegDeltaTimeLarge,
  LegDeltaSpeedLarge,
//  LegStats1,
//  LegStats2,  
  TurnPoints,
  RaceAvgSpeedLarge,
  RaceDistanceLarge,
  RaceDistRemainLarge,
  RaceDeltaTimeLarge,
  RaceDeltaSpeedLarge,
//  RaceStats1,
//  RaceStats2,
  GPSInfo,
  OLEDDispFuncMaxValue,
  OLEDDispFuncMinValue=GPSSpeedLarge
};

typedef enum OLEDRaceDisplayTypedef OLEDRraceDisplayTypedef_t;

enum LEDRaceDisplayTypedef {
  LegTime=0,
  LegDeltaSpeed,
  LegAverageSpeed,
  LegDistanceRemain,
  LegDeltaTime,    
  LegDistance,  
  GpsSpeed,
  RaceTime,
  RaceDeltaSpeed,
  RaceAverageSpeed,
  RaceDistanceRemain,
  RaceDistance,
  RaceDeltaTime,
  Dashes,
  LEDDispFuncMaxValue,
  LEDDispFuncMinValue=LegTime
};
typedef enum LEDRaceDisplayTypedef LEDRaceDisplayTypedef_t;

extern uint8_t menuItem;
extern raceDef_t *dispRace;
extern raceLegDef_t *dispRaceLeg;
extern bool raceSelectHighlight;
extern bool raceLegSelectHighlight;
extern void (*ledDispFunc)(void);
extern int OLEDDisplayActive[4];
extern int OLEDDisplaySelect[4];
extern int displaySelectLine;
extern bool displaySelectLineMode;
extern int LEDDisplayActive;
extern int LEDDisplaySelect;
extern const char *OLEDDisplayDescr[OLEDDispFuncMaxValue];
extern const char *LEDDisplayDescr[LEDDispFuncMaxValue];
extern uint8_t ledBrightness;
extern uint8_t oledBrightness;
extern uint8_t ledBrightnessTmp;
extern uint8_t oledBrightnessTmp;
extern event_t *displayUpdateEvent1;
extern event_t *displayUpdateEvent2;
extern event_t *displayUpdateEvent3;
extern event_t *displayUpdateEvent4;
extern event_t *displayUpdateFastEvent;
extern uint8_t displayTimeout;
extern uint8_t displayTimeoutTmp;
extern bool displayTimeOutEnable;


extern void (*OLEDDisplayFuncs[OLEDDispFuncMaxValue])(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
extern void (*LEDDisplayFuncs[LEDDispFuncMaxValue])(void);

struct displayContent {
  dispPos_t posX;
  dispPos_t posY;
  void (*display)(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
};

typedef struct displayContent displayContent_t;

class display {
  public:
    display(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &argScreen);
    void addContent(dispPos_t argPosX, dispPos_t argPosY, void (*argShowFunc)(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI&, dispPos_t, dispPos_t));
    void removeContent(displayContent_t *content);
    void clear(void);
    void show(void);
    void init(uint8_t brightness); //need to move default brightness to a #define in helpers.h or elsewhere.
    void setBrightness(uint8_t brightness);
    private:
    U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &screen;
    std::list<displayContent_t*> contentList; 

};

typedef class display display_t;

extern display_t oledDisplay1;
extern display_t oledDisplay2;
extern display_t oledDisplay3;    
extern display_t oledDisplay4;

void displaySetup(void);
extern "C" void displayUpdate(void);
void displayUpdate1(void);
void displayUpdate2(void);
void displayUpdate3(void);
void displayUpdate4(void);

void displayUpdateFast(void);

void clearDisplays(void);;

void displayGpsSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayLegAvgSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayLegDistanceMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayLegDistRemainMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayLegDeltaTimeMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayLegDeltaSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);

void displayGpsSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayLegAvgSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayLegDistanceLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayLegDistRemainLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayLegDeltaTimeLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayLegDeltaSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);

void displayRaceAvgSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayRaceDistanceLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayRaceDistRemainLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayRaceDeltaTimeLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayRaceDeltaSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);

void displayLegStats1(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY); 
void displayLegStats2(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY); 
void displayRaceStats1(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY); 
void displayRaceStats2(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);

void displayGPSInfo(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayRaceInfo(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayMenu(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayMenuTitle(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayDisplayConfig(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayLegSummaryActual(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayLegSummaryAdjusted(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayRaceSummary1(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayRaceSummary2(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);

void displayPoint(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);

void displayFirmwareConfirm(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayFirmwareUpdate(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displaySetLedBrightness(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displaySetOledBrightness(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displaySetDisplayTimeout(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displaySetLegTimeAdjust(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayAdjustLeg1(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displayAdjustLeg2(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displaySystemInfo(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);
void displaySetSpeedBandSource(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY);



void ledDispLegTime(void);
void ledDispLegDeltaSpeed(void);
void ledDispLegAverageSpeed(void);
void ledDispLegDistanceRemain(void);
void ledDispLegDeltaTime(void);
void ledDispLegDistance(void);
void ledDispGpsSpeed(void);

void ledDispRaceTime(void);
void ledDispRaceDeltaSpeed(void);
void ledDispRaceAverageSpeed(void);
void ledDispRaceDistanceRemain(void);
void ledDispRaceDistance(void);
void ledDispRaceDeltaTime(void);


void ledDispDashes(void);
void ledDispEights(void);
void ledDispBlank(void);
void ledDispStartCountdown(void);


void displayError(const char *err);


#endif