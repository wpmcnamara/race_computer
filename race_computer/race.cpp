#include "race.h"
#include "event.h"
#include "gps.h"
#include "keypad.h"
#include "storage.h"
#include "state_machine.h"
#include <ArduinoJson.h>

raceData_t race;
event_t *delayedStartEvent;
std::list<race_t *> races; 
std::list<race_t *>::iterator selectedRace;
std::list<race_t *>::iterator selectedRaceSave;
std::vector<raceLeg_t *>::iterator selectedRaceLeg;

void raceSetup(void) {

  /*
  race.activeRace=new race_t;
  race.activeRace->distance=10299.8;
  race.activeRace->speed=14.26058;
  race.activeLeg=new raceLeg_t;
  race.activeLeg->distance=10299.8;
  race.activeLeg->speed=14.26058;
  race.activeLeg=NULL;
  */
  race.activeRace=NULL;
  race.activeLeg=NULL;
  race.legData=new raceData_t;
  race.averageSpeed=0;
  //race.targetSpeed=14.26058;
  race.speedTargetBand=0.044704;
  //race.totalDistance=10299.8;
  race.distanceComplete=0;
  race.distance=0;
  //race.distanceRemaining=10299.8;
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
  //race.legData->targetSpeed=14.26058;  //31.9mph
  race.legData->speedTargetBand=0.044704;  //0.1mph
  //race.legData->totalDistance=10299.8; //6.4miles
  race.legData->distance=0;
  //race.legData->distanceRemaining=10299.8;
  race.legData->distanceComplete=0;
  race.legData->distanceOffset=0;
  race.legData->averageSpeed=0;
  race.legData->speedDelta=0;
  race.legData->activeRace=NULL;
  race.legData->activeLeg=NULL;
  race.legData->inProgress=false;
  race.legData->startTs.seconds=0;
  race.legData->startTs.millis=0;
  race.legData->endTs.seconds=0;
  race.legData->endTs.millis=0;
  //race.legData->startMark=5;
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
  File entry;
  JsonDocument doc;
  DeserializationError error;
  race_t *raceFile;
  raceLeg_t *raceLegFile;

  //race data files will be stored in a directory called "orc"
  //if it doesn't exist, then we've got races to load.
  if(!SD.exists("orc")) {
    return;
  }
  File orcDir=SD.open("orc");
  //process the orc directory.  Anything that ends in .csv will be considered a race file.
  while (true) {
    entry =  orcDir.openNextFile();
    if (! entry) {
      break;
    }
    //We are looking for files right now, so skip directories.
    if(entry.isDirectory()) {
      continue;
    }
    if(strstr(entry.name(), ".jsn")==NULL) {
      continue;
    }
    Serial.printf("Found race file: %s\n", entry.name());
    error=deserializeJson(doc, entry);
    if (error) {
      Serial.println("deserialization error");
      entry.close();
      continue;
    }
    entry.close();
    raceFile=new race_t;
    raceFile->descr=doc["descr"].as<String>();
    Serial.printf("race descr: %s\n", raceFile->descr.c_str());
    raceFile->distance=doc["distance"].as<float>();
    Serial.printf("race distance: %f\n", raceFile->distance);    
    raceFile->speed=doc["speed"].as<float>();
    Serial.printf("race speed: %f\n", raceFile->speed);    
    raceFile->mark=doc["tmark"].as<int>();
    Serial.printf("race mark: %d\n\n", raceFile->mark);    
    raceFile->inProgress=false;
    races.push_back(raceFile);
    for (JsonObject jsonLeg : doc["legs"].as<JsonArray>()) {
      raceLegFile=new raceLeg_t;
      raceLegFile->descr=jsonLeg["descr"].as<String>();
      Serial.printf("   leg descr: %s\n", raceLegFile->descr.c_str());
      raceLegFile->id=jsonLeg["id"].as<int>();
      Serial.printf("   leg id: %d\n", raceLegFile->id);
      raceLegFile->speed=jsonLeg["speed"].as<float>();
      Serial.printf("   leg speed: %f\n", raceLegFile->speed);
      raceLegFile->distance=jsonLeg["distance"].as<float>();
      Serial.printf("   leg distance: %f\n\n\n", raceLegFile->distance);
      if(jsonLeg["tmark"].isNull()) {
        raceLegFile->mark=raceFile->mark;
        Serial.printf("   leg mark(from race): %f\n\n\n", raceLegFile->mark);
      } else{
        raceLegFile->mark=jsonLeg["tmark"].as<int>();
        Serial.printf("   leg mark: %f\n\n\n", raceLegFile->mark);
      }
      raceLegFile->inProgress=false;
      raceLegFile->complete=false;
      raceFile->raceLegs.push_back(raceLegFile);
      Serial.println("next leg");
    } 
  }
  selectedRace=races.begin();
  selectedRaceLeg=(*selectedRace)->raceLegs.begin();
  Serial.println((*selectedRace)->descr);
}

void setRace(race_t *selectedRace, raceLeg_t *selectedRaceLeg) {
  race.activeRace=selectedRace;
  race.activeLeg=selectedRaceLeg;
  race.targetSpeed=(selectedRace->speed)/2.23694;
  race.totalDistance=(selectedRace->distance)/0.000621372;
  race.distanceRemaining=race.totalDistance;
  race.startMark=selectedRace->mark;
  
  race.averageSpeed=0;
  race.distanceComplete=0;
  race.distance=0;
  race.distanceOffset=0;
  race.averageSpeed=0;
  race.speedDelta=0;
  race.timeDelta=0;  
  race.startTs.seconds=0;
  race.startTs.millis=0;
  race.endTs.seconds=0;
  race.endTs.millis=0;
}

void prepRace(void) {
  race.legData->targetSpeed=(race.activeLeg->speed)/2.23694;
  race.legData->distance=0;
  race.legData->totalDistance=(race.activeLeg->distance)/0.000621372;
  race.legData->distanceRemaining=race.legData->totalDistance;
  race.legData->startMark=race.activeLeg->mark;
  
  race.legData->distanceComplete=0;
  race.legData->distanceOffset=0;
  race.legData->averageSpeed=0;
  race.legData->speedDelta=0;
  race.legData->timeDelta=0;
  race.legData->activeRace=NULL;
  race.legData->activeLeg=NULL;
  race.legData->inProgress=false;
  race.legData->startTs.seconds=0;
  race.legData->startTs.millis=0;
  race.legData->endTs.seconds=0;
  race.legData->endTs.millis=0;  
}

void updateRace(void) {
  race.activeLeg->complete=true;
  race.distanceComplete=race.distance;
}

