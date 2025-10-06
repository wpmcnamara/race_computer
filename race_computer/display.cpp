#include "display.h"
#include "timer.h"
#include "gps.h"
#include "agr_logo_bottom.h"
#include "agr_logo_top.h"
#include "event.h"
#include "race.h"

//8 digit LED display
CK_MAX ledDisp(LED_DISP_LOAD);

#ifdef __BREADBOARD__
#define ROTATION U8G2_R0
#else
#define ROTATION U8G2_R2
#endif 

//OLED displays
//New Haven Displays 256x64 is a good enough match for the displays on the dev board.  U8G2_R0 sets rotation
//to 0 degrees.  Boards are configured for four wire SPI
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oledDisp1(ROTATION, OLED_DISP1_CS, OLED_DISP_DC, OLED_DISP1_RESET);
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oledDisp2(ROTATION, OLED_DISP2_CS, OLED_DISP_DC, OLED_DISP2_RESET);
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oledDisp3(ROTATION, OLED_DISP3_CS, OLED_DISP_DC, OLED_DISP3_RESET);
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI oledDisp4(ROTATION, OLED_DISP4_CS, OLED_DISP_DC, OLED_DISP4_RESET);

uint8_t *oledDisp1Buffer;
uint8_t *oledDisp2Buffer;
uint8_t *oledDisp3Buffer;
uint8_t *oledDisp4Buffer;

char buffer[256];

struct gpsDataStruct *gpsDataPtr;
orcTime_t *gpsTimePtr;
uint8_t menuItem=0;
race_t *dispRace;
raceLeg_t *dispRaceLeg;
bool raceSelectHighlight=false;
bool raceLegSelectHighlight=false;
void (*ledDispFunc)(void)=NULL;

std::list<displayContent_t*> displayList;

displayContent::displayContent(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &argScreen, 
  dispPos_t argPosX, 
  dispPos_t argPosY, 
  void (*argShowFunc)(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &, dispPos_t, dispPos_t)) : screen(argScreen),
  posX(argPosX), posY(argPosY), showFunc(argShowFunc) {};

void displayContent::display(void) {
  showFunc(screen, posX, posY);
}

