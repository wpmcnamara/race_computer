#include <list>
#include <string>
#include <stdint.h>
#include <ArduinoJson.h>
#include <CSV_Parser.h>
#include "racePoint.h"
#include "raceLegDef.h"
#include "storage.h"

racePoint::racePoint() {
    mDistance = 0;
    mSpeed = 0;
    mTurn = 0;
    mTurnDir = false;
    mDescrLine1 = "";
    mDescrLine2 = "";
    mTimeToPoint = 0;
    mDistToPoint = 0;
    mId = 0;
}

double racePoint::distance(units_t units) {
    if (units == imperial || units == miles) {
        return DISTANCE_INTERNAL_TO_MILES(mDistance);
    } else if (units == metric || units == km) {
        return DISTANCE_INTERNAL_TO_KILOMETERS(mDistance);
    } else if (units == internal) {
        return mDistance;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void racePoint::distance(double value, units_t units) {
    if (units == imperial || units == miles) {
        mDistance = DISTANCE_MILES_TO_INTERNAL(value);
    } else if (units == metric || units == km) {
        mDistance = DISTANCE_KILOMETERS_TO_INTERNAL(value);
    } else if (units == internal) {
        mDistance = value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double racePoint::speed(units_t units) {
    if (units == imperial || units == mph) {
        return SPEED_INTERNAL_TO_MPH(mSpeed);
    } else if (units == metric || units == kph) {
        return SPEED_INTERNAL_TO_KPH(mSpeed);
    } else if (units == internal) {
        return mSpeed;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void racePoint::speed(double value, units_t units) {
    if (units == imperial || units == mph) {
        mSpeed = SPEED_MPH_TO_INTERNAL(value);
    } else if (units == metric || units == kph) {
        mSpeed = SPEED_KPH_TO_INTERNAL(value);
    } else if (units == internal) {
        mSpeed = value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

int racePoint::turn(void) {
    return mTurn;
}

void racePoint::turn(int value) {
    mTurn = value;
}

bool racePoint::turnDir(void) {
    return mTurnDir;
}

void racePoint::turnDir(bool value) {
    mTurnDir = value;
}

String racePoint::descrLine1(void) {
    return mDescrLine1;
}

void racePoint::descrLine1(String value) {
    mDescrLine1 = value;
}

String racePoint::descrLine2(void) {
    return mDescrLine2;
}

void racePoint::descrLine2(String value) {
    mDescrLine2 = value;
}

double racePoint::timeToPoint(units_t units) {
    if (units == seconds) {
        return TIME_INTERNAL_TO_SECONDS(mTimeToPoint);
    } else if (units == milliseconds || units == internal) {
        return mTimeToPoint;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void racePoint::timeToPoint(double value, units_t units) {
    if (units == seconds) {
        mTimeToPoint = TIME_SECONDS_TO_INTERNAL(value);
    } else if (units == milliseconds || units == internal) {
        mTimeToPoint = value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

double racePoint::distToPoint(units_t units) {
    if (units == imperial || units == miles) {
        return DISTANCE_INTERNAL_TO_MILES(mDistToPoint);
    } else if (units == metric || units == km) {
        return DISTANCE_INTERNAL_TO_KILOMETERS(mDistToPoint);
    } else if (units == internal) {
        return mDistToPoint;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

void racePoint::distToPoint(double value, units_t units) {
    if (units == imperial || units == miles) {
        mDistToPoint = DISTANCE_MILES_TO_INTERNAL(value);
    } else if (units == metric || units == km) {
        mDistToPoint = DISTANCE_KILOMETERS_TO_INTERNAL(value);
    } else if (units == internal) {
        mDistToPoint = value;
    } else {
        Serial.printf("Invalid units passed to %s\n", __func__);
        while(1);
    }
}

unsigned int racePoint::id(void) {
    return mId;
}

void racePoint::id(unsigned int value) {
    mId = value;
}

void loadRacePoints(raceLegDef_t *raceLeg) {
  racePoint_t *point;
  char path[256];

  CSV_Parser cp(/*format*/ "udsfss", /*has_header*/ true, /*delimiter*/ ',');
  sprintf(path, "orrc/races/%s", raceLeg->pointsFile().c_str());
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
    point=new racePoint;
    point->id(row);
    Serial.printf(" point %d\n", point->id());
    point->turn(turn[row]);
    Serial.printf("   turn: %d\n", point->turn());
    if(strcmp(dir[row], "Right")==0) {
      point->turnDir(true);
    } else {
      point->turnDir(false);
    }
    Serial.printf("   dir: %d\n", point->turnDir());
    point->distance(distance[row], miles);
    Serial.printf("   distance: %fmi, %lfum\n", distance[row], point->distance(internal));
    point->descrLine1(descr1[row]);
    Serial.printf("   descr1: %s\n", point->descrLine1().c_str());
    point->descrLine2(descr2[row]);
    Serial.printf("   descr2: %s\n", point->descrLine2().c_str());  
    raceLeg->points.push_back(point);     
  }  
}

void clearRacePoints(raceLegDef_t *raceLeg) {
  racePoint_t *content;
  for (std::vector<racePoint_t *>::iterator it=raceLeg->points.begin(); it != raceLeg->points.end(); ++it) {
    //We could get interuptted in the middle of clean up, so we save the pointer and null the iterator 
    //reference before we delete the object.  
    content=*it;
    *it=NULL;
    delete content;
  }
  raceLeg->points.clear();
  race.activePoint=raceLeg->points.end();
}