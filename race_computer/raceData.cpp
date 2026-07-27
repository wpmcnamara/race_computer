#include "raceData.h"
#include "event.h"
#include "gps.h"
#include "keypad.h"
#include "storage.h"
#include "state_machine.h"
#include "display.h"
#include <ArduinoJson.h>
#include <CSV_Parser.h>
#include "raceDef.h"
#include "raceLegDef.h"
#include "racePoint.h"

raceData_t race;
event_t *delayedStartEvent;

bool autoAdjustLegTime=true;

void raceLegStartEvent(void) {
    race.raceLegStart();
}

raceData::raceData() {
  activeRace=NULL;
  mRaceSpeedDelta=0;
  mRaceLegEndSpeedDelta=0;
  mRaceTargetDistanceComplete=0;
  mRaceDriveDistanceComplete=0;
  mRaceActualDistanceComplete=0;
  mRaceTimeComplete=0;
  mRaceTargetTimeComplete=0;
  mRaceTime=0;
  mRaceDistanceRemaining=0;
  mRaceLegEndDistanceRemaining=0;
  mRaceTimeDelta=0;
  mRaceInProgress=false;

  activeLeg=NULL;
  mLegTargetTime=0;
  mLegAdjustedTargetSpeed=0;
  mLegAverageSpeed=0;
  mLegSpeedDelta=0;
  mLegDistanceComplete=0;
  mLegTime=0;
  mLegDistanceRemaining=0;
  mLegTimeDelta=0;
  mLegInProgress=false;

  mDistanceOffset=0;
  mStartTs=0;
  mEndTs=0;
  mStartMark=0;
  mDelayedStart=false;
  mTimerOffset=0;
}

void raceData::raceLegStart(void) {
  mLegInProgress=true;
  mRaceInProgress=true;
  activeRace->inProgress=true;
  mDelayedStart=false;
  stateMachine.status.flags.delayedStart=false;
  mDistanceOffset=gpsData.distance;
  stateMachine.status.flags.legActive=true;
}

void raceData::raceLegStop() {
  mLegInProgress=false;
  stateMachine.status.flags.legActive=false;
}

double raceData::legTargetTime(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mLegTargetTime);
    } else if (units==milliseconds) {
        return mLegTargetTime;
    } else {
        Serial.printf("Invalid units passed to %s: %d\n", __func__, units);
        while(1);
    }
}

double raceData::legAdjustedTargetSpeed(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mLegAdjustedTargetSpeed);
  } else {
    return SPEED_INTERNAL_TO_KPH(mLegAdjustedTargetSpeed);
  }
}

double raceData::legAverageSpeed(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mLegAverageSpeed);
  } else {
    return SPEED_INTERNAL_TO_KPH(mLegAverageSpeed);
  }  
}

double raceData::legSpeedDelta(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mLegSpeedDelta);
  } else {
    return SPEED_INTERNAL_TO_KPH(mLegSpeedDelta);
  }  
}

double raceData::raceSpeedDelta(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mRaceSpeedDelta);
  } else {
    return SPEED_INTERNAL_TO_KPH(mRaceSpeedDelta);
  }   
}

double raceData::legDistanceRemaining(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mLegDistanceRemaining);
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mLegDistanceRemaining);
  }  
}

double raceData::legTimeDelta(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mLegTimeDelta);
    } else if (units==milliseconds) {
        return mLegTimeDelta;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceData::raceAverageSpeed(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mRaceAverageSpeed);
  } else {
    return SPEED_INTERNAL_TO_KPH(mRaceAverageSpeed);
  }  
}

double raceData::raceLegEndSpeedDelta(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mRaceLegEndSpeedDelta);
  } else {
    return SPEED_INTERNAL_TO_KPH(mRaceLegEndSpeedDelta);
  }   
}

double raceData::raceDistanceComplete(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mRaceDistanceComplete);
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mRaceDistanceComplete);
  }  
}

double raceData::raceTargetDistanceComplete(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mRaceTargetDistanceComplete);
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mRaceTargetDistanceComplete);
  }  
}

double raceData::raceDriveDistanceComplete(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mRaceDriveDistanceComplete);
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mRaceDriveDistanceComplete);
  }  
}

double raceData::raceActualDistanceComplete(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mRaceActualDistanceComplete);
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mRaceActualDistanceComplete);
  }  
}