void displaySetup(void) {
  dispRace=NULL;
  dispRaceLeg=NULL;
  oledDisp1.setBufferPtr(new uint8_t[oledDisp1.getBufferSize()]);
  oledDisp1.initDisplay();
  oledDisp1.clearDisplay();
  oledDisp1.setPowerSave(0);
  oledDisp2.setBufferPtr(new uint8_t[oledDisp2.getBufferSize()]);
  oledDisp2.initDisplay();
  oledDisp2.clearDisplay();
  oledDisp2.setPowerSave(0);  
  oledDisp3.setBufferPtr(new uint8_t[oledDisp3.getBufferSize()]);
  oledDisp3.initDisplay();
  oledDisp3.clearDisplay();
  oledDisp3.setPowerSave(0);  
  oledDisp4.setBufferPtr(new uint8_t[oledDisp4.getBufferSize()]);
  oledDisp4.initDisplay();
  oledDisp4.clearDisplay();
  oledDisp4.setPowerSave(0);  
  
  ledDisp.begin();
  ledDisp.Set_Brightness(3);
  // Turn Off the Auto Refresh
  ledDisp.AutoRefresh(false);
  ledDisp.RefreshMe();
  ledDisp.Set_Position(0);
  ledDisp.ShowMe("01234567");

  oledDisp1.clearBuffer();
  oledDisp1.setFont(u8g2_font_spleen32x64_mf);	
  oledDisp1.drawStr(0,53,"Disp 1");	 
  oledDisp1.sendBuffer();
  oledDisp2.clearBuffer();
  oledDisp2.setFont(u8g2_font_spleen32x64_mf);	
  oledDisp2.drawStr(0,53,"Disp 2");	  
  oledDisp2.sendBuffer();
  oledDisp3.clearBuffer();
  oledDisp3.setFont(u8g2_font_spleen32x64_mf);	
  oledDisp3.drawStr(0,53,"Disp 3");	 
  oledDisp3.sendBuffer();
  oledDisp4.clearBuffer();
  oledDisp4.setFont(u8g2_font_spleen32x64_mf);	
  oledDisp4.drawStr(0,53,"Disp 4");	 
  oledDisp4.sendBuffer();

  delay(1000);
  ledDisp.ShowMe("--------");
  ledDisp.Set_Position(0);
  oledDisp1.clearBuffer();
  oledDisp1.drawXBM(18,2,agr_top_width, agr_top_height, agr_logo_top);
  oledDisp1.sendBuffer();
  oledDisp2.clearBuffer();
  oledDisp2.drawXBM(18,0,agr_bottom_width, agr_bottom_height, agr_logo_bottom);
  oledDisp2.sendBuffer();
  oledDisp3.clearBuffer();
  oledDisp3.setFont(u8g2_font_spleen16x32_mf);	
  oledDisp3.drawStr(55,20,"Open Race");	
  oledDisp3.drawStr(63,52,"Computer");	
  oledDisp3.sendBuffer();
  oledDisp4.clearBuffer();
  oledDisp4.setFont(u8g2_font_spleen12x24_mf);
  oledDisp4.drawStr(20,20,"\xA9 Patrick McNamara");	
  oledDisp4.drawStr(38,52,"firmware: 0.2.0");	
  oledDisp4.sendBuffer();
  delay(5000);

  new event_t(displayUpdateFast, eventRepeat, true, false, 0, 1, &Serial, "displayUpdateFast");
  new event_t(displayUpdate, eventRepeat, true, false, 0, 10, &Serial, "displayUpdate");
  gpsDataPtr=getGpsData();
  gpsTimePtr=getGpsTime();
}

void displayUpdate() {
  struct gpsDataStruct *gpsData=getGpsData();
  orcTime_t *gpsTime=getGpsTime();

  oledDisp1.clearBuffer();
  oledDisp2.clearBuffer();
  oledDisp3.clearBuffer();
  oledDisp4.clearBuffer();
  for (std::list<displayContent_t *>::iterator it=displayList.begin(); it != displayList.end(); ++it) {
      ((*it)->display)();
  }
  oledDisp1.sendBuffer();
  oledDisp2.sendBuffer();
  oledDisp3.sendBuffer();
  oledDisp4.sendBuffer();
}

void displayGPSInfo(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen16x32_mf);	
  sprintf(buffer, "%02d:%02d:%02d", gpsTimePtr->hour, gpsTimePtr->minute, gpsTimePtr->second);
  display.drawStr(64,20,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(194,20,"GMT");
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

void displayDeltaTimeMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  unsigned int y=0;
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

  if(race.legData->timeDelta==0) {
    sign=' ';
  } else if (race.legData->timeDelta<0) {
    sign='-';
  } else if (race.legData->timeDelta>0) {
    sign='+';
  }
  sprintf(buffer, "%c%3d.%03u", sign, abs(race.legData->timeDelta/1000), abs(race.legData->timeDelta%1000) );
  display.drawStr(81,y, buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(225,y,"sec");	
}

void displayDeltaSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
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

  sprintf(buffer, "%8.3f", race.legData->speedDelta*2.23694);
  oledDisp2.drawStr(97,y,buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(225,y,"mph");  
}

void displayDistanceMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
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
  sprintf(buffer, "%8.3f", race.legData->distance*0.000621372);
  display.drawStr(97,y,buffer);	 
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(225,y,"miles");	
}

void displayDistRemainMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
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
  sprintf(buffer, "%8.3f", race.legData->distanceRemaining*0.000621372);
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
  oledDisp4.drawStr(0,y,"    speed:");	 
  display.setFont(u8g2_font_spleen16x32_mf);	
  sprintf(buffer, "%8.3f", gpsDataPtr->speed*2.23694);
  display.drawStr(97,y,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(225,y,"mph");	
}

void displayAvgSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
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
  sprintf(buffer, "%8.3f", race.legData->averageSpeed*2.23694);
  display.drawStr(97,y,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(225,y,"mph");	
}

void displayGpsSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(0,41,"speed");	
  display.setFont(u8g2_font_logisoso50_tn);
  sprintf(buffer, "%7.3f", gpsDataPtr->speed*2.23694);
  display.drawStr(42,61,buffer);	
  display.setFont(u8g2_font_spleen12x24_mf);	
  display.drawStr(0,61,"mph");	  
}

void displayAvgSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(11,13,"leg");
  display.setFont(u8g2_font_spleen16x32_mf);
  display.drawStr(6,41,"\xd8S");	
  display.setFont(u8g2_font_logisoso50_tn);
  sprintf(buffer, "%7.3f", race.legData->averageSpeed*2.23694);
  display.drawStr(42,61,buffer);	
  display.setFont(u8g2_font_spleen12x24_mf);	
  display.drawStr(0,61,"mph");
}

void displayDistanceLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(11,14,"leg");
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(0,41," dist");
  display.drawStr(0,61,"miles");	
  display.setFont(u8g2_font_logisoso50_tn);
  sprintf(buffer, "%7.3f", race.legData->distance*0.000621372);
  display.drawStr(42,61,buffer);	
}

void displayDistRemainLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(11,13,"leg");
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(0,29," dist");
  display.drawStr(0,45,"  rem");
  display.drawStr(0,61,"miles");	
  display.setFont(u8g2_font_logisoso50_tn);
  sprintf(buffer, "%7.3f", race.legData->distanceRemaining*0.000621372);
  display.drawStr(42,61,buffer);	
}

void displayDeltaTimeLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  char sign;
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(11,13,"leg");  
  display.setFont(u8g2_font_spleen16x32_mf);
  display.drawStr(22,41,"T");	
  display.drawTriangle(4,41, 12,22, 21,41);
  display.setDrawColor(0);
  display.drawTriangle(7,38, 12,27, 17,38);
  display.setDrawColor(1);
  display.setFont(u8g2_font_logisoso50_tn);
  if(race.legData->timeDelta==0) {
    sign=' ';
  } else if (race.legData->timeDelta<0) {
    sign='-';
  } else if (race.legData->timeDelta>0) {
    sign='+';
  }  
  sprintf(buffer, "%c%3d.%03u", sign, abs(race.legData->timeDelta/1000), abs(race.legData->timeDelta%1000));
  display.drawStr(42,61,buffer);	
  display.setFont(u8g2_font_spleen12x24_mf);	
  display.drawStr(0,61,"sec");
}

