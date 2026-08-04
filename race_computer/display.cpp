#include "display.h"
#include "timer.h"
#include "gps.h"
#include "agr_logo_bottom.h"
#include "agr_logo_top.h"
#include "event.h"
#include "raceData.h"
#include "menu.h"
#include "bsp.h"
#include "keypad.h"
#include "time_date.h"

//8 digit LED display
CK_MAX ledDisp(LED_DISP_LOAD);


// US Central Time Zone (Chicago, Houston)
//TimeChangeRule usCDT = {"CDT", Second, Sun, Mar, 2, -300};
//TimeChangeRule usCST = {"CST", First, Sun, Nov, 2, -360};
//Timezone usCT(usCDT, usCST);


#define ROTATION U8G2_R0

//OLED displays
//New Haven Displays 256x64 is a good enough match for the displays on the dev board.  U8G2_R0 sets rotation
//to 0 degrees.  Boards are configured for four wire SPI
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oledDisp1(ROTATION, OLED_DISP1_CS, OLED_DISP_DC, OLED_DISP1_RESET);
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oledDisp2(ROTATION, OLED_DISP2_CS, OLED_DISP_DC, OLED_DISP2_RESET);
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oledDisp3(ROTATION, OLED_DISP3_CS, OLED_DISP_DC, OLED_DISP3_RESET);
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oledDisp4(ROTATION, OLED_DISP4_CS, OLED_DISP_DC, OLED_DISP4_RESET);

display_t oledDisplay1(oledDisp1);
display_t oledDisplay2(oledDisp2);
display_t oledDisplay3(oledDisp3);
display_t oledDisplay4(oledDisp4);

char buffer[256];

struct gpsDataStruct *gpsDataPtr;
orcTime_t *gpsTimePtr;
uint8_t menuItem=0;
raceDef_t *dispRace;
raceLegDef_t *dispRaceLeg;
bool raceSelectHighlight=false;
bool raceLegSelectHighlight=false;
//event_t *displayUpdateEvent;// = NULL;
event_t *displayUpdateEvent1;
event_t *displayUpdateEvent2;
event_t *displayUpdateEvent3;
event_t *displayUpdateEvent4;
event_t *displayUpdateFastEvent;

void (*ledDispFunc)(void)=NULL;

uint8_t ledBrightness;
uint8_t oledBrightness=125;
uint8_t ledBrightnessTmp;
uint8_t oledBrightnessTmp;
uint8_t displayTimeout;
uint8_t displayTimeoutTmp;
bool displayTimeOutEnable=false;

int OLEDDisplayActive[4]={GPSSpeedLarge,
                                LegDeltaTimeLarge,
                                LegDistRemainLarge,
                                TurnPoints};
int OLEDDisplaySelect[4]={GPSSpeedLarge,
                                LegDeltaTimeLarge,
                                LegDistRemainLarge,
                                TurnPoints};
int LEDDisplayActive=LegDeltaSpeed;
int LEDDisplaySelect=LegDeltaSpeed;
const char *OLEDDisplayDescr[OLEDDispFuncMaxValue]={
  "Current GPS Speed",
  "Leg - Average Speed",
  "Leg - Distance Traveled",
  "Leg - Distance Remaining",
  "Leg - Time Delta",
  "Leg - Speed Delta",
//  "Leg - Stats 1",
//  "Leg - Stats 2",
  "Leg - Turns",  
  "Race - Average Speed",
  "Race - Distance Traveled",
  "Race - Distance Remaining",
  "Race - Time Delta",
  "Race - Speed Delta",  
//  "Race -- Stats 1",
//  "Race -- Stats 2",
  "GPS Info"
};
const char *LEDDisplayDescr[LEDDispFuncMaxValue]={
  "Leg - Time",
  "Leg - Speed Delta",
  "Leg - Average Speed",
  "Leg - Distance Remaining",
  "Leg - Time Delta",
  "Leg - Distance Traveled",
  "GPS Speed",
  "Race - Time",
  "Race - Speed Delta",
  "Race - Average Speed",
  "Race - Distance Remaining",
  "Race - Distance Traveled",
  "Race - Time Delta",
  "Dashes"
};

void (*OLEDDisplayFuncs[OLEDDispFuncMaxValue])(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) = {
  displayGpsSpeedLarge,
  displayLegAvgSpeedLarge,
  displayLegDistanceLarge,
  displayLegDistRemainLarge,
  displayLegDeltaTimeLarge,
  displayLegDeltaSpeedLarge,
//  displayLegStats1,
//  displayLegStats2,
  displayPoint,  
  displayRaceAvgSpeedLarge,
  displayRaceDistanceLarge,
  displayRaceDistRemainLarge,
  displayRaceDeltaTimeLarge,
  displayRaceDeltaSpeedLarge,  
//  displayRaceStats1,
//  displayRaceStats2,
  displayGPSInfo
};

void (*LEDDisplayFuncs[LEDDispFuncMaxValue])(void)= {
  ledDispLegTime,
  ledDispLegDeltaSpeed,
  ledDispLegAverageSpeed,
  ledDispLegDistanceRemain,
  ledDispLegDeltaTime,
  ledDispLegDistance,
  ledDispGpsSpeed,
  ledDispRaceTime,
  ledDispRaceDeltaSpeed,
  ledDispRaceAverageSpeed,
  ledDispRaceDistanceRemain,
  ledDispRaceDistance,
  ledDispRaceDeltaTime,
  ledDispDashes  
};

int displaySelectLine=0;
bool displaySelectLineMode=false;


std::list<displayContent_t*> displayList;

display::display(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &argScreen) : screen(argScreen) {};

void display::init(uint8_t brightness=125) {
  screen.setBufferPtr(new uint8_t[screen.getBufferSize()]);
  screen.initDisplay();
  //screen.clearDisplay();
  screen.setPowerSave(0);
  setBrightness(brightness);
  screen.clearBuffer();
  screen.drawBox(0,0,256,64);
  screen.sendBuffer();
}

void display::setBrightness(uint8_t brightness) {
  screen.setContrast(brightness);
}

void display::addContent(dispPos_t argPosX, dispPos_t argPosY, void (*argShowFunc)(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI&, dispPos_t, dispPos_t)) {
  displayContent_t *newContent=new displayContent_t{argPosX, argPosY,argShowFunc};
  //newContent->posX=argPosX;
  //newContent->posY=argPosY;
  //newContent->display=argShowFunc;
  contentList.push_back(newContent);
}

void display::removeContent(displayContent_t *content) {
  //displayContent_t *contentPtr;
  contentList.remove(content);
  delete content;
}

void display::clear(void) {
  displayContent_t *content;
  for (std::list<displayContent_t *>::iterator it=contentList.begin(); it != contentList.end(); ++it) {
    //We could get interuptted in the middle of clean up, so we save the pointer and null the iterator 
    //reference before we delete the object.  If we are interrupted and show() is executed, the NULL 
    //check will prevent a crash.
    content=*it;
    *it=NULL;
    delete content;
  }
  //now that all the entries are null, we can clear the list.
  contentList.clear();
}

void display::show(void) {
  //if something else is using the SPI bus, we don't want to try to use it.  We'll just skip this update and try again next time.
  if(SPILock) {
    return;
  }
  screen.clearBuffer();
  for (std::list<displayContent_t *>::iterator it=contentList.begin(); it != contentList.end(); ++it) {
    if (*it != NULL) {
      ((*it)->display)(screen, (*it)->posX, (*it)->posY);
    }
  }
  screen.sendBuffer();
}

