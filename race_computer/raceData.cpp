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
  activeRace->inProgress(true);
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
    } else if (units==milliseconds || units==internal) {
        return mLegTargetTime;
    } else {
        Serial.printf("Invalid units passed to %s: %d\n", __func__, units);
        while(1);
    }
}

double raceData::legAdjustedTargetSpeed(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mLegAdjustedTargetSpeed);
  } else if (units==internal) {
    return mLegAdjustedTargetSpeed;
  } else {
    return SPEED_INTERNAL_TO_KPH(mLegAdjustedTargetSpeed);
  }
}

double raceData::legAverageSpeed(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mLegAverageSpeed);
  } else if (units==internal) {
    return mLegAverageSpeed;
  } else {
    return SPEED_INTERNAL_TO_KPH(mLegAverageSpeed);
  }  
}

double raceData::legSpeedDelta(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mLegSpeedDelta);
  } else if (units==internal) {
    return mLegSpeedDelta;
  } else {
    return SPEED_INTERNAL_TO_KPH(mLegSpeedDelta);
  }  
}

double raceData::raceSpeedDelta(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mRaceSpeedDelta);
  } else if (units==internal) {
    return mRaceSpeedDelta;
  } else {
    return SPEED_INTERNAL_TO_KPH(mRaceSpeedDelta);
  }   
}

double raceData::legDistanceRemaining(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mLegDistanceRemaining);
  } else if (units==internal) {
    return mLegDistanceRemaining;
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mLegDistanceRemaining);
  }  
}

double raceData::legTimeDelta(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mLegTimeDelta);
    } else if (units==milliseconds || units==internal) {
        return mLegTimeDelta;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceData::raceAverageSpeed(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mRaceAverageSpeed);
  } else if (units==internal) {
    return mRaceAverageSpeed;
  } else {
    return SPEED_INTERNAL_TO_KPH(mRaceAverageSpeed);
  }  
}

double raceData::raceLegEndSpeedDelta(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mRaceLegEndSpeedDelta);
  } else if (units==internal) {
    return mRaceLegEndSpeedDelta;
  } else {
    return SPEED_INTERNAL_TO_KPH(mRaceLegEndSpeedDelta);
  }   
}

double raceData::raceDistanceComplete(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mRaceDistanceComplete);
  } else if (units==internal) {
    return mRaceDistanceComplete;
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mRaceDistanceComplete);
  }  
}

double raceData::raceTargetDistanceComplete(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mRaceTargetDistanceComplete);
  } else if (units==internal) {
    return mRaceTargetDistanceComplete;
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mRaceTargetDistanceComplete);
  }  
}

double raceData::raceDriveDistanceComplete(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mRaceDriveDistanceComplete);
  } else if (units==internal) {
    return mRaceDriveDistanceComplete;
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mRaceDriveDistanceComplete);
  }  
}

double raceData::raceActualDistanceComplete(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mRaceActualDistanceComplete);
  } else if (units==internal) {
    return mRaceActualDistanceComplete;
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mRaceActualDistanceComplete);
  }  
}

double raceData::raceTimeComplete(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mRaceTimeComplete);
    } else if (units==milliseconds || units==internal) {
        return mRaceTimeComplete;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceData::raceTargetTimeComplete(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mRaceTargetTimeComplete);
    } else if (units==milliseconds || units==internal) {
        return mRaceTargetTimeComplete;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceData::legDistanceComplete(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mLegDistanceComplete);
  } else if (units==internal) {
    return mLegDistanceComplete;
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mLegDistanceComplete);
  }  
}

double raceData::legTime(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mLegTime);
    } else if (units==milliseconds || units==internal) {
        return mLegTime;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }

}

double raceData::raceTime(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mRaceTime);
    } else if (units==milliseconds || units==internal) {
        return mRaceTime;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceData::raceDistanceRemaining(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mRaceDistanceRemaining);
  } else if (units==internal) {
    return mRaceDistanceRemaining;
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mRaceDistanceRemaining);
  }  
}

double raceData::raceLegEndDistanceRemaining(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mRaceLegEndDistanceRemaining);
  } else if (units==internal) {
    return mRaceLegEndDistanceRemaining;
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mRaceLegEndDistanceRemaining);
  }  
}

