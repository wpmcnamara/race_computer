#include <list>
#include <string>
#include <stdint.h>
#include <ArduinoJson.h>
#include <CSV_Parser.h>
#include "racePoint.h"
#include "raceLegDef.h"
#include "storage.h"

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
    Serial.printf("   distance: %fmi, %lfum\n", distance[row], point->distance);
    point->descrLine1=descr1[row];
    Serial.printf("   descr1: %s\n", point->descrLine1.c_str());
    point->descrLine2=descr2[row];
    Serial.printf("   descr2: %s\n", point->descrLine2.c_str());  
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