void displaySetup(void) {
  int x;
  displayTimeout=30;
  dispRace=NULL;
  dispRaceLeg=NULL;
  //we can't do display initalization during object construction because the SPI bus is not yet initialized.  
  //So we have to do it here in setup.
  oledDisplay1.init();
  oledDisplay2.init();
  oledDisplay3.init();
  oledDisplay4.init();
  
  ledDisp.begin();
  ledDisp.Set_Brightness(3);
  ledBrightness=3;
  // Turn Off the Auto Refresh
  ledDisp.AutoRefresh(false);
  ledDisp.RefreshMe();
  ledDisp.Set_Position(0);
  ledDisp.ShowMe("88888888");

  delay(1000);
  ledDisp.ShowMe("--------");
  ledDisp.Set_Position(0);

  //This following chunk needs to be re-architected into display functions and display content objects.  
  // For now, it is a quick and dirty way to get the splash screen up on the displays.
  oledDisp1.clearBuffer();
  oledDisp1.drawXBM(18,2,agr_top_width, agr_top_height, agr_logo_top);
  oledDisp1.sendBuffer();
  oledDisp2.clearBuffer();
  oledDisp2.drawXBM(18,0,agr_bottom_width, agr_bottom_height, agr_logo_bottom);
  oledDisp2.sendBuffer();
  oledDisp3.clearBuffer();
  oledDisp3.setFont(u8g2_font_spleen16x32_mf);	
  oledDisp3.drawStr(16,20,"Open Road Race");	
  oledDisp3.drawStr(63,52,"Computer");	
  oledDisp3.sendBuffer();
  oledDisp4.clearBuffer();
  oledDisp4.setFont(u8g2_font_spleen12x24_mf);
  sprintf(buffer, "\xA9 Patrick McNamara");
  x=128-(oledDisp4.getStrWidth(buffer)/2);
  oledDisp4.drawStr(x,20,buffer);	
  sprintf(buffer, "%s", VERSION_STRING);
  x=128-(oledDisp4.getStrWidth(buffer)/2);
  oledDisp4.drawStr(x,52,buffer);	
  oledDisp4.sendBuffer();
  delay(5000);
  oledDisp1.clearBuffer();
  oledDisp2.clearBuffer();
  oledDisp3.clearBuffer();
  oledDisp4.clearBuffer();
  oledDisp1.sendBuffer();
  oledDisp2.sendBuffer();
  oledDisp3.sendBuffer();
  oledDisp4.sendBuffer();
  displayUpdateFastEvent=new event_t(displayUpdateFast, eventRepeat, true, false, 0, 1, &Serial, "displayUpdateFast");
  //displayUpdateEvent=new event_t(displayUpdate, eventRepeat, true, false, 0, 10, &Serial, "displayUpdate");
  //Since we update each display independently and we don't want them to occur all at the same event tick, we add a
  //stagger() to each one to offset them by one event tick.
  displayUpdateEvent1=new event_t(displayUpdate1, eventRepeat, true, false, 0, 10, &Serial, "displayUpdate1");
  displayUpdateEvent1->stagger(1);
  displayUpdateEvent2=new event_t(displayUpdate2, eventRepeat, true, false, 0, 10, &Serial, "displayUpdate2");
  displayUpdateEvent2->stagger(2);
  displayUpdateEvent3=new event_t(displayUpdate3, eventRepeat, true, false, 0, 10, &Serial, "displayUpdate3");
  displayUpdateEvent3->stagger(3);
  displayUpdateEvent4=new event_t(displayUpdate4, eventRepeat, true, false, 0, 10, &Serial, "displayUpdate4");
  displayUpdateEvent4->stagger(4);
  gpsDataPtr=getGpsData();
  gpsTimePtr=getGpsTime();
}

void clearDisplays(void) {
  oledDisplay1.clear();
  oledDisplay2.clear();
  oledDisplay3.clear();
  oledDisplay4.clear();
}

extern "C" void displayUpdate(void) {
  oledDisplay1.show();
  oledDisplay2.show();
  oledDisplay3.show();
  oledDisplay4.show();  
}

void displayUpdate1(void) {
  oledDisplay1.show();
}

void displayUpdate2(void) {
  oledDisplay2.show();
}

void displayUpdate3(void) {
  oledDisplay3.show();
}

void displayUpdate4(void) {
  oledDisplay4.show();
}




void displayGPSInfo(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  time_t utc = now();
  TimeChangeRule *tcr;
  time_t t = timeDateSettings.getTz()->toLocal(utc, &tcr);

  display.setFont(u8g2_font_spleen16x32_mf);   
  sprintf(buffer, "%02d:%02d:%02d", hour(t), minute(t), second(t));
  display.drawStr(64,20,buffer);    
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(194,20,tcr->abbrev);
  sprintf(buffer, "lattitude: %f\xB0", gpsDataPtr->lat);
  display.drawStr(0,32,buffer);	
  sprintf(buffer, "longitude: %f\xB0", gpsDataPtr->lon);
  display.drawStr(0,44,buffer);	
  sprintf(buffer, "satellites: %2d", gpsDataPtr->siv);
  display.drawStr(0,56,buffer);	
  switch(gpsDataPtr->fix) {
    case 1:
      display.drawStr(128,56, "fix: DR");
      break;
    case 2:
      display.drawStr(128,56, "fix: 2D");
      break;
    case 3:
      display.drawStr(128,56, "fix: 3D");
      break;
    case 4:
      display.drawStr(128,56, "fix: GNSS+DR");
      break;
    case 5:
      display.drawStr(128,56, "fix: Time");
      break;
    default:
      display.drawStr(128,56, "fix: None");
      break;
  }
}

void displayLegDeltaTimeMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  unsigned int y=0;
  int32_t timeDelta=round(race.legTimeDelta(milliseconds));
  char sign;
  switch (posY) {
    case dispTop:
      y=26;
      break;
    case dispMiddle:
      y=30;
      break;
    case dispBottom:
      y=58;
      break;
    case dispLeft:
    case dispRight:
    case dispNA:
      return;
      break;
  }
  display.setFont(u8g2_font_spleen8x16_mf);
  display.drawStr(0,y,"   time  :");
  display.drawLine(61,y,66,y-11);
  display.drawLine(62,y-1,66,y-10); 

  display.drawLine(66,y-11,71,y);
  display.drawLine(66,y-10,70,y-1);
 
  display.drawLine(61,y,71,y);
  display.drawLine(62,y-1,70,y-1);
  display.setFont(u8g2_font_spleen16x32_mf);	

  if(timeDelta==0) {
    sign=' ';
  } else if (timeDelta<0) {
    sign='-';
  } else if (timeDelta>0) {
    sign='+';
  }
  sprintf(buffer, "%c%3ld.%03ld", sign, abs(timeDelta/1000), abs(timeDelta%1000) );
  display.drawStr(81,y, buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(225,y,"sec");	
}

void displayLegDeltaSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  unsigned int y=0;
  switch (posY) {
    case dispTop:
      y=26;
      break;
    case dispMiddle:
      y=30;
      break;
    case dispBottom:
      y=58;
      break;
    case dispLeft:
    case dispRight:
    case dispNA:
      return;
      break;
  }
  display.setFont(u8g2_font_spleen8x16_mf);
  display.drawStr(0,y,"  speed  :");
  display.drawLine(61,y,66,y-11);
  display.drawLine(62,y-1,66,y-10); 

  display.drawLine(66,y-11,71,y);
  display.drawLine(66,y-10,70,y-1);
 
  display.drawLine(61,y,71,y);
  display.drawLine(62,y-1,70,y-1);
  display.setFont(u8g2_font_spleen16x32_mf);	

  sprintf(buffer, "%8.3f",race.legSpeedDelta(imperial));
  display.drawStr(97,y,buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(225,y,"mph");  
}

void displayLegDistanceMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  unsigned int y=0;
  switch (posY) {
    case dispTop:
      y=26;
      break;
    case dispMiddle:
      y=30;
      break;
    case dispBottom:
      y=58;
      break;
    case dispLeft:
    case dispRight:
    case dispNA:
      return;
      break;
  }
  display.setFont(u8g2_font_spleen8x16_mf);	 
  display.drawStr(0,y," distance:");	
  display.setFont(u8g2_font_spleen16x32_mf);	
  sprintf(buffer, "%8.3f", race.legDistanceComplete(imperial));
  display.drawStr(97,y,buffer);	 
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(225,y,"miles");	
}

void displayLegDistRemainMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  unsigned int y=0;
  switch (posY) {
    case dispTop:
      y=26;
      break;
    case dispMiddle:
      y=30;
      break;
    case dispBottom:
      y=58;
      break;
    case dispLeft:
    case dispRight:
    case dispNA:
      return;
      break;
  }
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(0,y," dist rem:");	
  display.setFont(u8g2_font_spleen16x32_mf);	
  sprintf(buffer, "%8.3f", race.legDistanceRemaining(imperial));
  display.drawStr(97,y,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(225,y,"miles");	
}

void displayGpsSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  unsigned int y=0;
  switch (posY) {
    case dispTop:
      y=26;
      break;
    case dispMiddle:
      y=30;
      break;
    case dispBottom:
      y=58;
      break;
    case dispLeft:
    case dispRight:
    case dispNA:
      return;
      break;
  }
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(0,y,"    speed:");	 
  display.setFont(u8g2_font_spleen16x32_mf);	
  sprintf(buffer, "%8.3f", SPEED_INTERNAL_TO_MPH(gpsDataPtr->speed));
  display.drawStr(97,y,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(225,y,"mph");	
}

void displayLegAvgSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  unsigned int y=0;
  switch (posY) {
    case dispTop:
      y=26;
      break;
    case dispMiddle:
      y=30;
      break;
    case dispBottom:
      y=58;
      break;
    case dispLeft:
    case dispRight:
    case dispNA:
      return;
      break;      
  }
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(0,y,"  speed \xd8:");		
  sprintf(buffer, "%8.3f", race.legAverageSpeed(imperial));
  display.drawStr(97,y,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(225,y,"mph");	
}

void displayGpsSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(0,41,"speed");	
  display.setFont(u8g2_font_logisoso50_tn);
  sprintf(buffer, "%7.3f", SPEED_INTERNAL_TO_MPH(gpsDataPtr->speed));
  display.drawStr(42,61,buffer);	
  display.setFont(u8g2_font_spleen12x24_mf);	
  display.drawStr(0,61,"mph");	  
}

void displayLegAvgSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(11,13,"leg");
  display.setFont(u8g2_font_spleen16x32_mf);
  display.drawStr(6,41,"\xd8S");	
  display.setFont(u8g2_font_logisoso50_tn);
  sprintf(buffer, "%7.3f", race.legAverageSpeed(imperial));
  display.drawStr(42,61,buffer);	
  display.setFont(u8g2_font_spleen12x24_mf);	
  display.drawStr(0,61,"mph");
}

void displayLegDistanceLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(11,14,"leg");
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(0,41," dist");
  display.drawStr(0,61,"miles");	
  display.setFont(u8g2_font_logisoso50_tn);
  sprintf(buffer, "%7.3f", race.legDistanceComplete(imperial));
  display.drawStr(42,61,buffer);	
}

void displayLegDistRemainLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(11,13,"leg");
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(0,29," dist");
  display.drawStr(0,45,"  rem");
  display.drawStr(0,61,"miles");	
  display.setFont(u8g2_font_logisoso50_tn);
  sprintf(buffer, "%7.3f", race.legDistanceRemaining(imperial));
  display.drawStr(42,61,buffer);	
}

void displayLegDeltaTimeLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  char sign;
  int32_t timeDelta=(int32_t)round(race.legTimeDelta(milliseconds));
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(11,13,"leg");  
  display.setFont(u8g2_font_spleen16x32_mf);
  display.drawStr(22,41,"T");	
  display.drawTriangle(4,41, 12,22, 21,41);
  display.setDrawColor(0);
  display.drawTriangle(7,38, 12,27, 17,38);
  display.setDrawColor(1);
  display.setFont(u8g2_font_logisoso50_tn);
  if(timeDelta==0) {
    sign=' ';
  } else if (timeDelta<0) {
    sign='-';
  } else if (timeDelta>0) {
    sign='+';
  }  
  sprintf(buffer, "%c%3ld.%03ld", sign, abs(timeDelta/1000), abs(timeDelta%1000));
  display.drawStr(42,61,buffer);	
  display.setFont(u8g2_font_spleen12x24_mf);	
  display.drawStr(0,61,"sec");
}

void displayLegDeltaSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(11,13,"leg");
  display.setFont(u8g2_font_spleen16x32_mf);
  display.drawStr(22,41,"S");	
  display.drawTriangle(4,41, 12,22, 21,41);
  display.setDrawColor(0);
  display.drawTriangle(7,38, 12,27, 17,38);
  display.setDrawColor(1);
  display.setFont(u8g2_font_logisoso50_tn);
  sprintf(buffer, "%7.3f", race.legSpeedDelta(imperial));
  display.drawStr(42,61,buffer);	
  display.setFont(u8g2_font_spleen12x24_mf);	
  display.drawStr(0,61,"mph");
}

void displayRaceAvgSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(8,13,"race");
  display.setFont(u8g2_font_spleen16x32_mf);
  display.drawStr(6,41,"\xd8S");	
  display.setFont(u8g2_font_logisoso50_tn);
  sprintf(buffer, "%7.3f", race.raceAverageSpeed(imperial));
  display.drawStr(42,61,buffer);	
  display.setFont(u8g2_font_spleen12x24_mf);	
  display.drawStr(0,61,"mph");
}

void displayRaceDistanceLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(8,14,"race");
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(0,41," dist");
  display.drawStr(0,61,"miles");	
  display.setFont(u8g2_font_logisoso50_tn);
  sprintf(buffer, "%7.3f", race.raceActualDistanceComplete(imperial));
  display.drawStr(42,61,buffer);	
}

void displayRaceDistRemainLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(8,13,"race");
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(0,29," dist");
  display.drawStr(0,45,"  rem");
  display.drawStr(0,61,"miles");	
  display.setFont(u8g2_font_logisoso50_tn);
  sprintf(buffer, "%7.3f", race.raceDistanceRemaining(imperial));
  display.drawStr(42,61,buffer);	
}

void displayRaceDeltaTimeLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  char sign;
  int32_t timeDelta=(int32_t)round(race.raceTimeDelta(milliseconds));
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(8,13,"race");  
  display.setFont(u8g2_font_spleen16x32_mf);
  display.drawStr(22,41,"T");	
  display.drawTriangle(4,41, 12,22, 21,41);
  display.setDrawColor(0);
  display.drawTriangle(7,38, 12,27, 17,38);
  display.setDrawColor(1);
  display.setFont(u8g2_font_logisoso50_tn);
  if(timeDelta==0) {
    sign=' ';
  } else if (timeDelta<0) {
    sign='-';
  } else if (timeDelta>0) {
    sign='+';
  }  
  sprintf(buffer, "%c%3ld.%03ld", sign, abs(timeDelta/1000), abs(timeDelta%1000));
  display.drawStr(42,61,buffer);	
  display.setFont(u8g2_font_spleen12x24_mf);	
  display.drawStr(0,61,"sec");
}

void displayRaceDeltaSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(8,13,"race");
  display.setFont(u8g2_font_spleen16x32_mf);
  display.drawStr(22,41,"S");	
  display.drawTriangle(4,41, 12,22, 21,41);
  display.setDrawColor(0);
  display.drawTriangle(7,38, 12,27, 17,38);
  display.setDrawColor(1);
  display.setFont(u8g2_font_logisoso50_tn);
  sprintf(buffer, "%7.3f", race.raceSpeedDelta(imperial));
  display.drawStr(42,61,buffer);	
  display.setFont(u8g2_font_spleen12x24_mf);	
  display.drawStr(0,61,"mph");
}

void displayLegAvgSpeedSmall(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  int x=0;
  int y=0;
  switch (posY) {
    case dispTop:
      y=12;
      break;
    case dispMiddle:
      y=32;
      break;
    case dispBottom:
      y=44;
      break;
    case dispLeft:
    case dispRight:
    case dispNA:
      return;
      break;      
  } 
  switch (posX) {
    case dispTop:
      x=0;
      break;
    case dispMiddle:
      x=67;
      break;
    case dispBottom:
      x=128;
      break;
    case dispLeft:
    case dispRight:
    case dispNA:
      return;
      break;      
  } 
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(x,y," \xd8S:");	
  display.setFont(u8g2_font_spleen8x16_mf);	
  sprintf(buffer, "%8.3f", race.legAverageSpeed(imperial));
  display.drawStr(x+36,y,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(x+100,y,"mph");	
}

void displayLegStats1(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  sprintf(buffer, "aT:%4.03f atT:%4.03f dT:%4.03f", 
    race.legTime(seconds),
    race.legTargetTime(seconds),
    race.legTimeDelta(seconds));
  display.drawStr(1,11,buffer);  
  sprintf(buffer, "aS:%3.03f atS:%3.03f dS:%3.03f",
    race.legAverageSpeed(imperial),
    race.legAdjustedTargetSpeed(imperial),
    race.legSpeedDelta(imperial)
  );
  display.drawStr(1,24,buffer); 
  sprintf(buffer, "aD:%3.03f dD:%3.03f rD:%3.03f",
    race.legDistanceComplete(imperial),
    DISTANCE_INTERNAL_TO_MILES(race.activeLeg->driveDistance),
    race.legDistanceRemaining(imperial)
  );
  display.drawStr(1,37,buffer); 
  sprintf(buffer, "pT:%4.03f patT:%4.03f aT:%3.03f",
    race.activeLeg->targetTime,
    race.legTargetTime(seconds),
    race.legTargetTime(seconds)-race.activeLeg->targetTime
  );
  display.drawStr(1,50,buffer);
  sprintf(buffer, "pS:%3.03f dS:%3.03f atS:%3.03f",
    race.activeLeg->speed,
    race.activeLeg->driveSpeed,
    race.legAdjustedTargetSpeed(imperial)
  );
  display.drawStr(1,63,buffer);
}

void displayLegStats2(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  sprintf(buffer, "tD:%4.03f dD:%4.03f cD:%4.03f", 
    race.activeLeg->distance,
    race.activeLeg->driveDistance,
    race.legDistanceComplete(imperial));
  display.drawStr(1,11,buffer);  
  sprintf(buffer, "cD:%3.03f dR:%3.03f",
    race.legDistanceComplete(imperial),
    race.legDistanceRemaining(imperial)
  );
  display.drawStr(1,24,buffer); 
}

void displayRaceStats1(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  sprintf(buffer, "tT:%4.03f aT:%4.03f dT:%4.03f", 
    TIME_INTERNAL_TO_SECONDS(race.activeLeg->raceLegEndTargetTime),
    race.raceTime(seconds),
    race.raceTimeDelta(seconds));
  display.drawStr(1,11,buffer);  
  sprintf(buffer, "tdS:%3.03f aS:%3.03f dS:%3.03f",
    SPEED_INTERNAL_TO_MPH(race.activeLeg->raceLegEndDriveAvgSpeed),
    race.raceAverageSpeed(imperial),
    race.raceLegEndSpeedDelta(imperial)
  );
  display.drawStr(1,24,buffer); 
  sprintf(buffer, "rcD:%3.03f ledD:%3.03f lerD:%3.03f",
    race.raceDistanceComplete(imperial),
    DISTANCE_INTERNAL_TO_MILES(race.activeLeg->raceLegEndDriveDistance),
    race.raceLegEndDistanceRemaining(imperial)
  );
  display.drawStr(1,37,buffer); 
  sprintf(buffer, "pT:%4.03f ctT:%4.03f rdT:%3.03f",
    TIME_INTERNAL_TO_SECONDS(race.activeLeg->raceLegEndTargetTime),
    race.raceTargetTimeComplete(seconds),
    race.raceTimeDelta(seconds)
  );
  display.drawStr(1,50,buffer);
  sprintf(buffer, "pS:%3.03f dS:%3.03f",
    race.activeRace->speed(imperial),
    race.activeRace->driveSpeed(imperial)
  );
  display.drawStr(1,63,buffer);
}

void displayRaceStats2(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  sprintf(buffer, "tD:%4.03f dD:%4.03f tcD:%4.03f", 
    race.activeRace->distance(imperial),
    race.activeRace->driveDistance(imperial),
    race.raceTargetDistanceComplete(imperial)
  );
  display.drawStr(1,11,buffer);  
  sprintf(buffer, "cdD:%3.03f acD:%3.03f dR:%3.03f",
    race.raceDriveDistanceComplete(imperial),
    race.raceActualDistanceComplete(imperial),
    race.raceDistanceRemaining(imperial)
  );
  display.drawStr(1,24,buffer); 
  sprintf(buffer, "tT:%4.03f cT:%4.03f",
    race.activeRace->targetTime(seconds),
    race.raceTimeComplete(seconds)
  );
  display.drawStr(1,50,buffer);  
}


void displayRaceInfo(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(0,12,"Race Selected");
  if(dispRace!=NULL) {
    sprintf(buffer, "race: %s", dispRace->descr.c_str());
    if(raceSelectHighlight) {
      display.drawButtonUTF8(1, 24, U8G2_BTN_INV|U8G2_BTN_BW1, 255,  0,  0, buffer );
    } else {
      display.drawButtonUTF8(1, 24, U8G2_BTN_BW0, 255,  0,  0, buffer );
    }
    sprintf(buffer, "dist: %8.3f  spd: %7.3f  mark: %0.0fs", dispRace->distance(imperial), dispRace->speed(imperial), dispRace->mark(seconds));
    display.drawStr(1,36,buffer);
    sprintf(buffer, "leg: %s", dispRaceLeg->descr.c_str());
    if(raceLegSelectHighlight) {
      display.drawButtonUTF8(1, 48, U8G2_BTN_INV|U8G2_BTN_BW1, 255,  0,  0, buffer );
    } else {
      display.drawButtonUTF8(1, 48, U8G2_BTN_BW0, 255,  0,  0, buffer );
    }
    sprintf(buffer, "dist: %8.3f  spd: %7.3f  mark: %0.0fs", DISTANCE_INTERNAL_TO_MILES(dispRaceLeg->distance), SPEED_INTERNAL_TO_MPH(dispRaceLeg->speed), TIME_INTERNAL_TO_SECONDS(dispRaceLeg->mark));
    display.drawStr(1,60,buffer);
  } else {
    display.drawStr(1,24,"race: none");
    display.drawStr(1,48,"leg: none");
  }

}

void displayMenu(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  int8_t activeLine=(*menuStack.back()).getActiveLine();
  uint8_t i;
  uint8_t y=13;
  if(activeLine==-1) {
    return;
  }
  display.setFont(u8g2_font_spleen6x12_mf);
  for(i=0; i<4; i++) {
    if(i==activeLine) {
      display.drawButtonUTF8(1, y, U8G2_BTN_INV|U8G2_BTN_BW1, 255,  0,  0, (*menuStack.back())[i] );
    } else {
      display.drawButtonUTF8(1, y, U8G2_BTN_BW0, 255,  0,  0, (*menuStack.back())[i]);
    }
    if(i==0) {
      if((*menuStack.back()).moreUp()) {
        display.setFont(u8g2_font_open_iconic_arrow_1x_t);
        display.setDrawColor(2);
        display.drawStr(248,14,"\x43");
        display.setDrawColor(1);
        display.setFont(u8g2_font_spleen6x12_mf);
      }
    }
    if(i==3) {
      if((*menuStack.back()).moreDown()) {
        display.setFont(u8g2_font_open_iconic_arrow_1x_t);
        display.setDrawColor(2);
        display.drawStr(248,53,"\x40");
        display.setDrawColor(1);
        display.setFont(u8g2_font_spleen6x12_mf);
      }
    }
    y+=13;
  }

}

void displayMenuTitle(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  int x;
  static unsigned long ts_last=0;
  unsigned long ts=millis();
  static bool signalVisible=false;
  const char *title=(*menuStack.back()).getMenuTitle();
  time_t utc = now();
  TimeChangeRule *tcr;
  time_t t = timeDateSettings.getTz()->toLocal(utc, &tcr);
  if(ts-ts_last > 500) {
    signalVisible=!signalVisible;
    ts_last=ts;
  }
  display.setFont(u8g2_font_spleen12x24_mf);	
  x=128-(display.getStrWidth(title)/2);
  display.drawStr(x,60,title);
  if(race.raceInProgress()) {
    display.setFont(u8g2_font_spleen6x12_mf);
    display.drawStr(80,36,"Race In Progress"); 
  }

  if(gpsDataPtr->fixValid || signalVisible) {
    display.setFont(u8g2_font_open_iconic_www_1x_t);
    display.drawStr(0,8,"\x51");
  }
  if(gpsDataPtr->fix!=0) {
    display.setFont(u8g2_font_open_iconic_thing_1x_t);
    if(gpsDataPtr->fix==2 || gpsDataPtr->fix==3 || gpsDataPtr->fix==4) {
      display.drawStr(0,17,"\x4f");
    } else {
      display.drawStr(0,17,"\x44");
    }
    display.setFont(u8g2_font_spleen5x8_mf);
    sprintf(buffer, "%dD", gpsDataPtr->fix);
    display.drawStr(9,9,buffer);
    sprintf(buffer, "%2d", gpsDataPtr->siv);
    display.drawStr(9,17,buffer);
    sprintf(buffer, "lat: %+4f\xB0", gpsDataPtr->lat);
    x=128-(display.getStrWidth(buffer)/2);
    display.drawStr(x,7,buffer);	
    sprintf(buffer, "lon: %+4f\xB0", gpsDataPtr->lon);
    x=128-(display.getStrWidth(buffer)/2);
    display.drawStr(x,15,buffer);	
  }
  display.setFont(u8g2_font_spleen6x12_mf);	
  sprintf(buffer, "%02d:%02d:%02d", hour(t), minute(t), second(t));
  display.drawStr(207,9,buffer);	
  display.setFont(u8g2_font_spleen5x8_mf);
  display.drawStr(240,16,tcr->abbrev);
  
}

void displayLegSummaryActual(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  int y;
  int x;
  char sign=' ';
  int32_t targetTime=round(race.activeLeg->targetTime);
  int32_t legTime=round(race.legTime(milliseconds));
  float timeDelta=race.legTimeDelta(seconds);
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(32,12,"Leg Actual: ");
  sprintf(buffer, "%02ld:%02ld:%0ld.%03ld", 
    legTime/3600000, 
    (legTime/60000)%60,
    (legTime/1000)%60,
    legTime%1000);
  display.drawStr(128,12,buffer);

  y=32;
  x=0;
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(x,y," \xd8S:");	
  display.setFont(u8g2_font_spleen8x16_mf);	
  sprintf(buffer, "%7.3f", race.legAverageSpeed(imperial));
  display.drawStr(x+39,y,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(x+97,y,"mph");	

  y=32;
  x=128;
  display.setFont(u8g2_font_spleen8x16_mf);
  display.drawStr(x,y,"  T:");
  display.drawLine(x,y,x+5,y-11);
  display.drawLine(x+1,y-1,x+5,y-10); 

  display.drawLine(x+5,y-11,x+10,y);
  display.drawLine(x+5,y-10,x+9,y-1);
 
  display.drawLine(x,y,x+10,y);
  display.drawLine(x+1,y-1,x+9,y-1);
  display.setFont(u8g2_font_spleen8x16_mf);	
if (timeDelta<0) {
    sign='-';
  } else if (timeDelta>0) {
    sign='+';
  } else {
    sign=' ';
  }
  sprintf(buffer, "%c%3.03f", sign, abs(timeDelta));
  display.drawStr(x+31,y, buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(x+97,y,"sec");	

  y=44;
  x=0;
  display.setFont(u8g2_font_spleen8x16_mf);
  display.drawStr(x,y,"  S:");
  display.drawLine(x,y,x+5,y-11);
  display.drawLine(x+1,y-1,x+5,y-10); 

  display.drawLine(x+5,y-11,x+10,y);
  display.drawLine(x+5,y-10,x+9,y-1);
 
  display.drawLine(x,y,x+10,y);
  display.drawLine(x+1,y-1,x+9,y-1);
  display.setFont(u8g2_font_spleen8x16_mf);	

  sprintf(buffer, "%+8.3f", race.legSpeedDelta(imperial));
  display.drawStr(x+31,y, buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(x+97,y,"mph");	

  y=56;
  x=0;
  display.setFont(u8g2_font_spleen8x16_mf);
  display.drawStr(x,y,"  S:");
  display.drawCircle(x+6, y-6, 4);
  display.drawCircle(x+6, y-6, 2);
  display.drawCircle(x+6, y-6, 0); 

  display.setFont(u8g2_font_spleen8x16_mf);	
  sprintf(buffer, " %7.3f", race.legAdjustedTargetSpeed(imperial));
  display.drawStr(x+31,y, buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(x+97,y,"mph");	

  y=44;
  x=128;
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(x,y,"Dis:");	
  display.setFont(u8g2_font_spleen8x16_mf);	
  sprintf(buffer, "%7.3f", race.legDistanceComplete(imperial));
  display.drawStr(x+39,y,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(x+97,y,"miles");	

  y=56;
  x=128;
  display.setFont(u8g2_font_spleen8x16_mf);
  display.drawStr(x,y,"  T:");
  display.drawCircle(x+5, y-6, 4);
  display.drawCircle(x+5, y-6, 2);
  display.drawCircle(x+5, y-6, 0); 

  display.setFont(u8g2_font_spleen8x16_mf);	

  sprintf(buffer, "%02ld:%02ld:%02ld.%03ld", 
    targetTime/3600000, 
    (targetTime/60000)%60,
    (targetTime/1000)%60,
    targetTime%1000);
  display.drawStr(x+31,y, buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	

}

void displayLegSummaryAdjusted(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  int y;
  int x;
  char sign;
  //We use actual leg time rather than targetTime to account for any time delta corrections applied.
  int32_t targetTime=(int32_t)round(race.legTargetTime(milliseconds));
  double adjustedAverageSpeed=race.legAdjustedAverageSpeed(imperial);
  int32_t adjustedTimeDelta=round(race.legTimeDelta(milliseconds));
  display.setFont(u8g2_font_spleen8x16_mf);	

  x=136;
  y=12;
  sprintf(buffer, "Leg Adjusted   D: %+7.3f", DISTANCE_INTERNAL_TO_MILES(race.activeLeg->distance-race.activeLeg->driveDistance));
  display.drawStr(32,12,buffer);
  display.drawLine(x,y,x+5,y-11);
  display.drawLine(x+1,y-1,x+5,y-10); 

  display.drawLine(x+5,y-11,x+10,y);
  display.drawLine(x+5,y-10,x+9,y-1);
 
  display.drawLine(x,y,x+10,y);
  display.drawLine(x+1,y-1,x+9,y-1);

  y=32;
  x=0;
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(x,y," \xd8S:");	
  display.setFont(u8g2_font_spleen8x16_mf);	
  sprintf(buffer, "%7.3f", adjustedAverageSpeed);
  display.drawStr(x+39,y,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(x+97,y,"mph");	

  y=32;
  x=128;
  display.setFont(u8g2_font_spleen8x16_mf);
  display.drawStr(x,y,"  T:");
  display.drawLine(x,y,x+5,y-11);
  display.drawLine(x+1,y-1,x+5,y-10); 

  display.drawLine(x+5,y-11,x+10,y);
  display.drawLine(x+5,y-10,x+9,y-1);
 
  display.drawLine(x,y,x+10,y);
  display.drawLine(x+1,y-1,x+9,y-1);
  display.setFont(u8g2_font_spleen8x16_mf);	
  if(adjustedTimeDelta==0) {
    sign=' ';
  } else if (adjustedTimeDelta<0) {
    sign='-';
  } else if (adjustedTimeDelta>0) {
    sign='+';
  }
  sprintf(buffer, "%c%3ld.%03ld", sign, abs(adjustedTimeDelta/1000), abs(adjustedTimeDelta%1000));
  display.drawStr(x+31,y, buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(x+97,y,"sec");	

  y=44;
  x=0;
  display.setFont(u8g2_font_spleen8x16_mf);
  display.drawStr(x,y,"  S:");
  display.drawLine(x,y,x+5,y-11);
  display.drawLine(x+1,y-1,x+5,y-10); 

  display.drawLine(x+5,y-11,x+10,y);
  display.drawLine(x+5,y-10,x+9,y-1);
 
  display.drawLine(x,y,x+10,y);
  display.drawLine(x+1,y-1,x+9,y-1);
  display.setFont(u8g2_font_spleen8x16_mf);	

  sprintf(buffer, "%+8.3f",adjustedAverageSpeed-SPEED_INTERNAL_TO_MPH(race.activeLeg->speed));
  display.drawStr(x+31,y, buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(x+97,y,"mph");	

  y=56;
  x=0;
  display.setFont(u8g2_font_spleen8x16_mf);
  display.drawStr(x,y,"  S:");
  display.drawCircle(x+6, y-6, 4);
  display.drawCircle(x+6, y-6, 2);
  display.drawCircle(x+6, y-6, 0); 

  display.setFont(u8g2_font_spleen8x16_mf);	
  sprintf(buffer, " %7.3f", SPEED_INTERNAL_TO_MPH(race.activeLeg->speed));
  display.drawStr(x+31,y, buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(x+97,y,"mph");	

  y=44;
  x=128;
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(x,y,"Dis:");	
  display.setFont(u8g2_font_spleen8x16_mf);	
  sprintf(buffer, "%7.3f", DISTANCE_INTERNAL_TO_MILES(race.activeLeg->distance));
  display.drawStr(x+39,y,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(x+97,y,"miles");	

  y=56;
  x=128;
  display.setFont(u8g2_font_spleen8x16_mf);
  display.drawStr(x,y,"  T:");
  display.drawCircle(x+5, y-6, 4);
  display.drawCircle(x+5, y-6, 2);
  display.drawCircle(x+5, y-6, 0); 

  display.setFont(u8g2_font_spleen8x16_mf);	

  sprintf(buffer, "%02ld:%02ld:%02ld.%03ld", 
    targetTime/3600000, 
    (targetTime/60000)%60,
    (targetTime/1000)%60,
    targetTime%1000);
  display.drawStr(x+31,y, buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	
}

void displayRaceSummary1(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {

  int32_t raceTime=(int)round(race.raceTime(milliseconds));
  int32_t targetTime=(int)round(race.activeLeg->raceLegEndTargetTime);
  display.setFont(u8g2_font_spleen12x24_mf);
  sprintf(buffer, "Race:  %02ld:%02ld:%02ld.%03ld", 
    raceTime/3600000, 
    (raceTime/60000)%60,
    (raceTime/1000)%60,
    raceTime%1000);
  display.drawStr(0,20,buffer);
   sprintf(buffer, "   T:  %02ld:%02ld:%02ld.%03ld", 
    targetTime/3600000, 
    (targetTime/60000)%60,
    (targetTime/1000)%60,
    targetTime%1000);
  display.drawStr(0,40,buffer);
  display.drawCircle(24, 32, 7);
  display.drawCircle(24, 32, 6);
  display.drawCircle(24, 32, 3);
  display.drawCircle(24, 32, 4);
  display.drawCircle(24, 32, 1);
  display.drawCircle(24, 32, 0); 
  sprintf(buffer, "   T: %+3.03f", race.raceTimeDelta(seconds));
  display.drawStr(0,60,buffer);
  display.drawTriangle(14,60, 23,45, 33,60);
  display.setDrawColor(0);
  display.drawTriangle(17,58, 23,48, 29,58);
  display.setDrawColor(1);
}

void displayRaceSummary2(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  double averageSpeed=race.raceAdjustedAverageSpeed(imperial);
  display.setFont(u8g2_font_spleen12x24_mf);
  sprintf(buffer, "Dist: %8.3f", race.raceTargetDistanceComplete(imperial));
  display.drawStr(0,20,buffer);
  sprintf(buffer, "  \xd8S: %8.3f", averageSpeed);
  display.drawStr(0,40,buffer);
  sprintf(buffer, "   S: %+8.3f", averageSpeed-SPEED_INTERNAL_TO_MPH(race.activeLeg->raceLegEndAvgSpeed));
  display.drawStr(0,60,buffer);
  display.drawTriangle(14,60, 23,45, 33,60);
  display.setDrawColor(0);
  display.drawTriangle(17,58, 23,48, 29,58);
  display.setDrawColor(1);
}

void displayPoint(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  char dir;
  int x;
  double distRemainingInt=0;
  float distRemaining=0;
  float timeRemaining=0;
  char tUnit[4]="sec";
  if(race.activePoint==race.activeLeg->points.end()) {
    display.setFont(u8g2_font_spleen16x32_mf);
    display.drawStr(24, 60, "No Point Data");
    tUnit[0]=0;
  } else {
    //Calculate distance to next point in 
    distRemainingInt=(*race.activePoint)->distance-DISTANCE_MILES_TO_INTERNAL(race.legDistanceComplete(imperial));
    distRemaining=DISTANCE_INTERNAL_TO_MILES(distRemainingInt);
    //Calculate time to point based on our current instantaneous speed
    //We convert this to floating point seconds to make the rest of the logic simpler
    if(gpsData.speed!=0) {
      timeRemaining=TIME_INTERNAL_TO_SECONDS(distRemainingInt/gpsData.speed);
    } else {
      timeRemaining=0;
      tUnit[0]=0;
    }
    //if we are more than sixty seconds out(60,000 milliseconds), we are going to print time in either 
    //minutes or hours
    if(timeRemaining>60) {
      timeRemaining/=60;
      //If we are more than an hour out, print time in hours.
      if(timeRemaining>60) {
        timeRemaining/=60;
        strcpy(tUnit, "hr");
      } else {
        strcpy(tUnit, "min");
      }
      
    }
    if((*(race.activePoint))->turnDir==0) {
      dir='L';
    } else {
      dir='R';
    }
    if((*(race.activePoint))->turn>0) {
      display.setFont(u8g2_font_spleen16x32_mf);
      sprintf(buffer, "%d%c", (*race.activePoint)->turn, dir );
    } else {
      display.setFont(u8g2_font_open_iconic_embedded_2x_t);
      sprintf(buffer, "\x47\x47");
    }
    x=3+((64-display.getStrWidth(buffer))/2);
    display.drawStr(x, 25, buffer);
    display.setFont(u8g2_font_spleen16x32_mf);
    if(distRemaining>=100) {
      sprintf(buffer, "%4d", (int)distRemaining);
    } else if(distRemaining>=10) {
      sprintf(buffer, "%4.1f", distRemaining);
    } else {
      sprintf(buffer, "%4.2f", distRemaining);
    }
    display.drawStr(73, 25, buffer);
    
    sprintf(buffer, "%5.2f", timeRemaining);
    display.drawStr(153, 25, buffer);   



    display.setFont(u8g2_font_spleen8x16_mf);
    display.drawStr(0,46, (*race.activePoint)->descrLine1.c_str());
    display.drawStr(0,60, (*race.activePoint)->descrLine2.c_str());
    
  }
  display.setFont(u8g2_font_spleen5x8_mf);
  display.drawStr(138, 25, "mi");
  display.drawStr(233, 25, tUnit);
  display.drawRFrame(0, 0, 70, 32, 5);
  display.drawRFrame(72, 0, 78, 32, 5);
  display.drawRFrame(152, 0, 99, 32, 5);
  
}

void displayFirmwareConfirm(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen16x32_mf);	
  display.drawStr(5,20,"Firmware update");	
  display.setFont(u8g2_font_spleen12x24_mf);
  display.drawStr(25,48,"exit or continue?");	
}

void displayFirmwareUpdate(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  const char *firmwareAction[5]={"",
                            "Reading Firmware",
                            "Writing Firmware",
                            "Clean Up",
                            "Reboot"};
  int x;
  int progress;
  display.setFont(u8g2_font_spleen16x32_mf);
  x=128-(display.getStrWidth(firmwareAction[firmwareUpdateState])/2);	
  display.drawStr(x,20,firmwareAction[firmwareUpdateState]);	
  if(firmwareProgress==0) {
    progress=0;
  } else if (firmwareProgress>firmwareSize) {
    progress=100;
  } else {
    progress=(firmwareProgress*100)/firmwareSize;
  }
  display.drawFrame(27,32,200,16);
  display.drawBox(27,32,(progress*2),16);

}

void displaySetLedBrightness(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  int x;
  display.setFont(u8g2_font_spleen8x16_mf);
  sprintf(buffer, "Brightness: %d", ledBrightnessTmp);
  x=128-(display.getStrWidth(buffer)/2);
  display.drawStr(x,20,buffer);
  display.drawFrame(27,32,200,16);
  display.drawBox(27,32,(200.0/14.0)*(float)ledBrightnessTmp,16);
  return;
}

void displaySetOledBrightness(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  int x;
  display.setFont(u8g2_font_spleen8x16_mf);
  sprintf(buffer, "Brightness: %d", oledBrightnessTmp);
  x=128-(display.getStrWidth(buffer)/2);
  display.drawStr(x,20,buffer);
  display.drawFrame(27,32,200,16);
  display.drawBox(27,32,(200.0/250.0)*(float)oledBrightnessTmp,16);
  return;
}

void displaySetDisplayTimeout(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  int x;
  display.setFont(u8g2_font_spleen8x16_mf);
  sprintf(buffer, "Timeout: %d min", displayTimeoutTmp);
  x=128-(display.getStrWidth(buffer)/2);
  display.drawStr(x,20,buffer);
  display.drawFrame(27,32,200,16);
  display.drawBox(27,32,(200.0/60)*(float)displayTimeoutTmp,16);
  return;
}

void displayAdjustLeg1(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  int x;
  int y;

  //The distance and speed values used by the adjustment menus are already in floating point mph and miles/
  //No need to convert here.
  //Seconds are still in milliseconds.
  display.setFont(u8g2_font_spleen12x24_mf);
  sprintf(buffer, " Time:  %02ld:%02ld:%02ld.%03ld", 
    legAdjustTime/3600000, 
    (legAdjustTime/60000)%60,
    (legAdjustTime/1000)%60,
    legAdjustTime%1000);
  display.drawStr(0,20,buffer);
  sprintf(buffer, " Dist: %7.3f", legAdjustDist);
  display.drawStr(0,40,buffer);
  sprintf(buffer, "Speed: %7.3f", legAdjustSpeed);
  display.drawStr(0,60,buffer);
  display.setFont(u8g2_font_spleen8x16_mf);
  display.drawStr(169,40,"mi");
  display.drawStr(169,60,"mph");
  y=(legAdjustRow*20)+2;
  if(legAdjustRow==0) {
    x=(legAdjustColumn*-12)+191;
  } else {
    x=(legAdjustColumn*-12)+119;
  }
  if(legAdjustRow<3) {
    if(legAdjustMode) {
      if(legAdjustMode==2) {
        display.setDrawColor(2);
        display.drawBox(x, y, 14, 20);
        display.setDrawColor(1);
      } else {
        display.drawFrame(x, y, 14, 20);
      }
    }
  }
}

void displayAdjustLeg2(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  int x;
  int y;

  //The distance and speed values used by the adjustment menus are already in floating point mph and miles
  //No need to convert here.
  //Seconds are still in milliseconds.
  display.setFont(u8g2_font_spleen12x24_mf);
  sprintf(buffer, " Mark:  %02ld", legAdjustMark);
  display.drawStr(0,20,buffer);
  display.setFont(u8g2_font_spleen8x16_mf);
  display.drawStr(121,20,"sec");
  y=((legAdjustRow-3)*20)+2;
  if(legAdjustRow==0) {
    x=(legAdjustColumn*-12)+191;
  } else {
    x=(legAdjustColumn*-12)+119;
  }
  if(legAdjustRow>2) {
    if(legAdjustMode) {
      if(legAdjustMode==2) {
        display.setDrawColor(2);
        display.drawBox(x, y, 14, 20);
        display.setDrawColor(1);
      } else {
        display.drawFrame(x, y, 14, 20);
      }
    }
  }
}

void displaySystemInfo(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen8x16_mf);
  if(strlen(hardwareVersion.pcbOther)!=0) {
    sprintf(buffer, "HW Version: %d.%d-%s", hardwareVersion.pcbMajor, hardwareVersion.pcbMinor, hardwareVersion.pcbOther);
  } else {
    sprintf(buffer, "HW Version: %d.%d", hardwareVersion.pcbMajor, hardwareVersion.pcbMinor);
  }
  display.drawStr(0,20,buffer);
  display.drawStr(0,40,VERSION_STRING);
  if(strlen(hardwareVersion.serialOther)!=0) {
    sprintf(buffer, "Serial #: %06ld-%s", hardwareVersion.serialNo, hardwareVersion.serialOther);
  } else {
    sprintf(buffer, "Serial #: %06ld", hardwareVersion.serialNo);
  }
  display.drawStr(0,60,buffer);
  return;
}

void displayUpdateFast(void) {
  if(SPILock) {
    return;
  }
  if(ledDispFunc!=NULL) {
    ledDispFunc();
  }
}

void ledDispLegTime(void) {
  int32_t ts=(int32_t)getTimeStamp();
  int seconds;
  int millis;
  seconds=ts/1000;
  millis=ts%1000;
  if(!race.legInProgress()) {
    return;
  }
  ledDisp.RefreshMe();
  sprintf(buffer, "%02d.%02d", seconds/3600, (seconds/60)%60);
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer);
  sprintf(buffer, "%02d.%02d", (seconds/60)%60, seconds%60);
  ledDisp.Set_Position(2);
  ledDisp.ShowMe(buffer); 
  sprintf(buffer, "%02d.%02d", seconds%60, millis/10);
  ledDisp.Set_Position(4);
  ledDisp.ShowMe(buffer);     
}

void ledDispRaceTime(void) {
  uint32_t ts=(uint32_t)(getTimeStamp()+race.raceTimeComplete(milliseconds));
  int seconds;
  int millis;
  seconds=ts/1000;
  millis=ts%1000;
  if(!race.legInProgress()) {
    return;
  }
  ledDisp.RefreshMe();
  sprintf(buffer, "%02d.%02d", seconds/3600, (seconds/60)%60);
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer);
  sprintf(buffer, "%02d.%02d", (seconds/60)%60, seconds%60);
  ledDisp.Set_Position(2);
  ledDisp.ShowMe(buffer); 
  sprintf(buffer, "%02d.%02d", seconds%60, millis/10);
  ledDisp.Set_Position(4);
  ledDisp.ShowMe(buffer);     
}


void ledDispLegDeltaSpeed(void) {
  ledDisp.RefreshMe();
  sprintf(buffer, "%9.3f", race.legSpeedDelta(imperial));
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer); 
}

void ledDispRaceDeltaSpeed(void) {
  ledDisp.RefreshMe();
  sprintf(buffer, "%9.3f", race.raceSpeedDelta(imperial));
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer); 
}

void ledDispGpsSpeed(void) {
  ledDisp.RefreshMe();
  sprintf(buffer, "%9.3f", SPEED_INTERNAL_TO_MPH(gpsData.speed));
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer); 
}

void ledDispLegAverageSpeed(void) {
  ledDisp.RefreshMe();
  sprintf(buffer, "%9.3f",race.legAverageSpeed(imperial));
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer); 
}

void ledDispLegDistanceRemain(void) {
  ledDisp.RefreshMe();
  sprintf(buffer, "%9.3f", race.legDistanceRemaining(imperial));
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer); 
}

void ledDispLegDeltaTime(void) {
  ledDisp.RefreshMe();
  sprintf(buffer, "%9.3f", race.legTimeDelta(seconds));
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer); 
}

void ledDispLegDistance(void) {
  ledDisp.RefreshMe();
  sprintf(buffer, "%9.3f", race.legDistanceComplete(imperial));
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer); 
}

void ledDispRaceAverageSpeed(void) {
  ledDisp.RefreshMe();
  sprintf(buffer, "%9.3f", race.raceAverageSpeed(imperial));
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer); 
}

void ledDispRaceDistanceRemain(void) {
  ledDisp.RefreshMe();
  sprintf(buffer, "%9.3f", race.raceDistanceRemaining(imperial));
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer); 
}

void ledDispRaceDistance(void) {
  ledDisp.RefreshMe();
  sprintf(buffer, "%9.3f", race.raceDriveDistanceComplete(imperial));
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer); 
}

