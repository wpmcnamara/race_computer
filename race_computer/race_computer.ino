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
#include "state_machine.h"


void setup() {
  // put your setup code here, to run once:
  Serial.begin(1000000);
  Wire.begin();
  Wire.setClock(400000);
  keypadSetup();
  displaySetup();

  storageSetup();
  gpsSetup();
  timerSetup();
  raceSetup();
}

void loop() {
  stateMachine.run();
}
