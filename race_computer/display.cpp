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

char buffer[256];

struct gpsDataStruct *gpsDataPtr;
orcTime_t *gpsTimePtr;

void displaySetup(void) {
  ledDisp.begin();
  ledDisp.Set_Brightness(3);
  // Turn Off the Auto Refresh
  ledDisp.AutoRefresh(false);
  ledDisp.RefreshMe();
  sprintf(buffer, "01234567");
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer);
  oledDisp1.begin();
  oledDisp2.begin();
  oledDisp3.begin();
  oledDisp4.begin();

  oledDisp1.clearBuffer();
  oledDisp1.setFont(u8g2_font_spleen32x64_mf);	// choose a suitable font
  sprintf(buffer, "Disp 1");
  oledDisp1.drawStr(0,53,buffer);	// write something to the internal memory  
  oledDisp1.sendBuffer();
  oledDisp2.clearBuffer();
  oledDisp2.setFont(u8g2_font_spleen32x64_mf);	// choose a suitable font
  sprintf(buffer, "Disp 2");
  oledDisp2.drawStr(0,53,buffer);	// write something to the internal memory  
  oledDisp2.sendBuffer();

  oledDisp3.clearBuffer();
  oledDisp3.setFont(u8g2_font_spleen32x64_mf);	// choose a suitable font
  sprintf(buffer, "Disp 3");
  oledDisp3.drawStr(0,53,buffer);	// write something to the internal memory  
  oledDisp3.sendBuffer();
  oledDisp4.clearBuffer();
  oledDisp4.setFont(u8g2_font_spleen32x64_mf);	// choose a suitable font
  sprintf(buffer, "Disp 4");
  oledDisp4.drawStr(0,53,buffer);	// write something to the internal memory  
  oledDisp4.sendBuffer();

  delay(1000);
  sprintf(buffer, "--------");
  ledDisp.ShowMe(buffer);
  ledDisp.Set_Position(0);
  oledDisp1.clearBuffer();
  oledDisp1.drawXBM(18,2,agr_top_width, agr_top_height, agr_logo_top);
  oledDisp1.sendBuffer();
  oledDisp2.clearBuffer();
  oledDisp2.drawXBM(18,0,agr_bottom_width, agr_bottom_height, agr_logo_bottom);
  oledDisp2.sendBuffer();
  oledDisp3.clearBuffer();
  oledDisp3.setFont(u8g2_font_spleen16x32_mf);	// choose a suitable font
  sprintf(buffer, "Open Race");
  oledDisp3.drawStr(55,20,buffer);	// write something to the internal memory
  sprintf(buffer, "Computer");
  oledDisp3.drawStr(63,52,buffer);	// write something to the internal memory
  oledDisp3.sendBuffer();
  oledDisp4.clearBuffer();
  oledDisp4.setFont(u8g2_font_spleen12x24_mf);
  sprintf(buffer, "%c Patrick McNamara",169);
  oledDisp4.drawStr(20,20,buffer);	// write something to the internal memory
  sprintf(buffer, "firmware: 0.0.1");
  oledDisp4.drawStr(38,52,buffer);	// write something to the internal memory
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
  displayGPSInfo(oledDisp1, dispNA, dispNA);
  oledDisp1.sendBuffer();					

  oledDisp2.clearBuffer();
  displayDeltaTimeMed(oledDisp2, dispNA, dispTop);
  displayDeltaSpeedMed(oledDisp2, dispNA, dispBottom);
  oledDisp2.sendBuffer();					

  oledDisp3.clearBuffer();
  displayDistRemainMed(oledDisp3, dispNA, dispTop);
  displayDistanceMed(oledDisp3, dispNA, dispBottom);
  oledDisp3.sendBuffer();

  oledDisp4.clearBuffer();
  displayGpsSpeedMed(oledDisp4, dispNA, dispTop);
  displayGpsAvgSpeedMed(oledDisp4, dispNA, dispBottom);
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
  if(gpsDataPtr->fix == 0) sprintf(buffer, "fix: none");
  else if(gpsDataPtr->fix == 1) sprintf(buffer, "fix: DR");
  else if(gpsDataPtr->fix == 2) sprintf(buffer, "fix: 2D");
  else if(gpsDataPtr->fix == 3) sprintf(buffer, "fix: 3D");
  else if(gpsDataPtr->fix == 4) sprintf(buffer, "fix: GNSS+DR");
  else if(gpsDataPtr->fix == 5) sprintf(buffer, "fix: Time");
  display.drawStr(128,56,buffer);	
}

void displayDeltaTimeMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  unsigned int y;
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
  sprintf(buffer, "   time  :");
  display.drawStr(0,y,buffer);
  display.drawLine(61,y,66,y-11);
  display.drawLine(62,y-1,66,y-10); 

  display.drawLine(66,y-11,71,y);
  display.drawLine(66,y-10,70,y-1);
 
  display.drawLine(61,y,71,y);
  display.drawLine(62,y-1,70,y-1);
  display.setFont(u8g2_font_spleen16x32_mf);	

  sprintf(buffer, "%9.3f", race.legData->timeDelta);
  oledDisp2.drawStr(81,y, buffer);
  display.setFont(u8g2_font_spleen6x12_mf);	
  sprintf(buffer, "mph");
  display.drawStr(225,y,buffer);	
}

void displayDeltaSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  unsigned int y;
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
  sprintf(buffer, "  speed  :");
  display.drawStr(0,y,buffer);
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
  sprintf(buffer, "sec");
  display.drawStr(225,y,buffer);  
}

void displayDistanceMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  unsigned int y;
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
  sprintf(buffer, " distance:");
  display.drawStr(0,y,buffer);	
  display.setFont(u8g2_font_spleen16x32_mf);	
  sprintf(buffer, "%8.3f", race.legData->distance*0.000621372);
  display.drawStr(97,y,buffer);	 
  display.setFont(u8g2_font_spleen6x12_mf);	
  sprintf(buffer, "miles");
  display.drawStr(225,y,buffer);	
}

void displayDistRemainMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  unsigned int y;
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
  sprintf(buffer, " dist rem:");
  display.drawStr(0,y,buffer);	
  display.setFont(u8g2_font_spleen16x32_mf);	
  sprintf(buffer, "%8.3f", race.legData->distanceRemaining*0.000621372);
  display.drawStr(97,y,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);	
  sprintf(buffer, "miles");
  display.drawStr(225,y,buffer);	
}

void displayGpsSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  unsigned int y;
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
  sprintf(buffer, "    speed:");
  oledDisp4.drawStr(0,y,buffer);	 
  display.setFont(u8g2_font_spleen16x32_mf);	
  sprintf(buffer, "%8.3f", gpsDataPtr->speed*2.23694);
  display.drawStr(97,y,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);	
  sprintf(buffer, "mph");
  display.drawStr(225,y,buffer);	
}

void displayGpsAvgSpeedMed(U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI &display, dispPos_t posX, dispPos_t posY) {
  unsigned int y;
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
  sprintf(buffer, "avg speed:");
  display.drawStr(0,y,buffer);	
  display.setFont(u8g2_font_spleen16x32_mf);	
  sprintf(buffer, "%8.3f", race.legData->averageSpeed*2.23694);
  display.drawStr(97,y,buffer);	
  display.setFont(u8g2_font_spleen6x12_mf);	
  sprintf(buffer, "mph");
  display.drawStr(225,y,buffer);	
}

void displayUpdateFast(void) {
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