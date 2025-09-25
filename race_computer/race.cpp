#include "race.h"
#include "event.h"
#include "gps.h"
#include "keypad.h"

raceData_t race;
event_t *delayedStartEvent;

void raceSetup(void) {
  race.activeRace=new race_t;
  race.activeRace->distance=10299.8;
  race.activeRace->speed=14.26058;
  race.activeLeg=new raceLeg_t;
  race.activeLeg->distance=10299.8;
  race.activeLeg->speed=14.26058;
  race.activeLeg=NULL;
  
  race.legData=new raceData_t;
  race.averageSpeed=0;
  race.targetSpeed=14.26058;
  race.speedTargetBand=0.044704;
  race.totalDistance=10299.8;
  race.distanceComplete=0;
  race.distance=0;
  race.distanceRemaining=10299.8;
  race.distanceOffset=0;
  race.averageSpeed=0;
  race.speedDelta=0;
  race.timeDelta=0;
  race.startTs.seconds=0;
  race.startTs.millis=0;
  race.endTs.seconds=0;
  race.endTs.millis=0;
  race.startMark=0;
  race.delayedStart=false;
  race.inProgress=false;


  race.legData->averageSpeed=0;
  race.legData->targetSpeed=14.26058;  //31.9mph
  race.legData->speedTargetBand=0.044704;  //0.1mph
  race.legData->totalDistance=10299.8; //6.4miles
  race.legData->distance=0;
  race.legData->distanceRemaining=10299.8;
  race.legData->distanceComplete=0;
  race.legData->distanceOffset=0;
  race.legData->averageSpeed=0;
  race.legData->speedDelta=0;
  race.legData->activeRace=race.activeRace;
  race.legData->activeLeg=race.activeLeg;
  race.legData->inProgress=false;
  race.legData->startTs.seconds=0;
  race.legData->startTs.millis=0;
  race.legData->endTs.seconds=0;
  race.legData->endTs.millis=0;
  race.legData->startMark=5;
  race.legData->delayedStart=false;
  race.delayedStart=true;
  race.legData->timeDelta=0;

  delayedStartEvent=new event_t(raceLegStart, eventSingle, false, false, 0, 0, &Serial, "delayedStartEvent");

}

void raceLegStart(void) {
  race.legData->inProgress=true;
  race.legData->delayedStart=false;
  race.legData->distanceOffset=gpsData.distance;
  if(!race.inProgress) {
    race.inProgress=true;
  }
  startStopStopBlink();
  Serial.printf("start clock:  %02d:%02d:%02d.%03d\n", gpsData.gpsTime.hour, gpsData.gpsTime.minute, gpsData.gpsTime.second, gpsData.gpsTime.millis);
  //startPressInt();
}

void raceLegStop() {
  race.legData->inProgress=false;
}