double raceData::distanceOffset(units_t units) {
  if(units==imperial) {
    return DISTANCE_INTERNAL_TO_MILES(mDistanceOffset);
  } else if (units==internal) {
    return mDistanceOffset;
  } else {
    return DISTANCE_INTERNAL_TO_KILOMETERS(mDistanceOffset);
  }  
}

double raceData::startTs(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mStartTs);
    } else if (units==milliseconds || units==internal) {
        return mStartTs;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceData::endTs(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mEndTs);
    } else if (units==milliseconds || units==internal) {
        return mEndTs;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double raceData::startMark(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mStartMark);
    } else if (units==milliseconds || units==internal) {
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
    } else if (units==milliseconds || units==internal) {
        return mTimerOffset;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }    
}

double raceData::raceTimeDelta(units_t units) {
    if(units==seconds) {
        return TIME_INTERNAL_TO_SECONDS(mRaceTimeDelta);
    } else if (units==milliseconds || units==internal) {
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
  } else if (units==internal) {
    return mLegAdjustedAverageSpeed;
  } else {
    return SPEED_INTERNAL_TO_KPH(mLegAdjustedAverageSpeed);
  }   
}

double raceData::raceAdjustedAverageSpeed(units_t units) {
  if(units==imperial) {
    return SPEED_INTERNAL_TO_MPH(mRaceAdjustedAverageSpeed);
  } else if (units==internal) {
    return mRaceAdjustedAverageSpeed;
  } else {
    return SPEED_INTERNAL_TO_KPH(mRaceAdjustedAverageSpeed);
  }   
}

