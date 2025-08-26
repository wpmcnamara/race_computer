#include <SPI.h>
#include <Wire.h>
#include "bsp.h"
#include "helpers.h"
#include "keypad.h"
#include "display.h"
#include "gps.h"
#include "timer.h"
#include "storage.h"




void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000);
  keypadSetup();
  displaySetup();
  gpsSetup();
  storageSetup();
  timerSetup();
}

void loop() {
  static int loops=0;

  if (!refresh) {
    return;
  }
  refresh=false;
  loops++;

  timerUpdate();
  keypadUpdate();
  if(loops==10) {
    gpsUpdate();
    displayUpdate();
  }
  displayUpdateFast();
  if(loops%2==0) {
    readKeypad();
  }
  if(loops%2==0) {
    readKeypad();
  }
  if(loops==10) { 
    loops=0;
  }
}