void displayDeltaSpeedLarge(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  display.drawStr(11,13,"leg");
  display.setFont(u8g2_font_spleen16x32_mf);
  display.drawStr(22,41,"S");	
  display.drawTriangle(4,41, 12,22, 21,41);
  display.setDrawColor(0);
  display.drawTriangle(7,38, 12,27, 17,38);
  display.setDrawColor(1);
  display.setFont(u8g2_font_logisoso50_tn);
  sprintf(buffer, "%7.3f", race.legData->speedDelta*2.23694);
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
  sprintf(buffer, "%8.3f", race.legData->averageSpeed*2.23694);
  display.drawStr(x+36,y,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(x+100,y,"mph");	
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
    sprintf(buffer, "dist: %8.3f  speed: %07.3f  timing: %ds", dispRace->distance, dispRace->speed, dispRace->mark);
    display.drawStr(1,36,buffer);
    sprintf(buffer, "leg: %s", dispRaceLeg->descr.c_str());
    if(raceLegSelectHighlight) {
      display.drawButtonUTF8(1, 48, U8G2_BTN_INV|U8G2_BTN_BW1, 255,  0,  0, buffer );
    } else {
      display.drawButtonUTF8(1, 48, U8G2_BTN_BW0, 255,  0,  0, buffer );
    }
    sprintf(buffer, "dist: %8.3f  speed: %07.3f  timing: %ds", dispRaceLeg->distance, dispRaceLeg->speed, dispRaceLeg->mark);
    display.drawStr(1,60,buffer);
  } else {
    display.drawStr(1,24,"race: none");
    display.drawStr(1,48,"leg: none");
  }

}
void displayMenu(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen6x12_mf);
  if(menuItem==0) {
  display.drawButtonUTF8(1, 13, U8G2_BTN_INV|U8G2_BTN_BW1, 255,  0,  0, "Start Leg" );
  } else {
  display.drawButtonUTF8(1, 13, U8G2_BTN_BW0, 255,  0,  0, "Start Leg" );
  }
  if(menuItem==1) {
  display.drawButtonUTF8(1, 26, U8G2_BTN_INV|U8G2_BTN_BW1, 255,  0,  0, "Select Race" );
  } else {
  display.drawButtonUTF8(1, 26, U8G2_BTN_BW0, 255,  0,  0, "Select Race" );
  }
  if(menuItem==2) {
  display.drawButtonUTF8(1, 39, U8G2_BTN_INV|U8G2_BTN_BW1, 255,  0,  0, "Select Leg" );
  } else {
  display.drawButtonUTF8(1, 39, U8G2_BTN_BW0, 255,  0,  0, "Select Leg" );
  }
}
void displayMenuTitle(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen12x24_mf);	
  display.drawStr(74,20,"Main Menu");
  if(race.inProgress) {
    display.setFont(u8g2_font_spleen6x12_mf);
    display.drawStr(80,36,"Race In Progress"); 
  }
}

void displayRaceSelectTitle(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  display.setFont(u8g2_font_spleen12x24_mf);	
  display.drawStr(62,20,"Race Select");
}

void displayRaceLegSelectTitle(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
 display.setFont(u8g2_font_spleen12x24_mf);	
 display.drawStr(38,20,"Race Leg Select");
}

