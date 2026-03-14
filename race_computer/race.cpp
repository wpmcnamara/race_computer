#include "race.h"
#include "event.h"
#include "gps.h"
#include "keypad.h"
#include "storage.h"
#include "state_machine.h"
#include "display.h"
#include <ArduinoJson.h>
#include <CSV_Parser.h>

raceData_t race;
raceData_t tmpLeg;
event_t *delayedStartEvent;
std::list<race_t *> races; 
std::list<race_t *>::iterator selectedRace;
std::list<race_t *>::iterator selectedRaceSave;
std::vector<raceLeg_t *>::iterator selectedRaceLeg;

void raceSetup(void) {

  race.activeRace=NULL;
  race.activeLeg=NULL;
  race.legData=new raceData_t;
  race.averageSpeed=0;
  race.speedTargetBand=0.0;
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
  race.startMark=0;
  race.delayedStart=false;
  race.inProgress=false;


  race.legData->averageSpeed=0;
  race.legData->speedTargetBand=0.0;  
  race.legData->distance=0;
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
  File32 entry;
  JsonDocument doc;
  YAMLNode yamlnode;
  DeserializationError error;
  race_t *raceFile;
  raceLeg_t *raceLegFile;
  char fileName[256];
  bool noRaceFound=true;
  unsigned char *buffer;
  uint32_t len;
  //see if we have an SD card or not.  If we don't we are going to fall back to a hardcoded set of
  //races and legs.
  if(!sdCardPresent) {
    Serial.println("No SD card.  Loading default race definitions");
    loadDefaultRaces();
    return;
  }
  //disable display and GPS use of the SPI bus to prevent collisions
  doSPILock();
  //race data files will be stored in a directory called "orc"
  //if it doesn't exist, then we've got races to load.
  if(!sdCard.exists("orrc")) {
    Serial.println("Data directory not found.  Loading default race definitions.");
    doSPIUnlock();
    loadDefaultRaces();
    return;
  }
  File32 orcDir=sdCard.open("orrc/races");
  doSPIUnlock();
  //process the orc directory.  Anything that ends in .csv will be considered a race file.
  while (true) {
    doSPILock();
    entry =  orcDir.openNextFile();
    if (! entry) {
      break;
    }
    //We are looking for files right now, so skip directories.
    if(entry.isDirectory()) {
      doSPIUnlock();
      continue;
    }
    entry.getName(fileName,256);
    if(strstr(fileName, ".yml")==NULL) {
      doSPIUnlock();
      continue;
    }
    noRaceFound=false;
    Serial.printf("Found race file: %s\n", fileName);
    len=entry.fileSize();
    buffer=(unsigned char *)malloc(len+1);
    if(buffer==0) {
      Serial.println("malloc failed!");
      while(1);
    }
    len=entry.read(buffer, len);
    buffer[len]=0;
    raceFile=new race_t;
    entry.getName(fileName, 256);
    raceFile->fileName=fileName;
    entry.close();
    doSPIUnlock();
    error=deserializeYml(doc,(const char *) buffer);
    free(buffer);
    if (error) {
      Serial.println("deserialization error");
      delete raceFile;
      continue;
    }
    Serial.printf("race filename: %s\n", raceFile->fileName.c_str());
    raceFile->descr=doc["descr"].as<String>();
    Serial.printf("race descr: %s\n", raceFile->descr.c_str());
    raceFile->distance=doc["distance"].as<float>();
    Serial.printf("race distance: %f\n", raceFile->distance);       
    raceFile->speed=doc["speed"].as<float>();
    Serial.printf("race speed: %f\n", raceFile->speed);  
    raceFile->speedRange=doc["speed_range"].as<float>();
    Serial.printf("race speed range: %f\n", raceFile->speedRange);
    raceFile->mark=doc["tmark"].as<int>();
    Serial.printf("race mark: %d\n\n", raceFile->mark);    
    raceFile->inProgress=false;
    races.push_back(raceFile);
    for (JsonObject jsonLeg : doc["legs"].as<JsonArray>()) {
      raceLegFile=new raceLeg_t;
      raceLegFile->descr=jsonLeg["descr"].as<String>();
      Serial.printf("   leg descr: %s\n", raceLegFile->descr.c_str());
      raceLegFile->pointsFile=jsonLeg["points"].as<String>();
      Serial.printf("   leg pointsFile: %s\n", raceLegFile->pointsFile.c_str());
      raceLegFile->id=jsonLeg["id"].as<int>();
      Serial.printf("   leg id: %d\n", raceLegFile->id);
      raceLegFile->speed=jsonLeg["speed"].as<float>();
      if(jsonLeg["speed_range"].isNull()) {
        raceLegFile->speedRange=raceFile->speedRange;
        Serial.printf("   speed range (from race): %f\n\n\n", raceLegFile->speedRange);
      } else {
        raceLegFile->speedRange=jsonLeg["speed_range"].as<float>();
        Serial.printf("   speed range: %f\n\n\n", raceLegFile->speedRange);
      }
      Serial.printf("   leg speed: %f\n", raceLegFile->speed);
      raceLegFile->distance=jsonLeg["distance"].as<float>();
      Serial.printf("   leg distance: %f\n\n\n", raceLegFile->distance);
      if(jsonLeg["drive_distance"].isNull()) {
        raceLegFile->driveDistance=raceLegFile->distance;
        Serial.printf("   drive distance is leg distance: %f\n\n\n", raceLegFile->driveDistance);
      } else {
        raceLegFile->driveDistance=jsonLeg["drive_distance"].as<float>();
        Serial.printf("   drive distance: %f\n\n\n", raceLegFile->driveDistance);
      }
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
  if(noRaceFound) {
    Serial.println("No race files found.  Loading default race definitions");
    loadDefaultRaces();
    return;   
  }
  selectedRace=races.begin();
  selectedRaceLeg=(*selectedRace)->raceLegs.begin();
  Serial.println((*selectedRace)->descr);
}

void setRace(race_t *selectedRace, raceLeg_t *selectedRaceLeg) {
  std::vector<raceLeg_t *>::iterator raceLegIt;
  race.activeRace=selectedRace;
  race.activeLeg=selectedRaceLeg;
  race.targetSpeed=(selectedRace->speed)/2.23694;
  race.speedTargetBand=(selectedRace->speedRange)/2.23694;
  race.totalDistance=(selectedRace->distance)/0.000621372;
  race.distanceRemaining=race.totalDistance;
  race.startMark=selectedRace->mark;

  //Figure out how long the entire race will take, in seconds;
  race.time=(double)race.totalDistance/race.targetSpeed;
  //This will give us the actual driven distance for the entire race.
  race.driveDistance=0;
  raceLegIt=selectedRace->raceLegs.begin();
  while(raceLegIt!=selectedRace->raceLegs.end()) {
    race.driveDistance+=((*raceLegIt)->driveDistance/0.000621372);
    ++raceLegIt;
  }
  //Actual target speed to hit, to finish the drive distance in the required
  //time.  Should result in the race average speed over the official race
  //distance.
  race.adjustedTargetSpeed=(double)race.driveDistance/race.time;
  
  race.averageSpeed=0;
  race.distanceComplete=0;
  race.driveDistanceComplete=0;
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

  //leg definitions will have data stored in miles, and miles/hour.  We track things internally
  //in meters and m/s because that's what comes out of the GPS.  Convert the leg definition values
  //to internal values for tracking the race.

  //mph->m/s
  race.legData->targetSpeed=(race.activeLeg->speed)/2.23694;
  //mph->m/s
  race.legData->speedTargetBand=(race.activeLeg->speedRange)/2.23694;
  //miles->meters
  race.legData->totalDistance=(race.activeLeg->distance)/0.000621372;
  //miles->meters
  race.legData->driveDistance=(race.activeLeg->driveDistance)/0.000621372;
  //we don't translate distance remaining?
  race.legData->distanceRemaining=race.legData->driveDistance;
  race.legData->startMark=race.activeLeg->mark;
  //let calculate the actual target speed we need, in order to hit the race target.
  //this will be different if the drive distance is not the same as the race distance.  
  //If drive distance is shorter, then it will be slower.  Faster if longer.  The 
  //contant is the time.  We need to cover the drive distance in the time specified by
  //the leg distance.
  //leg time in seconds.
  race.legData->time=(double)race.legData->totalDistance/race.legData->targetSpeed;
  //now figure the adjusted targer based on how long the leg should take, and the 
  //actual distance we will drive.
  race.legData->adjustedTargetSpeed=(double)race.legData->driveDistance/race.legData->time;
}

void prepRace(void) {
  race.legData->distance=0;
  race.legData->distanceComplete=0;
  race.legData->driveDistanceComplete=0;
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

  loadRacePoints(race.activeLeg);
  race.legData->activePoint=race.activeLeg->points.begin();
}

void loadRacePoints(raceLeg_t *raceLeg) {
  racePoint_t *point;
  char path[256];

  CSV_Parser cp(/*format*/ "udsfss", /*has_header*/ true, /*delimiter*/ ',');
  sprintf(path, "orrc/races/%s", race.activeLeg->pointsFile.c_str());
  //disable display and GPS use of the SPI bus to prevent collisions
  doSPILock();
  if(!sdCard.exists(path)) {
    doSPIUnlock();
    Serial.println("point file not found");
    return;
  }
  Serial.printf("Loading point file: %s\n", path);

  //We are using the SdFat library, so we can;t call CSV_Parser::readSDfile() because it won't understand
  //long file names, so we implement effectively the same code directly here.
  File32 pointFile=sdCard.open(path);
  if(!pointFile) {
    Serial.println("  file open error");
    doSPIUnlock();
    return;
  }
  while (pointFile.available()) {
    cp << (char)pointFile.read();
  }
  pointFile.close();
  doSPIUnlock();
  // ensure that the last value of the file is parsed (even if the file doesn't end with '\n')
  cp.parseLeftover();
  cp.print(); // assumes that "Serial.begin()" was called before (otherwise it won't work)
  uint16_t *turn=(uint16_t *)cp["turn"];
  char **dir=(char **)cp["dir"];
  float *distance=(float *)cp["distance"];
  char **descr1=(char **)cp["descr1"];
  char **descr2=(char **)cp["descr2"];
  for(int row = 0; row < cp.getRowsCount(); row++) {
    point=new racePoint_t;
    point->id=row;
    Serial.printf(" point %d\n", point->id);
    point->turn=turn[row];
    Serial.printf("   turn: %d\n", point->turn);
    if(strcmp(dir[row], "Right")==0) {
      point->turnDir=1;
    } else {
      point->turnDir=0;
    }
    Serial.printf("   dir: %d\n", point->turnDir);
    point->distance=distance[row]/0.000621372;
    Serial.printf("   distance: %d\n", point->distance);
    point->descrLine1=descr1[row];
    Serial.printf("   descr1: %s\n", point->descrLine1.c_str());
    point->descrLine2=descr2[row];
    Serial.printf("   descr2: %s\n", point->descrLine2.c_str());  
    raceLeg->points.push_back(point);     
  }  
}

void clearRacePoints(raceLeg_t *raceLeg) {
  while(!raceLeg->points.empty()){
    Serial.printf("Deleting race point id: %d\n", (*(raceLeg->points.back())).id);
    delete raceLeg->points.back();
    raceLeg->points.erase(raceLeg->points.end()-1);
  }
  race.legData->activePoint=raceLeg->points.end();
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
  race.distance = race.distanceComplete;
  race.distanceRemaining = race.totalDistance - race.distance;  
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
  Serial.printf("  driveDistance=%d m\n", data->driveDistance);
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
  Serial.printf("  timeDelta=%d.%u s\n", data->timeDelta/1000, abs(data->timeDelta%1000));
  Serial.printf("  inProgess=%d\n", data->inProgress);
  Serial.printf("  legData=%#x\n", data->legData);
  Serial.printf("  activeRace=%#x\n", data->activeRace);
  Serial.printf("  activeLeg=%#x\n", data->activeLeg);
}

void raceCheckPoint(void) {
  JsonDocument doc;
  File32 checkPoint;
  if(!sdCardPresent) {
    Serial.println("No SD card.  Unable to checkpoint race");
    return;
  }
  //disable display and GPS use of the SPI bus to prevent collisions
  doSPILock();
  if(!sdCard.exists("orrc")) {
    doSPIUnlock();
    return;
  }
  if(sdCard.exists("orrc/system/race_checkpoint.yml")) {
    Serial.println("clearing race checkpoint");
    sdCard.remove("orrc/system/race_checkpoint.yml");
  }
  if(race.inProgress!=true) {
    Serial.println("no race in progress");
    doSPIUnlock();
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
  checkPoint=sdCard.open("orrc/system/race_checkpoint.yml", FILE_WRITE);
  if(!checkPoint) {
    Serial.println("Checkpoint open failed");

  }
  serializeYml(doc, checkPoint);
  serializeYml(doc, Serial);
  Serial.println();
  checkPoint.close();
  doSPIUnlock();

}

void loadRaceCheckPoint(void) {
  std::list<race_t *>::iterator raceIt;
  std::vector<raceLeg_t *>::iterator raceLegIt;
  int raceLegId;
  JsonDocument doc;
  File32 checkPoint;
  DeserializationError error;
  unsigned char *buffer;
  uint32_t len;
  if(!sdCardPresent) {
    Serial.println("No SD card.  Unable to load race checkpoint");
    return;
  }
  //disable display and GPS use of the SPI bus to prevent collisions
  doSPILock();
  if(!sdCard.exists("orrc/system/race_checkpoint.yml")) {
    doSPIUnlock();
    return;
  }  
  Serial.println("Found race checkpoint");
  checkPoint=sdCard.open("orrc/system/race_checkpoint.yml");
  len=checkPoint.fileSize();
  buffer=(unsigned char *)malloc(len+1);
  if(buffer==0) {
    Serial.println("malloc failed!");
    while(1);
  }
  len=checkPoint.read(buffer, len);
  buffer[len]=0;
  checkPoint.close();
  doSPIUnlock();
  error=deserializeYml(doc, (const char *)buffer);
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

void loadDefaultRaces(void) {
  race_t *raceDef;
  raceLeg_t *raceLegDef;

  raceDef=new race_t;
  raceDef->fileName="";
  raceDef->descr="Big Bend Open Road Race (default)";
  raceDef->speed=110.0;
  raceDef->speedRange=0.05;
  raceDef->distance=118.0;
  raceDef->mark=10;
  raceDef->inProgress=false;
  races.push_back(raceDef);

  raceLegDef=new raceLeg_t;
  raceLegDef->descr="BBORR Southbound";
  raceLegDef->pointsFile="";
  raceLegDef->id=1;
  raceLegDef->speed=110;
  raceLegDef->speedRange=0.05;
  raceLegDef->distance=59.0;
  raceLegDef->driveDistance=58.935;
  raceLegDef->mark=10;
  raceLegDef->inProgress=false;
  raceLegDef->complete=false;
  raceDef->raceLegs.push_back(raceLegDef);

  raceLegDef=new raceLeg_t;
  raceLegDef->descr="BBORR Northbound";
  raceLegDef->pointsFile="";
  raceLegDef->id=2;
  raceLegDef->speed=110;
  raceLegDef->speedRange=0.05;
  raceLegDef->distance=59.0;
  raceLegDef->driveDistance=58.926;
  raceLegDef->mark=10;
  raceLegDef->inProgress=false;
  raceLegDef->complete=false;
  raceDef->raceLegs.push_back(raceLegDef);

  raceDef=new race_t;
  raceDef->fileName="";
  raceDef->descr="BBORR Practice (default)";
  raceDef->speed=110.0;
  raceDef->speedRange=0.1;
  raceDef->distance=15.9;
  raceDef->mark=0;
  raceDef->inProgress=false;
  races.push_back(raceDef);

  raceLegDef=new raceLeg_t;
  raceLegDef->descr="BBORR Practice Westbound";
  raceLegDef->pointsFile="";
  raceLegDef->id=2;
  raceLegDef->speed=110;
  raceLegDef->speedRange=0.1;
  raceLegDef->distance=8.0;
  raceLegDef->driveDistance=8.0;
  raceLegDef->mark=0;
  raceLegDef->inProgress=false;
  raceLegDef->complete=false;
  raceDef->raceLegs.push_back(raceLegDef);

  raceLegDef=new raceLeg_t;
  raceLegDef->descr="BBORR Practice Eastbound";
  raceLegDef->pointsFile="";
  raceLegDef->id=2;
  raceLegDef->speed=110;
  raceLegDef->speedRange=0.1;
  raceLegDef->distance=7.9;
  raceLegDef->driveDistance=7.9;
  raceLegDef->mark=0;
  raceLegDef->inProgress=false;
  raceLegDef->complete=false;
  raceDef->raceLegs.push_back(raceLegDef);

  raceDef=new race_t;
  raceDef->fileName="";
  raceDef->descr="Test and debugging drive (default)";
  raceDef->speed=29.5;
  raceDef->speedRange=0.5;
  raceDef->distance=12.9;
  raceDef->mark=0;
  raceDef->inProgress=false;
  races.push_back(raceDef);

  raceLegDef=new raceLeg_t;
  raceLegDef->descr="Test Drive -- clockwise";
  raceLegDef->pointsFile="";
  raceLegDef->id=1;
  raceLegDef->speed=31.9;
  raceLegDef->speedRange=0.5;
  raceLegDef->distance=6.4;
  raceLegDef->driveDistance=6.4;
  raceLegDef->mark=0;
  raceLegDef->inProgress=false;
  raceLegDef->complete=false;
  raceDef->raceLegs.push_back(raceLegDef);

  raceLegDef=new raceLeg_t;
  raceLegDef->descr="Test Drive -- counterclockwise";
  raceLegDef->pointsFile="";
  raceLegDef->id=2;
  raceLegDef->speed=26.5;
  raceLegDef->speedRange=0.5;
  raceLegDef->distance=6.5;
  raceLegDef->driveDistance=6.5;
  raceLegDef->mark=0;
  raceLegDef->inProgress=false;
  raceLegDef->complete=false;
  raceDef->raceLegs.push_back(raceLegDef);

  selectedRace=races.begin();
  selectedRaceLeg=(*selectedRace)->raceLegs.begin();
  Serial.println((*selectedRace)->descr);
}