void ledDispRaceDeltaTime(void) {
  ledDisp.RefreshMe();
  sprintf(buffer, "%9.3f", race.raceTimeDelta(seconds));
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer); 
}

void ledDispDashes(void) {
  ledDisp.RefreshMe();
  ledDisp.Set_Position(0);
  ledDisp.ShowMe("--------");
}

void ledDispEights(void) {
  ledDisp.RefreshMe();
  ledDisp.Set_Position(0);
  ledDisp.ShowMe("88888888");
}

void ledDispBlank(void) {
  ledDisp.RefreshMe();
  ledDisp.Set_Position(0);
  ledDisp.ShowMe("        ");
}


void ledDispStartCountdown(void) {
  volatile double ts=getTimeStamp();
  ledDisp.RefreshMe();
  sprintf(buffer, "%9.2f", TIME_INTERNAL_TO_SECONDS(ts));
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer);
}

void displayError(const char *err) {
  oledDisp4.clearBuffer();
  oledDisp4.setFont(u8g2_font_spleen12x24_mf);
  oledDisp4.drawStr(0,24,err);	
  oledDisp4.sendBuffer();
}

void displayDisplayConfig(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  int oled=0;
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(0,12, "  LED:");
  if(displaySelectLine==0) {
    if(displaySelectLineMode==true) {
      display.drawButtonUTF8(42, 12, U8G2_BTN_INV|U8G2_BTN_BW1, 213,  0,  0, LEDDisplayDescr[LEDDisplaySelect] );
    } else {
      display.drawButtonUTF8(42, 12, U8G2_BTN_BW1, 255,  0,  0, LEDDisplayDescr[LEDDisplaySelect] );
    }
  } else {
    display.drawStr(42,12, LEDDisplayDescr[LEDDisplaySelect]);
  }
  for(oled=0; oled<4; oled++) {
    sprintf(buffer, "OLED%d:", oled+1);
    display.drawStr(0,24+(oled*12), buffer);
    if(displaySelectLine==oled+1) {
      if(displaySelectLineMode==true) {
        display.drawButtonUTF8(42, 24+(oled*12), U8G2_BTN_INV|U8G2_BTN_BW1, 213,  0,  0, OLEDDisplayDescr[OLEDDisplaySelect[oled]] );
      } else {
        display.drawButtonUTF8(42, 24+(oled*12), U8G2_BTN_BW1, 213,  0,  0, OLEDDisplayDescr[OLEDDisplaySelect[oled]] );
      }
    } else {
      display.drawStr(42,24+(oled*12), OLEDDisplayDescr[OLEDDisplaySelect[oled]]);
    }  
  }
}

