#include "bsp.h"


bool SPILock=false;

void bsp_setup(void) {

  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, HIGH);

  pinMode(GPS_CS, OUTPUT);
  digitalWrite(GPS_CS, HIGH);

  pinMode(LED_DISP_LOAD, OUTPUT);
  digitalWrite(LED_DISP_LOAD, HIGH);

  pinMode(OLED_DISP1_CS, OUTPUT);
  digitalWrite(OLED_DISP1_CS, HIGH);
  pinMode(OLED_DISP2_CS, OUTPUT);
  digitalWrite(OLED_DISP2_CS, HIGH);
  pinMode(OLED_DISP3_CS, OUTPUT);
  digitalWrite(OLED_DISP3_CS, HIGH);
  pinMode(OLED_DISP3_CS, OUTPUT);
  digitalWrite(OLED_DISP1_CS, HIGH);
}