//TODO:  move this into the raceDef class as thats where it belongs.
void computeRace (raceDef_t *raceDefinition) {
  std::vector<raceLegDef_t *>::iterator raceLegIt;
  double cumulativeRaceDistance=0;  // internal -- mm
  double cumulativeRaceTime=0;      // internal -- ms
  double cumulativeRaceDriveDistance=0;  // internal -- mm
  raceLegIt=raceDefinition->raceLegs.begin();
  while(raceLegIt!=raceDefinition->raceLegs.end()) {
    if((*raceLegIt)->targetTime(internal)!=0) {
      (*raceLegIt)->driveSpeed((*raceLegIt)->driveDistance(internal)/(*raceLegIt)->targetTime(internal), internal);
    } else {
      //can't have infinite speed, and shouldn't have a leg with zero target time, but just
      //in case, we will force speed to 0 if we have 0 time, as an error condition.
      (*raceLegIt)->driveSpeed(0, internal);
    }
    cumulativeRaceDistance+=(*raceLegIt)->distance(internal);
    cumulativeRaceDriveDistance+=(*raceLegIt)->driveDistance(internal);
    cumulativeRaceTime+=(*raceLegIt)->targetTime(internal);

    if(cumulativeRaceTime!=0) {
      (*raceLegIt)->raceLegEndAvgSpeed(cumulativeRaceDistance/cumulativeRaceTime, internal);   
      (*raceLegIt)->raceLegEndDriveAvgSpeed(cumulativeRaceDriveDistance/cumulativeRaceTime, internal);   
    } else {
      //Like above, we should never end up with a cumulativeRaceTimeValue that is zero, but if
      //we do, force the ending average speed to zero as the error condition. 
      (*raceLegIt)->raceLegEndAvgSpeed(0, internal);
      (*raceLegIt)->raceLegEndDriveAvgSpeed(0, internal);
    }
    
    (*raceLegIt)->raceLegEndTargetDistance(cumulativeRaceDistance, internal);
    (*raceLegIt)->raceLegEndDriveDistance(cumulativeRaceDriveDistance, internal);
    (*raceLegIt)->raceLegEndTargetTime(cumulativeRaceTime, internal);
    raceLegIt++;
  }
  raceDefinition->driveDistance(cumulativeRaceDriveDistance, internal);
  //if you define a silly race with zero time, we force the speeds to zero to keep from
  //breaking the laws of physics with infinite speed.
  if(cumulativeRaceTime!=0) {
    raceDefinition->driveSpeed(cumulativeRaceDriveDistance/cumulativeRaceTime,internal);
    //Sanity check defined race speed against the calculated speed from the cumulative leg average.
    if(abs(raceDefinition->speed(internal)-(cumulativeRaceDistance/cumulativeRaceTime))>0.0001) {
        Serial.println("  race speed mismatch, using calculated value\n");
        raceDefinition->speed(cumulativeRaceDistance/cumulativeRaceTime, internal);
    }
  } else {
    raceDefinition->driveSpeed(0, internal);
    raceDefinition->speed(0, internal);
  }
  if(abs(raceDefinition->targetTime(internal)-cumulativeRaceTime)>0.0001) {
      Serial.println("  race time mismatch, using calculated value\n");
      raceDefinition->targetTime(cumulativeRaceTime, internal);
  }
  if(abs(raceDefinition->distance(internal)-cumulativeRaceDistance)>0.0001) {
      Serial.println("  race distance mismatch, using calculated value\n");
      raceDefinition->distance(cumulativeRaceDistance,internal);
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
    raceFile->distance(doc["distance"].as<float>(),imperial);
    Serial.printf("race distance: %fmi, %0.3fmm\n",doc["distance"].as<float>(), raceFile->distance(internal));       
    raceFile->speed(doc["speed"].as<float>(),imperial);
    Serial.printf("race speed: %fmph, %0.3fmm/ms\n",doc["speed"].as<float>(), raceFile->speed(internal));  
    raceFile->speedRange(doc["speedRange"].as<float>(),imperial);
    Serial.printf("race speed range: %fmph, %0.3fmm/ms\n", doc["speedRange"].as<float>(), raceFile->speedRange(internal));
    raceFile->mark(doc["tmark"].as<int>(),seconds);
    Serial.printf("race mark: %ds, %0.3fms\n", doc["tmark"].as<int>(),raceFile->mark(internal));
    //Rather than doing all the conversions to external units and back to internal and worrying about multiple passes of rounding and
    //an impact on precision, well do the targetTime calcs in internal units.    
    raceFile->targetTime(raceFile->distance(internal)/raceFile->speed(internal), internal);
    Serial.printf("race target time: %0.03fs\n\n",raceFile->targetTime(seconds));
    raceFile->inProgress(false);
    races.push_back(raceFile);

    for (JsonObject jsonLeg : doc["legs"].as<JsonArray>()) {
      raceLegFile=new raceLegDef_t;
      raceLegFile->descr(jsonLeg["descr"].as<String>());
      Serial.printf("   leg descr: %s\n", raceLegFile->descr().c_str());
      raceLegFile->pointsFile(jsonLeg["points"].as<String>());
      Serial.printf("   leg pointsFile: %s\n", raceLegFile->pointsFile().c_str());
      raceLegFile->id(jsonLeg["id"].as<int>());
      Serial.printf("   leg id: %d\n", raceLegFile->id());
      raceLegFile->speed(jsonLeg["speed"].as<float>(), imperial);
      Serial.printf("   leg speed: %0.3fmph, %0.3fmm/ms\n", jsonLeg["speed"].as<float>(), raceLegFile->speed(internal));
      if(jsonLeg["speed_range"].isNull()) {
        //just copying the value over, no need to convert back and forth.  This does presume that both raceLegDef and raceDef store their
        //units the same internally.  It would be silly not to, for the use case of this program.
        raceLegFile->speedRange(raceFile->speedRange(internal), internal);
        Serial.printf("   speed range (from race): %0.3fmph\n", raceLegFile->speedRange(imperial));
      } else {
        raceLegFile->speedRange(jsonLeg["speedRange"].as<float>(), imperial);
        Serial.printf("   speed range: %0.3fmph, %0.3fmm/ms\n", jsonLeg["speedRange"].as<float>(), raceLegFile->speedRange(internal));
      }
      raceLegFile->distance(jsonLeg["distance"].as<float>(), imperial);
      Serial.printf("   leg distance: %0.3fmi, %0.3fmm\n", jsonLeg["distance"].as<float>(), raceLegFile->distance(internal));     
      //just keep all the values internal for calculating the targetTime.  In reality, for values that a used as the basic for calculating
      //other values, we should update the dependent values in their setters and we could do away with this externl (to the class) code.
      //thats a TODO 
      raceLegFile->targetTime(raceLegFile->distance(internal)/raceLegFile->speed(internal), internal);
      if(jsonLeg["driveDistance"].isNull()) {
        //same comment as speedRange above
        raceLegFile->driveDistance(raceLegFile->distance(internal), internal);
        Serial.printf("   drive distance is leg distance: %0.3f\n", raceLegFile->driveDistance(imperial));
      } else {
        raceLegFile->driveDistance(jsonLeg["driveDistance"].as<float>(), imperial);
        Serial.printf("   drive distance: %0.3fmi, %0.3fmm\n", jsonLeg["driveDistance"].as<float>(), raceLegFile->driveDistance(internal));
      }

      if(jsonLeg["tmark"].isNull()) {
        //again, no reason to convert for just copying data.  Same caution as earlier about units.
        raceLegFile->mark(raceFile->mark(internal), internal);
        Serial.printf("   leg mark(from race): %0.3fms\n", raceLegFile->mark(milliseconds));
      } else{
        raceLegFile->mark(jsonLeg["tmark"].as<int>(), seconds);
        Serial.printf("   leg mark: %ds, %0.3fms\n", jsonLeg["tmark"].as<int>(), raceLegFile->mark(milliseconds));
      }
      Serial.println("");
      raceLegFile->inProgress(false);
      raceLegFile->complete(false);

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

  mLegTargetTime=selectedRaceLeg->targetTime(internal);
  mLegAdjustedTargetSpeed=0;
  mLegAverageSpeed=0;
  mLegSpeedDelta=0;
  mLegDistanceComplete=0;
  mLegTime=0;
  mLegDistanceRemaining=selectedRaceLeg->driveDistance(internal);
  mLegTimeDelta=0;
  mLegInProgress=false;

  mDistanceOffset=0;
  mStartTs=0;
  mEndTs=0;
  mStartMark=selectedRaceLeg->mark(internal);
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
    mLegAdjustedTargetSpeed=selectedRaceLeg->driveDistance(internal)/mLegTargetTime;
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

  mLegDistanceRemaining=activeLeg->driveDistance(internal);
  loadRacePoints(activeLeg);
  activePoint=activeLeg->points.begin();
}

//We stopped accumulating distance when the start/stop button was pressed.  We don't get the final
//time for the race until the time mark comes in, meaning the average speed may be wrong by just a
//bit.  We need to do a final set of calculates to update all the running values that depend on time.
//Beyond that, we update final values based on the end of leg data.
void raceData::updateRace(void) {
  activeLeg->complete(true);
  mLegTime=mEndTs-mStartTs;
  //I don't think we can ever end up here with a legTime of 0, but in case we do, we force leg
  //average speed to zero in that case.  Same things for leg adjusted averages speed.
  if(mLegTime!=0) {
    mLegAverageSpeed = mLegDistanceComplete / mLegTime;
  } else {
    mLegAverageSpeed=0;
    mLegAdjustedAverageSpeed=activeLeg->distance(internal)/mLegTime;    
  }
  mLegSpeedDelta = mLegAverageSpeed - mLegAdjustedTargetSpeed;
  mLegTimeDelta=mLegTime-mLegTargetTime;

  mRaceTimeComplete+=mLegTime;
  mRaceTime=mRaceTimeComplete;
  mRaceTargetTimeComplete+=activeLeg->targetTime(internal);
  mRaceTimeDelta=mRaceTimeComplete-mRaceTargetTimeComplete;

  mRaceActualDistanceComplete+=mLegDistanceComplete;
  mRaceDriveDistanceComplete+=activeLeg->driveDistance(internal);
  mRaceTargetDistanceComplete+=activeLeg->distance(internal);
  mRaceDistanceRemaining=activeRace->driveDistance(internal)-mRaceDriveDistanceComplete;
  //both prior race time complete and current leg time would need to be zero, in the above addition
  //but in case we somehow get that case, force race average speed to zero rather than div/0. Same
  //thing for race adjusted average speed.
  if(mRaceTimeComplete!=0) {
    mRaceAverageSpeed=mRaceDistanceComplete/mRaceTimeComplete;
    mRaceAdjustedAverageSpeed=activeLeg->raceLegEndTargetDistance(internal)/mRaceTimeComplete;
  } else {
    mRaceAverageSpeed=0;
    mRaceAdjustedAverageSpeed=0;
  }
  mRaceSpeedDelta = mRaceAverageSpeed - activeRace->driveSpeed(internal);
  mRaceLegEndSpeedDelta = mRaceAverageSpeed - activeLeg->raceLegEndDriveAvgSpeed(internal);        
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
  doc["activeRaceInProgress"]=activeRace->inProgress();
  doc["raceInProgress"]=mRaceInProgress; 
  doc["legId"]=activeLeg->id();
  doc["activeLegInProgress"]=activeLeg->inProgress();
  doc["activeLegComplete"]=activeLeg->complete();

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
      Serial.println((*raceLegIt)->descr());
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
  activeRace->inProgress(doc["activeRaceInProgress"].as<bool>());
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
    } else if (units==milliseconds || units==internal) {
        mStartTs=ts;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
};

void raceData::endTs(double ts, units_t units){
    if(units==seconds) {
        mEndTs=TIME_SECONDS_TO_INTERNAL(ts);
    } else if (units==milliseconds || units==internal) {
        mEndTs=ts;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
};

void raceData::timerOffset(double offset, units_t units){
    if(units==seconds) {
        mTimerOffset=TIME_SECONDS_TO_INTERNAL(offset);
    } else if (units==milliseconds || units==internal) {
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
  mLegDistanceRemaining = activeLeg->driveDistance(internal) - mLegDistanceComplete;
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
  mRaceDistanceRemaining = activeRace->driveDistance(internal) - mRaceDistanceComplete;
  mRaceLegEndDistanceRemaining = activeLeg->raceLegEndDriveDistance(internal) - mRaceDistanceComplete;
  if(mRaceTimeComplete!=0 || elapsedTime!=0) {
    mRaceAverageSpeed = mRaceDistanceComplete / (mRaceTimeComplete+elapsedTime);
  } else {
    mRaceAverageSpeed=0;
  }
  mRaceSpeedDelta = mRaceAverageSpeed - activeRace->driveSpeed(internal);
  mRaceLegEndSpeedDelta = mRaceAverageSpeed - activeLeg->raceLegEndDriveAvgSpeed(internal);    
  //redo the same time delta calculations as above, only for the entire race distance instead of 
  //just the current leg.

  targetTime=mRaceDistanceComplete / activeLeg->raceLegEndDriveAvgSpeed(internal);
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
    if(speedDelta<(activeLeg->speedRange(internal)*-1.0)) {
      stateMachine.status.flags.buttonColor=1;
    } else if (speedDelta>activeLeg->speedRange(internal)) {
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
    } else if (units==milliseconds || units==internal) {
        mStartMark=mark;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
};

void raceData::legAdjustedTargetSpeed(double speed, units_t units) {
  if(units==imperial) {
    mLegAdjustedTargetSpeed=SPEED_MPH_TO_INTERNAL(speed);
  } else if (units==internal) {
    mLegAdjustedTargetSpeed=speed;
  } else {
    mLegAdjustedTargetSpeed=SPEED_KPH_TO_INTERNAL(speed);
  }
};

void raceData::legTargetTime(double targetTime, units_t units) {
    if(units==seconds) {
        mLegTargetTime=TIME_SECONDS_TO_INTERNAL(targetTime);
    } else if (units==milliseconds || units==internal) {
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
  raceDef->speed(29.5, mph);
  raceDef->speedRange(0.5, mph);
  raceDef->distance(12.9, miles);
  raceDef->mark(0,seconds);
  raceDef->inProgress(false);
  races.push_back(raceDef);

  raceLegDef=new raceLegDef_t;
  raceLegDef->descr("Test Drive -- clockwise");
  raceLegDef->pointsFile("");
  raceLegDef->id(1);
  raceLegDef->speed(31.9, imperial);
  raceLegDef->speedRange(0.5, imperial);
  raceLegDef->distance(6.4, imperial);
  raceLegDef->driveDistance(6.4, imperial);
  raceLegDef->mark(0, milliseconds);
  raceLegDef->inProgress(false);
  raceLegDef->complete(false);
  raceDef->raceLegs.push_back(raceLegDef);

  raceLegDef=new raceLegDef_t;
  raceLegDef->descr("Test Drive -- counterclockwise");
  raceLegDef->pointsFile("");
  raceLegDef->id(2);
  raceLegDef->speed(26.5, imperial);
  raceLegDef->speedRange(0.5, imperial);
  raceLegDef->distance(6.5, imperial);
  raceLegDef->driveDistance(6.5, imperial);
  raceLegDef->mark(0, milliseconds);
  raceLegDef->inProgress(false);
  raceLegDef->complete(false);
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