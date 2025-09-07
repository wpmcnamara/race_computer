#include "display.h"
#include "timer.h"
#include "gps.h"

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

}

void displayUpdate() {
  unsigned long tickHold=getTick();
  int cntHold=getCnt();
  struct gpsDataStruct *gpsData=getGpsData();
  UBX_TIM_TM2_data_t *gpsTimeStamp=getGpsTimestamp();
  double time=getTime();
  oledDisp1.clearBuffer();					// clear the internal memory
  oledDisp1.setFont(u8g2_font_spleen16x32_mf);	// choose a suitable font
  sprintf(buffer, "%02d:%02d:%02d", gpsData->hour, gpsData->minute, gpsData->second);
  oledDisp1.drawStr(64,20,buffer);	// write something to the internal memory
  oledDisp1.setFont(u8g2_font_spleen6x12_mf);
  oledDisp1.drawStr(194,20,"GMT");
  sprintf(buffer, "lattitude: %f\xB0", gpsData->lat);
  oledDisp1.drawStr(0,32,buffer);	// write something to the internal memory
  sprintf(buffer, "longitude: %f\xB0", gpsData->lon);
  oledDisp1.drawStr(0,44,buffer);	// write something to the internal memory
  sprintf(buffer, "satellites: %2d", gpsData->siv);
  oledDisp1.drawStr(0,56,buffer);	// write something to the internal memory
  if(gpsData->fix == 0) sprintf(buffer, "fix: none");
  else if(gpsData->fix == 1) sprintf(buffer, "fix: DR");
  else if(gpsData->fix == 2) sprintf(buffer, "fix: 2D");
  else if(gpsData->fix == 3) sprintf(buffer, "fix: 3D");
  else if(gpsData->fix == 4) sprintf(buffer, "fix: GNSS+DR");
  else if(gpsData->fix == 5) sprintf(buffer, "fix: Time");
  oledDisp1.drawStr(128,56,buffer);	// write something to the internal memory
  oledDisp1.sendBuffer();					// transfer internal memory to the display

  oledDisp2.clearBuffer();
  /*
  oledDisp2.setFont(u8g2_font_spleen6x12_mf);	// choose a suitable font
  sprintf(buffer, "speed: %09.4f", gpsData->speed*0.00223693629);
  oledDisp2.drawStr(0,12,buffer);	// write something to the internal memory  

  sprintf(buffer, "avg speed: %09.4f", gpsData->avgSpeed*0.00223693629);
  oledDisp2.drawStr(102,12,buffer);	// write something to the internal memory  
  sprintf(buffer, "%08ld.%1d", tickHold, cntHold/100);
  oledDisp2.drawStr(0,24,buffer);	// write something to the internal memory

  sprintf(buffer, "rcount=%d", gpsTimeStamp->count);  
  oledDisp2.drawStr(0,36,buffer);
  sprintf(buffer, "m:%d r:%d fe:%d tb:%d u:%d t:%d re:%d",
    gpsTimeStamp->flags.bits.mode,
    gpsTimeStamp->flags.bits.run,
    gpsTimeStamp->flags.bits.newFallingEdge,
    gpsTimeStamp->flags.bits.timeBase,
    gpsTimeStamp->flags.bits.utc,
    gpsTimeStamp->flags.bits.time,
    gpsTimeStamp->flags.bits.newRisingEdge
  );
  oledDisp2.drawStr(0,48,buffer);
  */  
  oledDisp2.setFont(u8g2_font_spleen8x16_mf);
  sprintf(buffer, " time  :");
  oledDisp2.drawStr(37,26,buffer);
  oledDisp2.drawLine(83,26,88,15);
  oledDisp2.drawLine(84,25,88,16); 

  oledDisp2.drawLine(88,15,93,26);
  oledDisp2.drawLine(88,16,92,25);
 
  oledDisp2.drawLine(83,26,93,26);
  oledDisp2.drawLine(84,25,92,25);

  sprintf(buffer, "speed  :");
  oledDisp2.drawStr(37,58,buffer);
  oledDisp2.drawLine(83,58,88,47);
  oledDisp2.drawLine(84,57,88,48);

  oledDisp2.drawLine(88,47,93,58);
  oledDisp2.drawLine(88,48,92,57);

  oledDisp2.drawLine(83,58,93,58);
  oledDisp2.drawLine(84,57,92,57);

  oledDisp2.setFont(u8g2_font_spleen16x32_mf);	// choose a suitable font

  //gps averqge speed is in meters per second.  Convert to miles per second
  double avgMps=gpsData->avgSpeed*0.000621371;
  //calculate our target speed in miles per second
  double targetMps=31.9/3600.0;
  //ticks are milliseconds, convert to seconds.  Multiply target speed by running time
  //to figure out how far we should have gone.
  double targetDistance=targetMps*time;
  //gps distance is in meters.  Convert to miles traveled.  
  //figure out the difference between how far we've actually gone and how
  //far we should have gone.
  double deltaDistance=(gpsData->distance*0.000621371)-targetDistance;
  //how long would it take to cover the distance delta at our target speed?
  double deltaTime=deltaDistance/targetMps;


  sprintf(buffer, "%8.3f", deltaTime);
  oledDisp2.drawStr(128,26, buffer);
  sprintf(buffer, "%8.3f", (gpsData->avgSpeed*0.00223693629)-31.9);
  oledDisp2.drawStr(128,58,buffer);
  oledDisp2.sendBuffer();					// transfer internal memory to the display

  oledDisp3.clearBuffer();
  oledDisp3.setFont(u8g2_font_spleen8x16_mf);	// choose a suitable font
  sprintf(buffer, "dist rem: ");
  oledDisp3.drawStr(17,26,buffer);	// write something to the internal memory  
  sprintf(buffer, "distance: ");
  oledDisp3.drawStr(17,58,buffer);	// write something to the internal memory   
  oledDisp3.setFont(u8g2_font_spleen16x32_mf);	// choose a suitable font
  sprintf(buffer, "%8.3f", 6.4-(gpsData->distance*0.000621371));
  oledDisp3.drawStr(97,26,buffer);	// write something to the internal memory  
  sprintf(buffer, "%8.3f", gpsData->distance*0.000621371);
  oledDisp3.drawStr(97,58,buffer);	// write something to the internal memory  
  oledDisp3.setFont(u8g2_font_spleen6x12_mf);	// choose a suitable font
  sprintf(buffer, "miles");
  oledDisp3.drawStr(225,58,buffer);	// write something to the internal memory  1
  oledDisp3.sendBuffer();

  oledDisp4.clearBuffer();
  oledDisp4.setFont(u8g2_font_spleen8x16_mf);	// choose a suitable font
  sprintf(buffer, "speed: ");
  oledDisp4.drawStr(69,26,buffer);	// write something to the internal memory  
  sprintf(buffer, "avg speed: ");
  oledDisp4.drawStr(37,58,buffer);	// write something to the internal memory  

  oledDisp4.setFont(u8g2_font_spleen16x32_mf);	// choose a suitable font
  sprintf(buffer, "%7.3f", gpsData->speed*0.00223693629);
  oledDisp4.drawStr(125,26,buffer);	// write something to the internal memory  

  sprintf(buffer, "%7.3f", gpsData->avgSpeed*0.00223693629);
  oledDisp4.drawStr(125,58,buffer);	// write something to the internal memory  

  oledDisp4.setFont(u8g2_font_spleen6x12_mf);	// choose a suitable font
  sprintf(buffer, "mph");
  oledDisp4.drawStr(237,58,buffer);	// write something to the internal memory  1
  oledDisp4.drawStr(237,26,buffer);	// write something to the internal memory  1
  oledDisp4.sendBuffer();
}

void displayUpdateFast(void) {
  unsigned long tickHold=getTick();
  int cntHold=getCnt();
 
  ledDisp.RefreshMe();
  sprintf(buffer, "%02ld.%02ld", tickHold/3600, (tickHold/60)%60);
  ledDisp.Set_Position(0);
  ledDisp.ShowMe(buffer);
  sprintf(buffer, "%02ld.%02ld", (tickHold/60)%60, tickHold%60);
  ledDisp.Set_Position(2);
  ledDisp.ShowMe(buffer); 
  sprintf(buffer, "%02ld.%02d", tickHold%60, cntHold/10);
  ledDisp.Set_Position(4);
  ledDisp.ShowMe(buffer);   
}