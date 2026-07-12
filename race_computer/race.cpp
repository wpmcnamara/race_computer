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
std::list<raceDef_t *> races; 
std::list<raceDef_t *>::iterator selectedRace;
std::list<raceDef_t *>::iterator selectedRaceSave;
std::vector<raceLegDef_t *>::iterator selectedRaceLeg;

bool autoAdjustLegTime=true;

void raceSetup(void) {
  race.activeRace=NULL;
  race.raceSpeedDelta=0;
  race.raceLegEndSpeedDelta=0;
  race.raceTargetDistanceComplete=0;
  race.raceDriveDistanceComplete=0;
  race.raceActualDistanceComplete=0;
  race.raceTimeComplete=0;
  race.raceTargetTimeComplete=0;
  race.raceTime=0;
  race.raceDistanceRemaining=0;
  race.raceLegEndDistanceRemaining=0;
  race.raceTimeDelta=0;
  race.raceInProgress=false;

  race.activeLeg=NULL;
  race.legTargetTime=0;
  race.legAdjustedTargetSpeed=0;
  race.legAverageSpeed=0;
  race.legSpeedDelta=0;
  race.legDistanceComplete=0;
  race.legTime=0;
  race.legDistanceRemaining=0;
  race.legTimeDelta=0;
  race.legInProgress=false;

  race.distanceOffset=0;
  race.startTs=0;
  race.endTs=0;
  race.startMark=0;
  race.delayedStart=0;
  race.timerOffset=0;

  delayedStartEvent=new event_t(raceLegStart, eventSingle, false, false, 0, 0, &Serial, "delayedStartEvent");
  loadRaces();
}

void raceLegStart(void) {
  race.legInProgress=true;
  race.raceInProgress=true;
  race.activeRace->inProgress=true;
  race.delayedStart=false;
  stateMachine.status.flags.delayedStart=false;
  race.distanceOffset=gpsData.distance;
  stateMachine.status.flags.legActive=true;
}

void raceLegStop() {
  race.legInProgress=false;
  stateMachine.status.flags.legActive=false;
}

