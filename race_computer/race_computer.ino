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
#include "race.h"


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
  raceSetup();
}

void loop() {
  struct gpsDataStruct *gpsData=getGpsData();
  if(gpsData->fix>=3 && !startStopIsBreathing() && !race.legData->inProgress && !race.legData->delayedStart) {
    startStopStartBreath();
  }
  if(gpsData->fix>=3 && !startStopIsBlinking() && race.legData->delayedStart) {
    startStopStopBreath();
    startStopStartBlink();
  }
  if(gpsData->fix<2) {
    if(startStopIsBreathing()) {
      startStopStopBreath();
    }
    if(startStopIsBlinking()) {
      startStopStopBlink();
    }
  }
  
  if(race.legData->inProgress) {
    if((race.legData->speedDelta)<(race.legData->speedTargetBand*-1.0)) {
      keypad.pixels.setPixelColor(0,0xFF0000);
      keypad.pixels.setPixelColor(1,0xFF0000);
      keypad.pixels.setPixelColor(2,0xFF0000);
      keypad.pixels.setPixelColor(3,0xFF0000);
    } else if ((race.legData->speedDelta)>race.legData->speedTargetBand) {
      keypad.pixels.setPixelColor(0,0x0000FF);
      keypad.pixels.setPixelColor(1,0x0000FF);
      keypad.pixels.setPixelColor(2,0x0000FF);
      keypad.pixels.setPixelColor(3,0x0000FF);      
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
