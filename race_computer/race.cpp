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
  race.activeRace->inProgress=true;
  race.legData->delayedStart=false;
  stateMachine.status.flags.delayedStart=false;
  race.legData->distanceOffset=gpsData.distance;
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
    //do these before we close the file, or entry.name() return empty.
    raceFile=new race_t;
    raceFile->fileName=entry.name();
    entry.close();
    Serial.printf("race filename: %s\n", raceFile->fileName.c_str());
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
  race.timeComplete.seconds=0;
  race.timeComplete.millis=0;
  race.startTs.seconds=0;
  race.startTs.millis=0;
  race.endTs.seconds=0;
  race.endTs.millis=0;
}

void prepRace(void) {
  race.legData->targetSpeed=(race.activeLeg->speed)/2.23694;
  race.legData->totalDistance=(race.activeLeg->distance)/0.000621372;
  race.legData->distanceRemaining=race.legData->totalDistance;
  race.legData->startMark=race.activeLeg->mark;
  
  race.legData->distance=0;
  race.legData->distanceComplete=0;
  race.legData->distanceOffset=0;
  race.legData->averageSpeed=0;
  race.legData->speedDelta=0;
  race.legData->timeDelta=0;
  race.legData->timeComplete.seconds=0;
  race.legData->timeComplete.millis=0;
  race.legData->activeRace=NULL;
  race.legData->activeLeg=NULL;
  race.legData->inProgress=false;
  race.legData->startTs.seconds=0;
  race.legData->startTs.millis=0;
  race.legData->endTs.seconds=0;
  race.legData->endTs.millis=0;  
}

void updateRace(void) {
  //we convert the timestamps to a fixed point integer representation to do math.  The macros to convert
  //to floats trigger some sort of bug where the seconds field is the same for the start and end.  Two
  //hours of trying to figure out what was going on, with no success, and we solve the problem a different
  //way.
  uint32_t startTs=(race.legData->startTs.seconds*1000)+race.legData->startTs.millis;
  uint32_t endTs=(race.legData->endTs.seconds*1000)+race.legData->endTs.millis;
  uint32_t timeComplete=(race.timeComplete.seconds*1000)+race.timeComplete.millis;
  uint32_t elapsedRaceTime=timeComplete+(endTs-startTs);
  race.activeLeg->complete=true;
  race.distanceComplete+=race.legData->totalDistance;
  race.timeComplete.seconds=elapsedRaceTime/1000;
  race.timeComplete.millis=elapsedRaceTime%1000;
  //Serial.println("Current race");
  //dumpRaceData(&race);
  //Serial.println("\n\nCurrent leg");
  //dumpRaceData(race.legData);
  //Serial.println("\n");
}

void dumpRaceData(raceData_t *data) {
  Serial.println("Race Data Dump");
  Serial.printf("  targetSpeed=%f m/s\n", data->targetSpeed);
  Serial.printf("  averageSpeed=%f m/s\n", data->averageSpeed);
  Serial.printf("  speedDelta=%f m/s\n", data->speedDelta);
  Serial.printf("  speedTargetBand=%f m/s\n", data->speedTargetBand);
  Serial.printf("  totalDistance=%d m\n", data->totalDistance);
  Serial.printf("  distanceComplete=%d m\n", data->distanceComplete);
  Serial.printf("  timeComplete sec=%d, millis=%d\n", data->timeComplete.seconds, data->timeComplete.millis);
  Serial.printf("  distance=%d m\n", data->distance);
  Serial.printf("  distanceRemaining=%d m\n", data->distanceRemaining);
  Serial.printf("  distanceOffset=%d m\n", data->distanceOffset);
  Serial.printf("  startTs sec=%d, millis=%d\n", data->startTs.seconds, data->startTs.millis);
  Serial.printf("  endTs sec=%d, millis=%d\n", data->endTs.seconds, data->endTs.millis);
  Serial.printf("  startMark=%d s\n", data->startMark);
  Serial.printf("  delayedStart=%d\n", data->delayedStart);
  Serial.printf("  timerOffset sec=%d, millis=%d\n", data->timerOffset.seconds, data->timerOffset.millis);
  Serial.printf("  timeDelta=%f s\n", data->timeDelta);
  Serial.printf("  inProgess=%d\n", data->inProgress);
  Serial.printf("  legData=%#x\n", data->legData);
  Serial.printf("  activeRace=%#x\n", data->activeRace);
  Serial.printf("  activeLeg=%#x\n", data->activeLeg);
}