void computeRace (raceDef_t *raceDefinition) {
  std::vector<raceLegDef_t *>::iterator raceLegIt;
  double cumulativeRaceDistance=0;
  double cumulativeRaceTime=0;
  double cumulativeRaceDriveDistance=0;
  raceLegIt=raceDefinition->raceLegs.begin();
  while(raceLegIt!=raceDefinition->raceLegs.end()) {
    (*raceLegIt)->driveSpeed=(*raceLegIt)->driveDistance/(*raceLegIt)->targetTime;

    cumulativeRaceDistance+=(*raceLegIt)->distance;
    cumulativeRaceDriveDistance+=(*raceLegIt)->driveDistance;
    cumulativeRaceTime+=(*raceLegIt)->targetTime;

    (*raceLegIt)->raceLegEndAvgSpeed=cumulativeRaceDistance/cumulativeRaceTime;      
    (*raceLegIt)->raceLegEndDriveAvgSpeed=cumulativeRaceDriveDistance/cumulativeRaceTime;
    (*raceLegIt)->raceLegEndTargetDistance=cumulativeRaceDistance;
    (*raceLegIt)->raceLegEndDriveDistance=cumulativeRaceDriveDistance;
    (*raceLegIt)->raceLegEndTargetTime=cumulativeRaceTime;
    raceLegIt++;
  }
  raceDefinition->driveDistance=cumulativeRaceDriveDistance;
  raceDefinition->driveSpeed=cumulativeRaceDriveDistance/cumulativeRaceTime;
  //Sanity check defined race speed against the calculated speed from the cumulative leg average.
  if(abs(raceDefinition->speed-(cumulativeRaceDistance/cumulativeRaceTime))>0.0001) {
    Serial.println("  race speed mismatch, using calculate value\n");
    raceDefinition->speed=cumulativeRaceDistance/cumulativeRaceTime;
  }
  if(abs(raceDefinition->targetTime-cumulativeRaceTime)>0.0001) {
    Serial.println("  race time mismatch, using calculated value\n");
    raceDefinition->targetTime=cumulativeRaceTime;
  }
  if(abs(raceDefinition->distance-cumulativeRaceDistance)>0.0001) {
    Serial.println("  race distance mismatch, using calculated value\n");
    raceDefinition->distance=cumulativeRaceDistance;
  }

}
void loadRaces() {
  File32 entry;
  JsonDocument doc;
  YAMLNode yamlnode;
  DeserializationError error;
  raceDef_t *raceFile;
  raceLegDef_t *raceLegFile;
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
  //doSPILock();
  //race data files will be stored in a directory called "orc"
  //if it doesn't exist, then we've got races to load.
  if(!sdCard.exists("orrc")) {
    Serial.println("Data directory not found.  Loading default race definitions.");
    //doSPIUnlock();
    loadDefaultRaces();
    return;
  }
  File32 orcDir=sdCard.open("orrc/races");
  //doSPIUnlock();
  //process the orc directory.  Anything that ends in .csv will be considered a race file.
  while (true) {
    //doSPILock();
    entry =  orcDir.openNextFile();
    if (! entry) {
      //doSPIUnlock();
      break;
    }
    //We are looking for files right now, so skip directories.
    if(entry.isDirectory()) {
      //doSPIUnlock();
      continue;
    }
    entry.getName(fileName,256);
    if(strstr(fileName, ".yml")==NULL) {
      //doSPIUnlock();
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
    raceFile=new raceDef_t;
    entry.getName(fileName, 256);
    raceFile->fileName=fileName;
    entry.close();
    //doSPIUnlock();
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
    raceFile->distance=DISTANCE_MILES_TO_INTERNAL(doc["distance"].as<float>());
    Serial.printf("race distance: %fmi, %0.3fmm\n",doc["distance"].as<float>(),raceFile->distance);       
    raceFile->speed=SPEED_MPH_TO_INTERNAL(doc["speed"].as<float>());
    Serial.printf("race speed: %fmph, %0.3fmm/ms\n",doc["speed"].as<float>(),raceFile->speed);  
    raceFile->speedRange=SPEED_MPH_TO_INTERNAL(doc["speedRange"].as<float>());
    Serial.printf("race speed range: %fmph, %0.3fmm/ms\n", doc["speedRange"].as<float>(), raceFile->speedRange);
    raceFile->mark=TIME_SECONDS_TO_INTERNAL(doc["tmark"].as<int>());
    Serial.printf("race mark: %ds, %0.3fms\n", doc["tmark"].as<int>(),raceFile->mark);    
    raceFile->targetTime=raceFile->distance/raceFile->speed;
    Serial.printf("race target time: %0.03fs\n\n",TIME_INTERNAL_TO_SECONDS(raceFile->targetTime));
    raceFile->inProgress=false;
    races.push_back(raceFile);

    for (JsonObject jsonLeg : doc["legs"].as<JsonArray>()) {
      raceLegFile=new raceLegDef_t;
      raceLegFile->descr=jsonLeg["descr"].as<String>();
      Serial.printf("   leg descr: %s\n", raceLegFile->descr.c_str());
      raceLegFile->pointsFile=jsonLeg["points"].as<String>();
      Serial.printf("   leg pointsFile: %s\n", raceLegFile->pointsFile.c_str());
      raceLegFile->id=jsonLeg["id"].as<int>();
      Serial.printf("   leg id: %d\n", raceLegFile->id);
      raceLegFile->speed=SPEED_MPH_TO_INTERNAL(jsonLeg["speed"].as<float>());
      Serial.printf("   leg speed: %0.3fmph, %0.3fmm/ms\n", jsonLeg["speed"].as<float>(),raceLegFile->speed);
      if(jsonLeg["speed_range"].isNull()) {
        raceLegFile->speedRange=raceFile->speedRange;
        Serial.printf("   speed range (from race): %0.3f\n", raceLegFile->speedRange);
      } else {
        raceLegFile->speedRange=SPEED_MPH_TO_INTERNAL(jsonLeg["speedRange"].as<float>());
        Serial.printf("   speed range: %0.3fmph, %0.3fmm/ms\n", jsonLeg["speedRange"].as<float>(), raceLegFile->speedRange);
      }
      raceLegFile->distance=DISTANCE_MILES_TO_INTERNAL(jsonLeg["distance"].as<float>());
      Serial.printf("   leg distance: %0.3fmi, %0.3fmm\n", jsonLeg["distance"].as<float>(), raceLegFile->distance);      
      raceLegFile->targetTime=raceLegFile->distance/raceLegFile->speed;
      if(jsonLeg["driveDistance"].isNull()) {
        raceLegFile->driveDistance=raceLegFile->distance;
        Serial.printf("   drive distance is leg distance: %0.3f\n", raceLegFile->driveDistance);
      } else {
        raceLegFile->driveDistance=DISTANCE_MILES_TO_INTERNAL(jsonLeg["driveDistance"].as<float>());
        Serial.printf("   drive distance: %0.3fmi, %0.3fmm\n", jsonLeg["driveDistance"].as<float>(), raceLegFile->driveDistance);
      }

      if(jsonLeg["tmark"].isNull()) {
        raceLegFile->mark=raceFile->mark;
        Serial.printf("   leg mark(from race): %0.3fms\n", raceLegFile->mark);
      } else{
        raceLegFile->mark=TIME_SECONDS_TO_INTERNAL(jsonLeg["tmark"].as<int>());
        Serial.printf("   leg mark: %ds, %0.3fms\n", jsonLeg["tmark"].as<int>(),raceLegFile->mark);
      }
      Serial.println("");
      raceLegFile->inProgress=false;
      raceLegFile->complete=false;

      raceFile->raceLegs.push_back(raceLegFile);
    } 
    computeRace(raceFile);
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

void setRace(raceDef_t *selectedRace) {
  std::vector<raceLegDef_t *>::iterator raceLegIt;
  race.activeRace=selectedRace;
  race.raceSpeedDelta=0;
  race.raceLegEndSpeedDelta=0;
  race.raceTargetDistanceComplete=0;
  race.raceDriveDistanceComplete=0;
  race.raceActualDistanceComplete=0;
  race.raceTimeComplete=0;
  race.raceTargetTimeComplete=0;
  race.raceTime=0;
  race.raceDistanceRemaining=0;
  race.raceLegEndDistanceRemaining=0;
  race.raceTimeDelta=0;
  race.raceInProgress=false;
}

void setLeg(raceLegDef_t *selectedRaceLeg) {
  race.activeLeg=selectedRaceLeg;

  race.legTargetTime=selectedRaceLeg->targetTime;
  race.legAdjustedTargetSpeed=0;
  race.legAverageSpeed=0;
  race.legSpeedDelta=0;
  race.legDistanceComplete=0;
  race.legTime=0;
  race.legDistanceRemaining=selectedRaceLeg->driveDistance;
  race.legTimeDelta=0;
  race.legInProgress=false;

  race.distanceOffset=0;
  race.startTs=0;
  race.endTs=0;
  race.startMark=selectedRaceLeg->mark;
  race.delayedStart=0;
  race.timerOffset=0;

  if(autoAdjustLegTime) {
  //let calculate the actual target speed we need, in order to hit the race target.
  //this will be different if the drive distance is not the same as the race distance.  
  //If drive distance is shorter, then it will be slower.  Faster if longer.  The 
  //contant is the time.  We need to cover the drive distance in the time specified by
  //the leg distance.
    race.legTargetTime-=race.raceTimeDelta;
  } 
  //now figure the adjusted targer based on how long the leg should take, and the 
  //actual distance we will drive.
  race.legAdjustedTargetSpeed=selectedRaceLeg->driveDistance/race.legTargetTime;
}

void prepRace(void) {
  race.legAverageSpeed=0;
  race.legSpeedDelta=0;
  race.legDistanceComplete=0;
  race.legTime=0;
  race.legDistanceRemaining=0;
  race.legTimeDelta=0;
  race.legInProgress=false;

  race.distanceOffset=0;
  race.startTs=0;
  race.endTs=0;
  race.delayedStart=0;
  race.timerOffset=0;

  race.legDistanceRemaining=race.activeLeg->driveDistance;
  loadRacePoints(race.activeLeg);
  race.activePoint=race.activeLeg->points.begin();
}

void loadRacePoints(raceLegDef_t *raceLeg) {
  racePoint_t *point;
  char path[256];

  CSV_Parser cp(/*format*/ "udsfss", /*has_header*/ true, /*delimiter*/ ',');
  sprintf(path, "orrc/races/%s", race.activeLeg->pointsFile.c_str());
  //disable display and GPS use of the SPI bus to prevent collisions
  //doSPILock();
  if(!sdCard.exists(path)) {
    //doSPIUnlock();
    Serial.println("point file not found");
    return;
  }
  Serial.printf("Loading point file: %s\n", path);

  //We are using the SdFat library, so we can;t call CSV_Parser::readSDfile() because it won't understand
  //long file names, so we implement effectively the same code directly here.
  File32 pointFile=sdCard.open(path);
  if(!pointFile) {
    Serial.println("  file open error");
    //doSPIUnlock();
    return;
  }
  while (pointFile.available()) {
    cp << (char)pointFile.read();
  }
  pointFile.close();
  //doSPIUnlock();
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
    point->distance=DISTANCE_MILES_TO_INTERNAL(distance[row]);
    Serial.printf("   distance: %fmi, %dum\n", distance[row], point->distance);
    point->descrLine1=descr1[row];
    Serial.printf("   descr1: %s\n", point->descrLine1.c_str());
    point->descrLine2=descr2[row];
    Serial.printf("   descr2: %s\n", point->descrLine2.c_str());  
    raceLeg->points.push_back(point);     
  }  
}

void clearRacePoints(raceLegDef_t *raceLeg) {
  while(!raceLeg->points.empty()){
    Serial.printf("Deleting race point id: %d\n", (*(raceLeg->points.back())).id);
    delete raceLeg->points.back();
    raceLeg->points.erase(raceLeg->points.end()-1);
  }
  race.activePoint=raceLeg->points.end();
}

//We stopped accumulating distance when the start/stop button was pressed.  We don't get the final
//time for the race until the time mark comes in, meaning the average speed may be wrong by just a
//bit.  We need to do a final set of calculates to update all the running values that depend on time.
//Beyond that, we update final values based on the end of leg data.
void updateRace(void) {
  race.activeLeg->complete=true;
  race.legTime=race.endTs-race.startTs;
  race.legAverageSpeed = race.legDistanceComplete / race.legTime;
  race.legSpeedDelta = race.legAverageSpeed - race.legAdjustedTargetSpeed;
  race.legTimeDelta=race.legTime-race.legTargetTime;

  race.raceTimeComplete+=race.legTime;
  race.raceTime=race.raceTimeComplete;
  race.raceTargetTimeComplete+=race.activeLeg->targetTime;
  race.raceTimeDelta=race.raceTimeComplete-race.raceTargetTimeComplete;

  race.raceActualDistanceComplete+=race.legDistanceComplete;
  race.raceDriveDistanceComplete+=race.activeLeg->driveDistance;
  race.raceTargetDistanceComplete+=race.activeLeg->distance;
  race.raceDistanceRemaining=race.activeRace->driveDistance-race.raceDriveDistanceComplete;

  race.raceAverageSpeed=race.raceDistanceComplete/race.raceTimeComplete;
  race.raceSpeedDelta = race.raceAverageSpeed - race.activeRace->driveSpeed;
  race.raceLegEndSpeedDelta = race.raceAverageSpeed - race.activeLeg->raceLegEndDriveAvgSpeed;      
  
}

void raceCheckPoint(void) {
  JsonDocument doc;
  File32 checkPoint;
  if(!sdCardPresent) {
    Serial.println("No SD card.  Unable to checkpoint race");
    return;
  }
  //disable display and GPS use of the SPI bus to prevent collisions
  //doSPILock();
  if(!sdCard.exists("orrc")) {
    //doSPIUnlock();
    return;
  }
  if(sdCard.exists("orrc/system/race_checkpoint.yml")) {
    Serial.println("clearing race checkpoint");
    sdCard.remove("orrc/system/race_checkpoint.yml");
  }
  if(race.raceInProgress!=true) {
    Serial.println("no race in progress");
    //doSPIUnlock();
    return;
  }
  Serial.println("Dumping race checkpoint");
  doc["raceFile"]=race.activeRace->fileName;
  doc["activeRaceInProgress"]=race.activeRace->inProgress;
  doc["raceInProgress"]=race.raceInProgress; 
  doc["legId"]=race.activeLeg->id;
  doc["activeLegInProgress"]=race.activeLeg->inProgress;
  doc["activeLegComplete"]=race.activeLeg->complete;

  doc["raceActualDistanceComplete"]=race.raceActualDistanceComplete;
  doc["raceDriveDistanceComplete"]=race.raceDriveDistanceComplete;
  doc["raceTargetDistanceComplete"]=race.raceTargetDistanceComplete;
  doc["raceDistanceRemaining"]=race.raceDistanceRemaining;
  doc["raceLegEndDistanceRemaining"]=race.raceLegEndDistanceRemaining;

  doc["raceTimeComplete"]=race.raceTimeComplete;
  doc["targetTimeComplete"]=race.raceTargetTimeComplete;
  doc["raceTime"]=race.raceTime;

  doc["raceTimeDelta"]=race.raceTimeDelta;
  doc["raceSpeedDelta"]=race.raceSpeedDelta;
  doc["raceLegEndSpeedDelta"]=race.raceLegEndSpeedDelta;

  checkPoint=sdCard.open("orrc/system/race_checkpoint.yml", FILE_WRITE);
  if(!checkPoint) {
    Serial.println("Checkpoint open failed");

  }
  serializeYml(doc, checkPoint);
  serializeYml(doc, Serial);
  Serial.println();
  checkPoint.close();
  //doSPIUnlock();

}

void loadRaceCheckPoint(void) {
  std::list<raceDef_t *>::iterator raceIt;
  std::vector<raceLegDef_t *>::iterator raceLegIt;
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
  //doSPILock();
  if(!sdCard.exists("orrc/system/race_checkpoint.yml")) {
    //doSPIUnlock();
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
  //doSPIUnlock();
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
  if(doc["activeLegInProgress"].as<bool>()==false && doc["activeLegComplete"].as<bool>()==true) {
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
  //Initialize the race represented in the checkpoint.
  setRace((*raceIt));
  //Now restart all the in progress values for the race.  This is effectively all race
  //values updated in updateRace().  We need to do this before we initialize the active
  //leg as it's initialization may depend on some of the restored race values.
  race.activeRace->inProgress=doc["activeRaceInProgress"].as<bool>();
  race.raceInProgress=doc["raceInProgress"].as<bool>();

  race.raceActualDistanceComplete=doc["raceActualDistanceComplete"].as<double>();
  race.raceDriveDistanceComplete=doc["raceDriveDistanceComplete"].as<double>();
  race.raceTargetDistanceComplete=doc["raceTargetDistanceComplete"].as<double>();
  race.raceDistanceRemaining=doc["raceDistanceRemaining"].as<double>();
  race.raceLegEndDistanceRemaining=doc["raceLegEndDistanceRemaining"].as<double>();

  race.raceTimeComplete=doc["raceTimeComplete"].as<double>();
  race.raceTargetTimeComplete=doc["targetTimeComplete"].as<double>();
  race.raceTime=doc["raceTime"].as<double>();

  race.raceTimeDelta=doc["raceTimeDelta"].as<double>();
  race.raceSpeedDelta=doc["raceSpeedDelta"].as<double>();
  race.raceLegEndSpeedDelta=doc["raceLegEndSpeedDelta"].as<double>();

  setLeg((*raceLegIt));
  selectedRace=raceIt;
  selectedRaceLeg=raceLegIt;
}

void loadDefaultRaces(void) {
  raceDef_t *raceDef;
  raceLegDef_t *raceLegDef;

  raceDef=new raceDef_t;
  raceDef->fileName="";
  raceDef->descr="Test and debugging drive (default)";
  raceDef->speed=SPEED_MPH_TO_INTERNAL(29.5);
  raceDef->speedRange=SPEED_MPH_TO_INTERNAL(0.5);
  raceDef->distance=DISTANCE_MILES_TO_INTERNAL(12.9);
  raceDef->mark=0;
  raceDef->inProgress=false;
  races.push_back(raceDef);

  raceLegDef=new raceLegDef_t;
  raceLegDef->descr="Test Drive -- clockwise";
  raceLegDef->pointsFile="";
  raceLegDef->id=1;
  raceLegDef->speed=SPEED_MPH_TO_INTERNAL(31.9);
  raceLegDef->speedRange=SPEED_MPH_TO_INTERNAL(0.5);
  raceLegDef->distance=DISTANCE_MILES_TO_INTERNAL(6.4);
  raceLegDef->driveDistance=DISTANCE_MILES_TO_INTERNAL(6.4);
  raceLegDef->mark=0;
  raceLegDef->inProgress=false;
  raceLegDef->complete=false;
  raceDef->raceLegs.push_back(raceLegDef);

  raceLegDef=new raceLegDef_t;
  raceLegDef->descr="Test Drive -- counterclockwise";
  raceLegDef->pointsFile="";
  raceLegDef->id=2;
  raceLegDef->speed=SPEED_MPH_TO_INTERNAL(26.5);
  raceLegDef->speedRange=SPEED_MPH_TO_INTERNAL(0.5);
  raceLegDef->distance=DISTANCE_MILES_TO_INTERNAL(6.5);
  raceLegDef->driveDistance=DISTANCE_MILES_TO_INTERNAL(6.5);
  raceLegDef->mark=0;
  raceLegDef->inProgress=false;
  raceLegDef->complete=false;
  raceDef->raceLegs.push_back(raceLegDef);
  computeRace(raceDef);
  selectedRace=races.begin();
  selectedRaceLeg=(*selectedRace)->raceLegs.begin();
  Serial.println((*selectedRace)->descr);
}