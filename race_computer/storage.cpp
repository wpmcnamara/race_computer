#include "storage.h"
#include "display.h"
#include <ArduinoJson.h>
//setup global objects on the board.
// set up variables using the SD utility library functions:
SdFat32 sdCard;
//SdVolume volume;
bool sdCardPresent;

void storageSetup(void) {
  uint32_t size;
  uint32_t sizeMB;
  doSPILock();
  if (!sdCard.begin(SDCARD_CS, SPI_SPEED)) {  
    sdCardPresent=false; 
    if (sdCard.card()->errorCode()) { 
      Serial.println("initialization failed. Things to check:");
      Serial.println("* is a card inserted?");
      Serial.println("* is your wiring correct?");
      Serial.println("* did you change the chipSelect pin to match your shield or module?");
      Serial.printf("errorCode: 0x%0X\n",int(sdCard.card()->errorCode()));
      Serial.printf("errorData: 0x%0X\n",int(sdCard.card()->errorData()));
      doSPIUnlock();
      return;
    }
    Serial.println("Card successfully initialized.\n");
    if (sdCard.vol()->fatType() == 0) {
      Serial.println("Can't find a valid FAT16/FAT32/exFAT partition.\n");
      doSPIUnlock();
      return;
    }
    Serial.println("Can't determine error type\n");
    doSPIUnlock();
    return;
  } else {
   Serial.println("Wiring is correct and a card is present.");
   sdCardPresent=true;
  }

  size = sdCard.card()->sectorCount();
  if (size == 0) {
    Serial.println("Can't determine the card size.\n");
    doSPIUnlock();
    return;
  }
  sizeMB = 0.000512 * size + 0.5;
  Serial.printf("Card size: %d\n", sizeMB);
  if (sdCard.fatType() <= 32) {
    Serial.printf("Volume is FAT%d", int(sdCard.fatType()));
  } else {
    Serial.println("Volume is exFAT");
  }
  Serial.printf(", Cluster size (bytes): %d\n", sdCard.vol()->bytesPerCluster());

  Serial.println("Files found (date time size name):");
  sdCard.ls(LS_R | LS_DATE | LS_SIZE);

  if ((sizeMB > 1100 && sdCard.vol()->sectorsPerCluster() < 64) ||
      (sizeMB < 2200 && sdCard.vol()->fatType() == 32)) {
    Serial.println("\nThis card should be reformatted for best performance.");
    Serial.println("Use a cluster size of 32 KB for cards larger than 1 GB.");
    Serial.println("Only cards larger than 2 GB should be formatted FAT32.\n");
    doSPIUnlock();
    return;
  }
  doSPIUnlock();
  return;
}

void saveSettings(void) {
  JsonDocument doc;
  JsonArray oledDisp;
  File32 settings;
  if(!sdCardPresent) {
    Serial.println("No SD card.  Unable to save settings.");
    return;
  }
  //disable display and GPS use of the SPI bus to prevent collisions
  doSPILock();
  if(!sdCard.exists("orrc")) {
    doSPIUnlock();
    return;
  }
  doSPIUnlock();
  Serial.println("Saving settings...");
  doc["displayTimeout"]=displayTimeout;
  doc["oledBrightness"]=oledBrightness;
  doc["ledBrightness"]=ledBrightness;
  oledDisp=doc["oledDisp"].to<JsonArray>();
  oledDisp[0]=OLEDDisplayActive[0];
  oledDisp[1]=OLEDDisplayActive[1];
  oledDisp[2]=OLEDDisplayActive[2];
  oledDisp[3]=OLEDDisplayActive[3];
  doc["ledDisp"]=LEDDisplayActive;
  doSPILock();
  if(sdCard.exists("orrc/system/settings.yml")) {
    Serial.println("clearing saved settings");
    sdCard.remove("orrc/system/settings.yml");
  }
  settings=sdCard.open("orrc/system/settings.yml", FILE_WRITE);
  if(!settings) {
    Serial.println("Settings open failed");
    doSPIUnlock();
    return;
  }
  serializeYml(doc, settings);
  serializeYml(doc, Serial);
  Serial.println();
  settings.close();
  doSPIUnlock();

};