void raceCheckPoint(void) {
  JsonDocument doc;
  File checkPoint;
  if(!SD.exists("orc")) {
    return;
  }
  if(SD.exists("orc/race.dat")) {
    Serial.println("clearing race checkpoint");
    SD.remove("orc/race.dat");
  }
  if(race.inProgress!=true) {
    return;
  }
  Serial.println("Dumping race checkpoint");
  doc["raceFile"]=race.activeRace->fileName;
  doc["raceInProgress"]=race.activeRace->inProgress;
  doc["legId"]=race.activeLeg->id;
  doc["legInProgress"]=race.activeLeg->inProgress;
  doc["legComplete"]=race.activeLeg->complete;
  doc["distanceComplete"]=race.distanceComplete;
  doc["timeSec"]=race.timeComplete.seconds;
  doc["timeMilli"]=race.timeComplete.millis;
  checkPoint=SD.open("orc/race.dat", FILE_WRITE);
  if(!checkPoint) {
    Serial.println("Checkpoint open failed");

  }
  serializeJsonPretty(doc, checkPoint);
  serializeJsonPretty(doc, Serial);
  Serial.println();
  checkPoint.close();

}

void loadRaceCheckPoint(void) {
  std::list<race_t *>::iterator raceIt;
  std::vector<raceLeg_t *>::iterator raceLegIt;
  int raceLegId;
  JsonDocument doc;
  File checkPoint;
  DeserializationError error;
  if(!SD.exists("orc/race.dat")) {
    return;
  }  
  Serial.println("Found race checkpoint");
  checkPoint=SD.open("orc/race.dat");
  error=deserializeJson(doc, checkPoint);
  checkPoint.close();
  if (error) {
    Serial.println("deserialization error");
    Serial.println(error.c_str());
    return;
  }
  raceIt=races.begin();
  while(raceIt!=races.end()) {
    if((*raceIt)->fileName==doc["raceFile"]) {
      Serial.print("Race checkpoint for: ");
      Serial.println(doc["raceFile"].as<String>());
      break;
    }
    ++raceIt;
  }
  if(raceIt==races.end()) {
    Serial.println("Could not find race definition for checkpoint");
    return;
  }
  raceLegId=doc["legId"].as<int>();
  Serial.printf("legId=%d\n", raceLegId);
  if(doc["legInProgess"].as<bool>()==false && doc["legComplete"].as<bool>()==true) {
    raceLegIt=(*raceIt)->raceLegs.begin();
    //This is really bad.  It takes advantage of leg indexs being 1 based and the
    //iterator indexes being 0 based.  Need to fix.
    raceLegIt+=raceLegId;
    if(raceLegIt==(*raceIt)->raceLegs.end()) {
      Serial.printf("leg id %d doesn't exist\n", raceLegId);
      return;
    } else {
      Serial.print("Race leg: ");
      Serial.println((*raceLegIt)->descr);
    }
  } else {
    return;
  }
  Serial.println("Setting race checkpoint");
  setRace((*raceIt), (*raceLegIt));
  race.activeRace->inProgress=doc["raceInProgress"].as<bool>();
  race.inProgress=race.activeRace->inProgress;
  race.distanceComplete=doc["distanceComplete"].as<int>();
  race.timeComplete.seconds=doc["timeSec"].as<int>();
  race.timeComplete.millis=doc["timeMilli"].as<int>(); 
  selectedRace=raceIt;
  selectedRaceLeg=raceLegIt;
}
