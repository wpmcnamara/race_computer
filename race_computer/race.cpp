#include "race.h"
#include "event.h"
#include "gps.h"
#include "keypad.h"
#include  "storage.h"
#include "state_machine.h"

raceData_t race;
event_t *delayedStartEvent;
std::list<race_t *> races; 

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
  race.legData->timeDelta=0;

  delayedStartEvent=new event_t(raceLegStart, eventSingle, false, false, 0, 0, &Serial, "delayedStartEvent");
  loadRaces();
}

void raceLegStart(void) {
  race.legData->inProgress=true;
  race.legData->delayedStart=false;
  stateMachine.status.flags.delayedStart=false;
  race.distanceOffset=gpsData.distance;
  if(!race.inProgress) {
    race.inProgress=true;
  }
  stateMachine.status.flags.legActive=true;
}

void raceLegStop() {
  race.legData->inProgress=false;
  stateMachine.status.flags.legActive=false;
}

void loadRaces() {
  //race data files will be stored in a directory called "orc"
  //if it doesn't exist, then we've got races to load.
  if(!SD.exists("orc")) {
    return;
  }
  File orcDir=SD.open("orc");
  //process the orc directory.  Anything that ends in .csv will be considered a race file.
  while (true) {
    File entry =  orcDir.openNextFile();
    if (! entry) {
      return;
    }
    //We are looking for files right now, so skip directories.
    if(entry.isDirectory()) {
      continue;
    }
    if(strstr(entry.name(), ".csv")==NULL) {
      continue;
    }
    Serial.printf("Found race file: %s\n", entry.name());
    Serial.println("Contents:");
    while(entry.available()) {
      Serial.write(entry.read());
    }
    entry.close();
  }
}

    