double raceData::raceTimeComplete(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mRaceTimeComplete);
    } else if (units==milliseconds) {
        return mRaceTimeComplete;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceData::raceTargetTimeComplete(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mRaceTargetTimeComplete);
    } else if (units==milliseconds) {
        return mRaceTargetTimeComplete;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceData::legDistanceComplete(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mLegDistanceComplete);
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mLegDistanceComplete);
  }  
}

double raceData::legTime(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mLegTime);
    } else if (units==milliseconds) {
        return mLegTime;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }

}

double raceData::raceTime(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mRaceTime);
    } else if (units==milliseconds) {
        return mRaceTime;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceData::raceDistanceRemaining(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mRaceDistanceRemaining);
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mRaceDistanceRemaining);
  }  
}

double raceData::raceLegEndDistanceRemaining(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mRaceLegEndDistanceRemaining);
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mRaceLegEndDistanceRemaining);
  }  
}

double raceData::distanceOffset(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mDistanceOffset);
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mDistanceOffset);
  }  
}

double raceData::startTs(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mStartTs);
    } else if (units==milliseconds) {
        return mStartTs;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceData::endTs(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mEndTs);
    } else if (units==milliseconds) {
        return mEndTs;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceData::startMark(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mStartMark);
    } else if (units==milliseconds) {
        return mStartMark;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }    
}

bool raceData::delayedStart(void) {
  return mDelayedStart;
}

double raceData::timerOffset(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mTimerOffset);
    } else if (units==milliseconds) {
        return mTimerOffset;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }    
}

double raceData::raceTimeDelta(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mRaceTimeDelta);
    } else if (units==milliseconds) {
        return mRaceTimeDelta;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

bool raceData::raceInProgress(void) {
    return mRaceInProgress;
}

bool raceData::legInProgress(void) {
    return mLegInProgress;
}

double raceData::legAdjustedAverageSpeed(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mLegAdjustedAverageSpeed);
  } else {
    return SPEED_INTERNAL_TO_KPH(mLegAdjustedAverageSpeed);
  }   
}

double raceData::raceAdjustedAverageSpeed(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mLegAdjustedAverageSpeed);
  } else {
    return SPEED_INTERNAL_TO_KPH(mLegAdjustedAverageSpeed);
  }   
}

