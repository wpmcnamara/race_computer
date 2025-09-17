#include <SPI.h>
#include <Wire.h>
#include "bsp.h"
#include "helpers.h"
#include "keypad.h"
#include "display.h"
#include "gps.h"
#include "timer.h"
#include "storage.h"
#include <IntervalTimer.h>


void setup() {
  // put your setup code here, to run once:
  Serial.begin(1000000);
  Wire.begin();
  Wire.setClock(400000);
  keypadSetup();
  displaySetup();
  gpsSetup();
  storageSetup();
  timerSetup();
}

void loop() {
  struct gpsDataStruct *gpsData=getGpsData();
  if(gpsData->fix>=3 && !startStopIsBreathing() && !timer_run) {
    startStopStartBreath();
  }
  if(gpsData->fix<2 && startStopIsBreathing()) {
    startStopStopBreath();
  }
  
  if(timer_run) {
    if(((gpsData->avgSpeed*0.00223693629)-31.9)<-0.1) {
      keypad.pixels.setPixelColor(0,0xFF0000);
      keypad.pixels.setPixelColor(1,0xFF0000);
      keypad.pixels.setPixelColor(2,0xFF0000);
      keypad.pixels.setPixelColor(3,0xFF0000);
    } else if (((gpsData->avgSpeed*0.00223693629)-31.9)>0.1) {
      keypad.pixels.setPixelColor(0,0xFFFF00);
      keypad.pixels.setPixelColor(1,0xFFFF00);
      keypad.pixels.setPixelColor(2,0xFFFF00);
      keypad.pixels.setPixelColor(3,0xFFFF00);      
    } else {
      keypad.pixels.setPixelColor(0,0x00FF00);
      keypad.pixels.setPixelColor(1,0x00FF00);
      keypad.pixels.setPixelColor(2,0x00FF00);
      keypad.pixels.setPixelColor(3,0x00FF00);

    }
    keypad.pixels.show();
  } else {
      keypad.pixels.setPixelColor(0,0x000000);
      keypad.pixels.setPixelColor(1,0x000000);
      keypad.pixels.setPixelColor(2,0x000000);
      keypad.pixels.setPixelColor(3,0x000000);
      keypad.pixels.show();     
  }
  
}