void loadSettings(void) {
  JsonDocument doc;
  JsonArray oledDisp;
  File32 settings;
  DeserializationError error;
  unsigned char *buffer;
  uint32_t len;
  if(!sdCardPresent) {
    Serial.println("No SD card.  Unable to load saved settings");
    return;
  }
  //disable display and GPS use of the SPI bus to prevent collisions
  doSPILock();
  if(!sdCard.exists("orrc/system/settings.yml")) {
    doSPIUnlock();
    Serial.println("No settings file found.");
    return;
  }  
  Serial.println("Found settings file");
  settings=sdCard.open("orrc/system/settings.yml");
  len=settings.fileSize();
  Serial.printf("file size: %d\n", len);
  buffer=(unsigned char *)malloc(len+1);
  if(buffer==0) {
    Serial.println("malloc failed!");
    while(1);
  }
  len=settings.read(buffer, len);
  buffer[len]=0;
  settings.close();
  doSPIUnlock();

  error=deserializeYml(doc, (const char *)buffer);
  free(buffer);
  if (error) {
    Serial.println("deserialization error");
    Serial.println(error.c_str());
    return;
  }
  if(!doc["displayTimeout"].isNull()) {
    displayTimeout=doc["displayTimeout"].as<uint8_t>();
    Serial.printf("displayTimeout=%d\n", displayTimeout);
  }
  if(!doc["oledBrightness"].isNull()) {
    oledBrightness=doc["oledBrightness"].as<uint8_t>();
    Serial.printf("oledBrightness=%d\n", oledBrightness);
    oledDisp1.setContrast(oledBrightness);
    oledDisp2.setContrast(oledBrightness);
    oledDisp3.setContrast(oledBrightness);
    oledDisp4.setContrast(oledBrightness);
  }
  if(!doc["ledBrightness"].isNull()) {
    ledBrightness=doc["ledBrightness"].as<uint8_t>();
    Serial.printf("ledBrightness=%d\n", ledBrightness);
    ledDisp.Set_Brightness(ledBrightness);
  }
  if(!doc["oledDisp"].isNull()) {
    oledDisp=doc["oledDisp"].as<JsonArray>();
    OLEDDisplayActive[0]=oledDisp[0].as<int>();
    Serial.printf("OLED Display 1=%d\n", OLEDDisplayActive[0]);
    OLEDDisplayActive[1]=oledDisp[1].as<int>();
    Serial.printf("OLED Display 2=%d\n", OLEDDisplayActive[1]);
    OLEDDisplayActive[2]=oledDisp[2].as<int>();
    Serial.printf("OLED Display 3=%d\n", OLEDDisplayActive[2]);
    OLEDDisplayActive[3]=oledDisp[3].as<int>();
    Serial.printf("OLED Display 4=%d\n", OLEDDisplayActive[3]);
  }
  if(!doc["ledDisp"].isNull()) {  
    LEDDisplayActive=doc["ledDisp"].as<int>();
    Serial.printf("LED Display=%d\n", LEDDisplayActive);
  }
};

void logRace(raceData_t *race, uint8_t type) {
  bool ret;
  bool writeHeader=false;
  char * buffer;
  char legStr[]="leg";
  char raceStr[]="race";
  char *entryType;
  float targetSpeed=ROUND3(race->targetSpeed*2.23694);
  float adjustedTargetSpeed=ROUND3(race->adjustedTargetSpeed*2.23694);
  float averageSpeed=ROUND3(race->averageSpeed*2.23694);
  float speedDelta=ROUND3(race->speedDelta*2.23694);
  float adjustedTargetTime=ROUND3((race->driveDistance/race->adjustedTargetSpeed));
  float speedTargetBand=ROUND3(race->speedTargetBand*2.23694);

  float totalDistance=ROUND3(race->totalDistance*0.000621372);
  float driveDistance=ROUND3(race->driveDistance*0.000621372);

  float distanceComplete=ROUND3(race->distanceComplete*0.000621372);
  float driveDistanceComplete=ROUND3(race->driveDistanceComplete*0.000621372);
  //double timeComplete;
  float distance=ROUND3(race->distance*0.000621372);
  //double startTime;
  //double endTime;
  double timeDelta=(double)race->timeDelta/1000.0;
  double time=(float (((race->endTs.seconds*1000)+race->endTs.millis) - ((race->startTs.seconds*1000)+race->startTs.millis)))/1000.0;
  File32 logFile;
  if(!sdCardPresent) {
    Serial.println("No SD card.  Unable to log race leg");
    return;
  }
  //disable display and GPS use of the SPI bus to prevent collisions
  doSPILock();
  if(!sdCard.exists("orrc/logs")) {
    ret=sdCard.mkdir("orrc/logs");
    if(!ret) {
      Serial.println("error creating orrc/logs");
      doSPIUnlock();
      return;
    }
  }  
  if(!sdCard.exists("orrc/logs/race_log.csv")) {
    writeHeader=true;
  }
  if(!logFile.open("orrc/logs/race_log.csv", O_WRITE | O_APPEND | O_CREAT)) {
    doSPIUnlock();
    Serial.println("Error opening log file");
    return;
  }
  if(writeHeader) {
    logFile.write("type,targetTime,targetSpeed,adjustedTargetSpeed,adjustedTargetTime,averageSpeed,speedDelta,speedTargetBand,totalDistance,driveDistance,distanceComplete,driveDistanceComplete,timeComplete,distance,startTime,endTime,timeDelta,time\n");
  }
  if(type==0) {
    entryType=legStr;
  } else {
    entryType=raceStr;
  }
  buffer=(char *)malloc(384);
  snprintf(buffer, 384, "%s,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0ld.%03d,%0.3f,%0ld.%03d,%0ld.%03d,%0.3f,%0.3f\n",
                          entryType,
                          ROUND3(race->time),
                          targetSpeed,
                          adjustedTargetSpeed,
                          adjustedTargetTime, //adjustedTargetTime
                          averageSpeed,
                          speedDelta,
                          speedTargetBand,
                          totalDistance,
                          driveDistance,
                          distanceComplete,
                          driveDistanceComplete,
                          race->timeComplete.seconds, //timeComplete
                          race->timeComplete.millis,
                          distance,
                          race->startTs.seconds, //startTime
                          race->startTs.millis,
                          race->endTs.seconds,   //endTime
                          race->endTs.millis,
                          timeDelta,
                          time
                        );
  logFile.write(buffer, strlen(buffer));
  logFile.sync();
  logFile.close();

  doSPIUnlock();
  free(buffer);
}