void displaySetLegTimeAdjust(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  int x;
  int w;
  display.setFont(u8g2_font_spleen6x12_mf);
  sprintf(buffer, "Leg Timing Update Mode");
  x=128-(display.getStrWidth(buffer)/2);
  display.drawStr(x,20,buffer);
  sprintf(buffer, "Automatic");
  w=display.getStrWidth(buffer);
  x=128-(w/2);
  display.drawStr(x, 52, buffer);
  display.drawStr(x, 40, "Manual");
  display.setDrawColor(2);  
  if(autoAdjustLegTime) {
    display.drawBox(x-1,43,w+1,10);  
  } else {
    display.drawBox(x-1,31,w+1,10); 
  }
  display.setDrawColor(1);
  if(autoAdjustLegTimeSave) {
    display.drawDisc(x-10,48,3,U8G2_DRAW_ALL);
  } else {
    display.drawDisc(x-10,36,3,U8G2_DRAW_ALL);
  }  

}
  
void displaySetSpeedBandSource(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  int x;
  int w;
  display.setFont(u8g2_font_spleen6x12_mf);
  sprintf(buffer, "Speed Band Comparison Source");
  x=128-(display.getStrWidth(buffer)/2);
  display.drawStr(x,15,buffer);
  sprintf(buffer, "Disabled");
  w=display.getStrWidth(buffer);
  x=128-(w/2);
  display.drawStr(x, 35, buffer);
  display.drawStr(x, 47, "Leg");
  display.drawStr(x, 59, "Race");


  display.setDrawColor(2);  
  switch(speedBandSource) {
    case 0:
      display.drawBox(x-1,26,w+1,10); 
      break;
    case 1:
      display.drawBox(x-1,38,w+1,10); 
      break;
    case 2:
      display.drawBox(x-1,50,w+1,10); 
      break;
    default:
      break;
  }

  display.setDrawColor(1);

  switch(speedBandSourceSave) {
    case 0:
      display.drawDisc(x-10,31,3,U8G2_DRAW_ALL);
      break;
    case 1:
      display.drawDisc(x-10,43,3,U8G2_DRAW_ALL);
      break;
    case 2:
      display.drawDisc(x-10,55,3,U8G2_DRAW_ALL);
      break;
    default:
      break;
  }
}