void computeRace (raceDef_t *raceDefinition) {
  std::vector<raceLegDef_t *>::iterator raceLegIt;
  double cumulativeRaceDistance=0;
  double cumulativeRaceTime=0;
  double cumulativeRaceDriveDistance=0;
  raceLegIt=raceDefinition->raceLegs.begin();
  while(raceLegIt!=raceDefinition->raceLegs.end()) {
    if((*raceLegIt)->targetTime) {
      (*raceLegIt)->driveSpeed=(*raceLegIt)->driveDistance/(*raceLegIt)->targetTime;
    } else {
      //can't have infinite speed, and shouldn't have a leg with zero target time, but just
      //in case, we will force speed to 0 if we have 0 time, as an error condition.
      (*raceLegIt)->driveSpeed=0;
    }
    cumulativeRaceDistance+=(*raceLegIt)->distance;
    cumulativeRaceDriveDistance+=(*raceLegIt)->driveDistance;
    cumulativeRaceTime+=(*raceLegIt)->targetTime;

    if(cumulativeRaceTime!=0) {
      (*raceLegIt)->raceLegEndAvgSpeed=cumulativeRaceDistance/cumulativeRaceTime;      
    } else {
      //Like above, we should never end up with a cumulativeRaceTimeValue that is zero, but if
      //we do, force the ending average speed to zero as the error condition.
      (*raceLegIt)->raceLegEndAvgSpeed=0;
    }
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
    Serial.printf("race speed: %fimperial, %0.3fmm/ms\n",doc["speed"].as<float>(),raceFile->speed);  
    raceFile->speedRange=SPEED_MPH_TO_INTERNAL(doc["speedRange"].as<float>());
    Serial.printf("race speed range: %fimperial, %0.3fmm/ms\n", doc["speedRange"].as<float>(), raceFile->speedRange);
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
      Serial.printf("   leg speed: %0.3fimperial, %0.3fmm/ms\n", jsonLeg["speed"].as<float>(),raceLegFile->speed);
      if(jsonLeg["speed_range"].isNull()) {
        raceLegFile->speedRange=raceFile->speedRange;
        Serial.printf("   speed range (from race): %0.3f\n", raceLegFile->speedRange);
      } else {
        raceLegFile->speedRange=SPEED_MPH_TO_INTERNAL(jsonLeg["speedRange"].as<float>());
        Serial.printf("   speed range: %0.3fimperial, %0.3fmm/ms\n", jsonLeg["speedRange"].as<float>(), raceLegFile->speedRange);
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

void raceData::setRace(raceDef_t *selectedRace) {
  activeRace=selectedRace;
  mRaceSpeedDelta=0;
  mRaceLegEndSpeedDelta=0;
  mRaceTargetDistanceComplete=0;
  mRaceDriveDistanceComplete=0;
  mRaceActualDistanceComplete=0;
  mRaceTimeComplete=0;
  mRaceTargetTimeComplete=0;
  mRaceTime=0;
  mRaceDistanceRemaining=0;
  mRaceLegEndDistanceRemaining=0;
  mRaceTimeDelta=0;
  mRaceInProgress=false;
}

void raceData::setLeg(raceLegDef_t *selectedRaceLeg) {
  activeLeg=selectedRaceLeg;

  mLegTargetTime=selectedRaceLeg->targetTime;
  mLegAdjustedTargetSpeed=0;
  mLegAverageSpeed=0;
  mLegSpeedDelta=0;
  mLegDistanceComplete=0;
  mLegTime=0;
  mLegDistanceRemaining=selectedRaceLeg->driveDistance;
  mLegTimeDelta=0;
  mLegInProgress=false;

  mDistanceOffset=0;
  mStartTs=0;
  mEndTs=0;
  mStartMark=selectedRaceLeg->mark;
  mDelayedStart=false;
  mTimerOffset=0;

  if(autoAdjustLegTime) {
  //let calculate the actual target speed we need, in order to hit the race target.
  //this will be different if the drive distance is not the same as the race distance.  
  //If drive distance is shorter, then it will be slower.  Faster if longer.  The 
  //contant is the time.  We need to cover the drive distance in the time specified by
  //the leg distance.
    mLegTargetTime-=mRaceTimeDelta;
    //under some test conditions we might end up with a negative target time, so we set
    //the floor to zero, which is still weird, but at least not negative.
    if(mLegTargetTime<0) {
      mLegTargetTime=0;
    }
  } 
  //now figure the adjusted targer based on how long the leg should take, and the 
  //actual distance we will drive.
  if(mLegTargetTime!=0) {
    mLegAdjustedTargetSpeed=selectedRaceLeg->driveDistance/mLegTargetTime;
  } else {
    //Can't have infinite speed, so we force it to zero as an error condition.  In reality,
    //this should never be and is mainly here to ensure code analysis doesn't complain about
    //the uncheckd potential for divide by zero.
    mLegAdjustedTargetSpeed=0;
  }
}

void raceData::prepRace(void) {
  mLegAverageSpeed=0;
  mLegSpeedDelta=0;
  mLegDistanceComplete=0;
  mLegTime=0;
  mLegDistanceRemaining=0;
  mLegTimeDelta=0;
  mLegInProgress=false;

  mDistanceOffset=0;
  mStartTs=0;
  mEndTs=0;
  mDelayedStart=false;
  mTimerOffset=0;

  mLegDistanceRemaining=activeLeg->driveDistance;
  loadRacePoints(activeLeg);
  activePoint=activeLeg->points.begin();
}

//We stopped accumulating distance when the start/stop button was pressed.  We don't get the final
//time for the race until the time mark comes in, meaning the average speed may be wrong by just a
//bit.  We need to do a final set of calculates to update all the running values that depend on time.
//Beyond that, we update final values based on the end of leg data.
void raceData::updateRace(void) {
  activeLeg->complete=true;
  mLegTime=mEndTs-mStartTs;
  mLegAverageSpeed = mLegDistanceComplete / mLegTime;
  mLegSpeedDelta = mLegAverageSpeed - mLegAdjustedTargetSpeed;
  mLegTimeDelta=mLegTime-mLegTargetTime;

  mRaceTimeComplete+=mLegTime;
  mRaceTime=mRaceTimeComplete;
  mRaceTargetTimeComplete+=activeLeg->targetTime;
  mRaceTimeDelta=mRaceTimeComplete-mRaceTargetTimeComplete;

  mRaceActualDistanceComplete+=mLegDistanceComplete;
  mRaceDriveDistanceComplete+=activeLeg->driveDistance;
  mRaceTargetDistanceComplete+=activeLeg->distance;
  mRaceDistanceRemaining=activeRace->driveDistance-mRaceDriveDistanceComplete;

  mRaceAverageSpeed=mRaceDistanceComplete/mRaceTimeComplete;
  mRaceSpeedDelta = mRaceAverageSpeed - activeRace->driveSpeed;
  mRaceLegEndSpeedDelta = mRaceAverageSpeed - activeLeg->raceLegEndDriveAvgSpeed;      
  
  mLegAdjustedAverageSpeed=activeLeg->distance/mLegTime;
  mRaceAdjustedAverageSpeed=activeLeg->raceLegEndTargetDistance/mRaceTimeComplete;

}

void raceData::raceCheckPoint(void) {
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
  if(mLegInProgress!=true) {
    Serial.println("no race in progress");
    //doSPIUnlock();
    return;
  }
  Serial.println("Dumping race checkpoint");
  doc["raceFile"]=activeRace->fileName;
  doc["activeRaceInProgress"]=activeRace->inProgress;
  doc["raceInProgress"]=mRaceInProgress; 
  doc["legId"]=activeLeg->id;
  doc["activeLegInProgress"]=activeLeg->inProgress;
  doc["activeLegComplete"]=activeLeg->complete;

  doc["raceActualDistanceComplete"]=mRaceActualDistanceComplete;
  doc["raceDriveDistanceComplete"]=mRaceDriveDistanceComplete;
  doc["raceTargetDistanceComplete"]=mRaceTargetDistanceComplete;
  doc["raceDistanceRemaining"]=mRaceDistanceRemaining;
  doc["raceLegEndDistanceRemaining"]=mRaceLegEndDistanceRemaining;

  doc["raceTimeComplete"]=mRaceTimeComplete;
  doc["targetTimeComplete"]=mRaceTargetTimeComplete;
  doc["raceTime"]=mRaceTime;

  doc["raceTimeDelta"]=mRaceTimeDelta;
  doc["raceSpeedDelta"]=mRaceSpeedDelta;
  doc["raceLegEndSpeedDelta"]=mRaceLegEndSpeedDelta;

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

void raceData::loadRaceCheckPoint(void) {
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
  activeRace->inProgress=doc["activeRaceInProgress"].as<bool>();
  mRaceInProgress=doc["raceInProgress"].as<bool>();

  mRaceActualDistanceComplete=doc["raceActualDistanceComplete"].as<double>();
  mRaceDriveDistanceComplete=doc["raceDriveDistanceComplete"].as<double>();
  mRaceTargetDistanceComplete=doc["raceTargetDistanceComplete"].as<double>();
  mRaceDistanceRemaining=doc["raceDistanceRemaining"].as<double>();
  mRaceLegEndDistanceRemaining=doc["raceLegEndDistanceRemaining"].as<double>();

  mRaceTimeComplete=doc["raceTimeComplete"].as<double>();
  mRaceTargetTimeComplete=doc["targetTimeComplete"].as<double>();
  mRaceTime=doc["raceTime"].as<double>();

  mRaceTimeDelta=doc["raceTimeDelta"].as<double>();
  mRaceSpeedDelta=doc["raceSpeedDelta"].as<double>();
  mRaceLegEndSpeedDelta=doc["raceLegEndSpeedDelta"].as<double>();

  setLeg((*raceLegIt));
  selectedRace=raceIt;
  selectedRaceLeg=raceLegIt;
}


void raceData::startTs(double ts, units_t units){
    if(units==seconds) {
        mStartTs=TIME_SECONDS_TO_INTERNAL(ts);
    } else if (units==milliseconds) {
        mStartTs=ts;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
};

void raceData::endTs(double ts, units_t units){
    if(units==seconds) {
        mEndTs=TIME_SECONDS_TO_INTERNAL(ts);
    } else if (units==milliseconds) {
        mEndTs=ts;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
};

void raceData::timerOffset(double offset, units_t units){
    if(units==seconds) {
        mTimerOffset=TIME_SECONDS_TO_INTERNAL(offset);
    } else if (units==milliseconds) {
        mTimerOffset=offset;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
};

void raceData::delayedStart(bool state){
  mDelayedStart=state;
};

void raceData::updateRunning(double elapsedTime, double distance) {
  double targetTime;
  double speedDelta;

  mLegTime=elapsedTime;
  mRaceTime=mRaceTimeComplete+elapsedTime;
  mLegDistanceComplete = distance - mDistanceOffset;
  mLegDistanceRemaining = activeLeg->driveDistance - mLegDistanceComplete;
  if(elapsedTime!=0) {
    mLegAverageSpeed = mLegDistanceComplete / elapsedTime;
  } else {
    mLegAverageSpeed=0;
  }
  //we use the adjustedTargetSpeed here as there isn't a good way to scale things from drive distance
  //to leg distance.  We could technically do it, but the scaling factor is tiny (0.1% as an example)
  //and ensuring no loss of precision isn't worth the hassle.
  mLegSpeedDelta = mLegAverageSpeed - mLegAdjustedTargetSpeed;    
  //Calculate how long it should have taken for us to travel the distance we have, at the actual driving 
  //target speed for the leg.
  targetTime=mLegDistanceComplete / mLegAdjustedTargetSpeed;
  //Our time delta is the difference between how long it should have taken and how long it did take.
  //Delta will be negative if we are faster, positive if we are slower.
  mLegTimeDelta=elapsedTime-targetTime;

  //we use the drive values here as it wouldn't make sense to use published values for 
  //the race distance when the leg distance is actual distance driven.  Race values, using
  //published distance are only valid at the end of a leg.
  mRaceDistanceComplete=mRaceDriveDistanceComplete + mLegDistanceComplete;
  mRaceDistanceRemaining = activeRace->driveDistance - mRaceDistanceComplete;
  mRaceLegEndDistanceRemaining = activeLeg->raceLegEndDriveDistance - mRaceDistanceComplete;
  if(mRaceTimeComplete!=0 || elapsedTime!=0) {
    mRaceAverageSpeed = mRaceDistanceComplete / (mRaceTimeComplete+elapsedTime);
  } else {
    mRaceAverageSpeed=0;
  }
  mRaceSpeedDelta = mRaceAverageSpeed - activeRace->driveSpeed;
  mRaceLegEndSpeedDelta = mRaceAverageSpeed - activeLeg->raceLegEndDriveAvgSpeed;    
  //redo the same time delta calculations as above, only for the entire race distance instead of 
  //just the current leg.

  targetTime=mRaceDistanceComplete / activeLeg->raceLegEndDriveAvgSpeed;
  mRaceTimeDelta=mRaceTime-targetTime;

  //This sets the color for the keypad buttons based on our speed delta.  Green if we are in-band.
  //Blue if we are slow and red if we are fast.  Current only works on the leg speedDelta.  Will
  //eventually make it a configuration setting to use either leg, or race.
  if(speedBandSource==1) {
    speedDelta=mLegSpeedDelta;
  } else {
    speedDelta=mRaceLegEndSpeedDelta;
  }
  
  if(speedBandSource!=0) {
    if(speedDelta<(activeLeg->speedRange*-1.0)) {
      stateMachine.status.flags.buttonColor=1;
    } else if (speedDelta>activeLeg->speedRange) {
      stateMachine.status.flags.buttonColor=2;
    } else {
      stateMachine.status.flags.buttonColor=3;
    }
  } else {
    stateMachine.status.flags.buttonColor=0;
  }

  if(activePoint!=activeLeg->points.end()) {
    if(mLegDistanceComplete > (*(activePoint))->distance) {
      activePoint++;
    }
  }
}

void raceData::raceInProgress(bool state){
  mRaceInProgress=state;
};

void raceData::startMark(double mark, units_t units) {
    if(units==seconds) {
        mStartMark=TIME_SECONDS_TO_INTERNAL(mark);
    } else if (units==milliseconds) {
        mStartMark=mark;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
  mStartMark=mark;
};

void raceData::legAdjustedTargetSpeed(double speed, units_t units) {
  if(units==imperial) {
    mLegAdjustedTargetSpeed=SPEED_MPH_TO_INTERNAL(speed);
  } else {
    mLegAdjustedTargetSpeed=SPEED_KPH_TO_INTERNAL(speed);
  }
};

void raceData::legTargetTime(double targetTime, units_t units) {
    if(units==seconds) {
        mLegTargetTime=TIME_SECONDS_TO_INTERNAL(targetTime);
    } else if (units==milliseconds) {
        mLegTargetTime=targetTime;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
};

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

void raceSetup(void) {
  delayedStartEvent=new event_t(raceLegStartEvent, eventSingle, false, false, 0, 0, &Serial, "delayedStartEvent");
  loadRaces();
}