void displayLegSummaryActual(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  int y;
  int x;
  char sign;
  long targetTime=((float)race.legData->totalDistance/race.legData->targetSpeed)*1000.0;
  long int legTime=((race.legData->endTs.seconds*1000)+race.legData->endTs.millis)-((race.legData->startTs.seconds*1000)+race.legData->startTs.millis);
  long timeDelta=legTime-targetTime;
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(32,12,"Leg Actual: ");
  sprintf(buffer, "%02ld:%02ld:%02ld.%03ld", 
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
  sprintf(buffer, "%7.3f", race.legData->averageSpeed*2.23694);
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
  if(timeDelta==0) {
    sign=' ';
  } else if (timeDelta<0) {
    sign='-';
  } else if (timeDelta>0) {
    sign='+';
  }
  sprintf(buffer, "%c%3d.%03u", sign, abs(timeDelta/1000), abs(timeDelta%1000));
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

  sprintf(buffer, "%+8.3f", (race.legData->averageSpeed-race.legData->targetSpeed)*2.23694);
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
  sprintf(buffer, " %7.3f", race.legData->targetSpeed*2.23694);
  display.drawStr(x+31,y, buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(x+97,y,"mph");	

  y=44;
  x=128;
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(x,y,"Dis:");	
  display.setFont(u8g2_font_spleen8x16_mf);	
  sprintf(buffer, "%7.3f", race.legData->distance*0.000621372);
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
  long int targetTime=((float)race.legData->totalDistance/race.legData->targetSpeed)*1000.0;
  long int legTime=((race.legData->endTs.seconds*1000)+race.legData->endTs.millis)-((race.legData->startTs.seconds*1000)+race.legData->startTs.millis);
  double adjustedAverageSpeed=(double)race.legData->totalDistance/((double)legTime/1000.0);
  long adjustedTimeDelta=legTime-targetTime;
  display.setFont(u8g2_font_spleen8x16_mf);	

  x=136;
  y=12;
  sprintf(buffer, "Leg Adjusted   D: %+7.3f", (float)(race.legData->totalDistance-race.legData->distance)*0.000621372);
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
  sprintf(buffer, "%7.3f", adjustedAverageSpeed*2.23694);
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
  sprintf(buffer, "%c%3d.%03u", sign, abs(adjustedTimeDelta/1000), abs(adjustedTimeDelta%1000));
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

  sprintf(buffer, "%+8.3f", (adjustedAverageSpeed-race.legData->targetSpeed)*2.23694);
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
  sprintf(buffer, " %7.3f", race.legData->targetSpeed*2.23694);
  display.drawStr(x+31,y, buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	
  display.drawStr(x+97,y,"mph");	

  y=44;
  x=128;
  display.setFont(u8g2_font_spleen8x16_mf);	
  display.drawStr(x,y,"Dis:");	
  display.setFont(u8g2_font_spleen8x16_mf);	
  sprintf(buffer, "%7.3f", race.legData->totalDistance*0.000621372);
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
  long int raceTime=(race.timeComplete.seconds*1000)+race.timeComplete.millis;
  long int targetTime=((double)race.distanceComplete/race.targetSpeed)*1000.0;
  double averageSpeed=(double)race.distanceComplete/((double)raceTime/1000.0);
  long int timeDelta=raceTime-targetTime;  
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
  sprintf(buffer, "   T: %+3d.%03u", timeDelta/1000, abs(timeDelta%1000));
  display.drawStr(0,60,buffer);
  display.drawTriangle(14,60, 23,45, 33,60);
  display.setDrawColor(0);
  display.drawTriangle(17,58, 23,48, 29,58);
  display.setDrawColor(1);
}
void displayRaceSummary2(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  long int raceTime=(race.timeComplete.seconds*1000)+race.timeComplete.millis;
  long int targetTime=((float)race.distanceComplete/race.targetSpeed)*1000.0;
  float averageSpeed=(float)race.distanceComplete/((float)raceTime/1000.0);
  long int timeDelta=raceTime-targetTime;  
  display.setFont(u8g2_font_spleen12x24_mf);
  sprintf(buffer, "Dist: %8.3f", race.distanceComplete*0.000621373);
  display.drawStr(0,20,buffer);
  sprintf(buffer, "  \xd8S: %8.3f", averageSpeed*2.23694);
  display.drawStr(0,40,buffer);
  sprintf(buffer, "   S: %+8.3f", (averageSpeed-race.targetSpeed)*2.23694);
  display.drawStr(0,60,buffer);
  display.drawTriangle(14,60, 23,45, 33,60);
  display.setDrawColor(0);
  display.drawTriangle(17,58, 23,48, 29,58);
  display.setDrawColor(1);
}

void displayPoint(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  char dir;
  int x;
  double distRemaining=0;
  if(race.legData->activePoint==race.activeLeg->points.end()) {
    display.setFont(u8g2_font_spleen16x32_mf);
    display.drawStr(24, 37, "No Point Data");
    return;
  }
  distRemaining=(*race.legData->activePoint)->distance-race.legData->distance;
  if((*race.legData->activePoint)->turnDir==0) {
    dir='L';
  } else {
    dir='R';
  }
  if((*race.legData->activePoint)->turn>0) {
    sprintf(buffer, "%d%c", (*race.legData->activePoint)->turn, dir );
  } else {
    sprintf(buffer, "!!");
  }
  if(distRemaining<3057) {
    display.setFont(u8g2_font_spleen16x32_mf);
    x=3+((64-display.getStrWidth(buffer))/2);
    display.drawStr(x, 25, buffer);
    sprintf(buffer, "%4.2f", distRemaining*0.000621372);
    display.drawStr(73, 25, buffer);
    if(gpsData.speed==0) {
      sprintf(buffer, "%0.2f", 0);
    } else {
      sprintf(buffer, "%5.2f", distRemaining/gpsData.speed);
    }
    display.drawStr(153, 25, buffer);   

    display.setFont(u8g2_font_spleen5x8_mf);
    display.drawStr(138, 25, "mi");
    display.drawStr(233, 25, "sec");

  }
  display.drawRFrame(0, 0, 70, 32, 5);
  display.drawRFrame(72, 0, 78, 32, 5);
  display.drawRFrame(152, 0, 99, 32, 5);
  if(distRemaining<804) {
    display.setFont(u8g2_font_spleen8x16_mf);
    display.drawStr(0,46, (*race.legData->activePoint)->descrLine1.c_str());
    display.drawStr(0,60, (*race.legData->activePoint)->descrLine2.c_str());
  }
}

void displayUpdateFast(void) {
  if(ledDispFunc!=NULL) {
    ledDispFunc();
  }
}

void ledDispLegTime(void) {
  timeStamp_t *ts=getTimeStamp();
  timeStamp dispTs;
  unsigned long millis=(ts->seconds*1000)+ts->millis;
  millis-=(race.legData->timerOffset.seconds*1000)+race.legData->timerOffset.millis;
  dispTs.seconds=millis/1000;
  dispTs.millis=millis%1000;
  if(!race.legData->inProgress) {
    return;
  }
  ledDisp.RefreshMe();
  sprintf(buffer, "%02ld.%02ld", dispTs.seconds/3600, (dispTs.seconds/60)%60);
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer);
  sprintf(buffer, "%02ld.%02ld", (dispTs.seconds/60)%60, dispTs.seconds%60);
  ledDisp.Set_Position(2);
  ledDisp.ShowMe(buffer); 
  sprintf(buffer, "%02ld.%02d", dispTs.seconds%60, dispTs.millis/10);
  ledDisp.Set_Position(4);
  ledDisp.ShowMe(buffer);     
}

void ledDispRaceTime(void) {
  timeStamp_t *ts=getTimeStamp();
  timeStamp dispTs;
  unsigned long millis=(ts->seconds*1000)+ts->millis;
  millis-=(race.legData->timerOffset.seconds*1000)+race.legData->timerOffset.millis;
  dispTs.seconds=millis/1000;
  dispTs.millis=millis%1000;
  if(!race.legData->inProgress) {
    return;
  }
  ledDisp.RefreshMe();
  sprintf(buffer, "%02ld.%02ld", dispTs.seconds/3600, (dispTs.seconds/60)%60);
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer);
  sprintf(buffer, "%02ld.%02ld", (dispTs.seconds/60)%60, dispTs.seconds%60);
  ledDisp.Set_Position(2);
  ledDisp.ShowMe(buffer); 
  sprintf(buffer, "%02ld.%02d", dispTs.seconds%60, dispTs.millis/10);
  ledDisp.Set_Position(4);
  ledDisp.ShowMe(buffer);     
}


void ledDispLegDeltaSpeed(void) {
  ledDisp.RefreshMe();
  sprintf(buffer, "%9.3f", race.legData->speedDelta*2.23694);
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer); 
}

void ledDispRaceDeltaSpeed(void) {
  ledDisp.RefreshMe();
  sprintf(buffer, "%9.3f", race.speedDelta*2.23694);
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer); 
}

void ledDispDashes(void) {
  ledDisp.begin();
  ledDisp.RefreshMe();
  ledDisp.Set_Position(0);
  ledDisp.ShowMe("--------");
}

void displayError(const char *err) {
  oledDisp4.clearBuffer();
  oledDisp4.setFont(u8g2_font_spleen12x24_mf);
  oledDisp4.drawStr(0,24,err);	
  oledDisp4.sendBuffer();
}