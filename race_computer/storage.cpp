#include "storage.h"
#include "display.h"
#include <ArduinoJson.h>
#include "keypad.h"
//setup global objects on the board.
// set up variables using the SD utility library functions:
SdFat32 sdCard;
//SdVolume volume;
bool sdCardPresent;

#define SD_CONFIG SdSpiConfig(SDCARD_CS, DEDICATED_SPI, SPI_SPEED)

void storageSetup(void) {
  uint32_t size;
  uint32_t sizeMB;
  doSPILock();
  if (!sdCard.begin(SdSpiConfig(SDCARD_CS, DEDICATED_SPI, SD_SCK_MHZ(4), &SPI1))) {
  //if (!sdCard.begin(SDCARD_CS, SPI_SPEED)) {  
  //if (!sdCard.begin(SD_CONFIG)) {  
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
  doc["adjustLegTime"]=autoAdjustLegTime;
  doc["speedBandSource"]=speedBandSource;
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
  if(!doc["adjustLegTime"].isNull()) {  
    autoAdjustLegTime=doc["adjustLegTime"].as<bool>();
    Serial.printf("Auto Adjust Leg Time=%d\n", autoAdjustLegTime);
  }
  if(!doc["speedBandSource"].isNull()) {  
    speedBandSource=doc["speedBandSource"].as<bool>();
    Serial.printf("Speed Band Comparison Source=%d\n", speedBandSource);
  }  

};

void logRace(raceData_t *race, uint8_t type) {
  bool ret;
  bool writeHeader=false;
  char * buffer;
  char legStr[]="leg";
  char raceStr[]="race";
  const char *descr=nullptr;
  char *entryType;
  float targetTime=0;
  float targetSpeed=0;
  float targetDistance=0;
  float driveDistance=0;
  float driveSpeed=0;
  float adjustedTargetTime=0;
  float adjustedDriveSpeed=0;
  float eolTargetTime=0;
  float eolTargetSpeed=0;
  float eolTargetDistance=0;
  float eolDriveDistance=0;
  float eolDriveAvgSpeed=0;
  float targetDistanceComplete=0;
  float driveDistanceComplete=0;
  float actualDistanceComplete=0;
  float distanceComplete=0;
  float distanceRemaining=0;
  float eolDistanceRemaining=0;
  float targetTimeComplete=0;
  float timeComplete=0;
  float time=0;
  float averageSpeed;
  float adjustedAverageSpeed;
  float speedDelta;
  float eolSpeedDelta;
  float timeDelta;
  float startTs;
  float endTs;

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
    logFile.write("type,description,targetTime,targetSpeed,targetDistance,driveDistance,driveSpeed,adjustedTargetTime,adjustedDriveSpeed,EOL targetTime,EOL targetSpeed,EOL targetDistance,\
      EOL driveDistance,EOL driveAvgSpeed,targetDistanceComplete,driveDistanceComplete,actualDistancceComplete,distanceComplete,distanceRemaining,EOL distanceRemainging,\
      targetTimeComplete,timeComplete,time,averageSpeed,adjustedAverageSpeed,speedDelta,EOL speedDelta,timeDelta,startTS,endTS\n");
  }
  
  if(type==0) {
    entryType=legStr;
    descr=race->activeLeg->descr.c_str();
    targetTime=TIME_INTERNAL_TO_SECONDS(race->activeLeg->targetTime);
    targetSpeed=SPEED_INTERNAL_TO_MPH(race->activeLeg->speed);
    targetDistance=DISTANCE_INTERNAL_TO_MILES(race->activeLeg->distance);
    driveDistance=DISTANCE_INTERNAL_TO_MILES(race->activeLeg->driveDistance);
    driveSpeed=SPEED_INTERNAL_TO_MPH(race->activeLeg->driveSpeed);
    adjustedTargetTime=TIME_INTERNAL_TO_SECONDS(race->legTargetTime);
    adjustedDriveSpeed=SPEED_INTERNAL_TO_MPH(race->legAdjustedTargetSpeed);
    eolTargetTime=targetTime;
    eolTargetSpeed=targetSpeed;
    eolTargetDistance=targetDistance;
    eolDriveDistance=driveDistance;
    eolDriveAvgSpeed=driveSpeed;
    targetDistanceComplete=targetDistance;
    driveDistanceComplete=driveDistance;
    actualDistanceComplete=DISTANCE_INTERNAL_TO_MILES(race->legDistanceComplete);
    distanceComplete=actualDistanceComplete;
    distanceRemaining=DISTANCE_INTERNAL_TO_MILES(race->legDistanceRemaining);
    eolDistanceRemaining=distanceRemaining;
    time=TIME_INTERNAL_TO_SECONDS(race->legTime);
    targetTimeComplete=TIME_INTERNAL_TO_SECONDS(race->activeLeg->targetTime);
    timeComplete=time;
    averageSpeed=SPEED_INTERNAL_TO_MPH(race->legAverageSpeed);
    adjustedAverageSpeed=SPEED_INTERNAL_TO_MPH(race->activeLeg->distance/race->legTime);
    speedDelta=SPEED_INTERNAL_TO_MPH(race->legSpeedDelta);
    eolSpeedDelta=speedDelta;
    timeDelta=TIME_INTERNAL_TO_SECONDS(race->legTimeDelta);
    startTs=TIME_INTERNAL_TO_SECONDS(race->startTs);
    endTs=TIME_INTERNAL_TO_SECONDS(race->endTs);
  } else {
    entryType=raceStr;
    descr=race->activeRace->descr.c_str();
    targetTime=TIME_INTERNAL_TO_SECONDS(race->activeRace->targetTime);
    targetSpeed=SPEED_INTERNAL_TO_MPH(race->activeRace->speed);
    targetDistance=DISTANCE_INTERNAL_TO_MILES(race->activeRace->distance);
    driveDistance=DISTANCE_INTERNAL_TO_MILES(race->activeRace->driveDistance);
    driveSpeed=SPEED_INTERNAL_TO_MPH(race->activeRace->driveSpeed);
    adjustedTargetTime=targetTime;
    adjustedDriveSpeed=driveSpeed;
    eolTargetTime=TIME_INTERNAL_TO_SECONDS(race->activeLeg->raceLegEndTargetTime);
    eolTargetSpeed=SPEED_INTERNAL_TO_MPH(race->activeLeg->raceLegEndAvgSpeed);
    eolTargetDistance=DISTANCE_INTERNAL_TO_MILES(race->activeLeg->raceLegEndTargetDistance);
    eolDriveDistance=DISTANCE_INTERNAL_TO_MILES(race->activeLeg->raceLegEndDriveDistance);
    eolDriveAvgSpeed=SPEED_INTERNAL_TO_MPH(race->activeLeg->raceLegEndDriveAvgSpeed);
    targetDistanceComplete=DISTANCE_INTERNAL_TO_MILES(race->raceTargetDistanceComplete);
    driveDistanceComplete=DISTANCE_INTERNAL_TO_MILES(race->raceDriveDistanceComplete);
    actualDistanceComplete=DISTANCE_INTERNAL_TO_MILES(race->raceActualDistanceComplete);
    distanceComplete=DISTANCE_INTERNAL_TO_MILES(race->raceDistanceComplete);
    distanceRemaining=DISTANCE_INTERNAL_TO_MILES(race->raceDistanceRemaining);
    eolDistanceRemaining=DISTANCE_INTERNAL_TO_MILES(race->raceLegEndDistanceRemaining);
    time=TIME_INTERNAL_TO_SECONDS(race->raceTime);
    targetTimeComplete=TIME_INTERNAL_TO_SECONDS(race->raceTargetTimeComplete);
    timeComplete=TIME_INTERNAL_TO_SECONDS(race->raceTimeComplete);
    averageSpeed=SPEED_INTERNAL_TO_MPH(race->raceAverageSpeed);
    adjustedAverageSpeed=SPEED_INTERNAL_TO_MPH(race->activeLeg->raceLegEndTargetDistance/race->raceTime);
    speedDelta=SPEED_INTERNAL_TO_MPH(race->raceSpeedDelta);
    eolSpeedDelta=SPEED_INTERNAL_TO_MPH(race->raceLegEndSpeedDelta);
    timeDelta=TIME_INTERNAL_TO_SECONDS(race->raceTimeDelta);
    startTs=0;
    endTs=0;
  }
  buffer=(char *)malloc(384);
  snprintf(buffer, 384, "%s,%s,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f,%0.03f\n",
    entryType,
    descr,
    targetTime,
    targetSpeed,
    targetDistance,
    driveDistance,
    driveSpeed,
    adjustedTargetTime,
    adjustedDriveSpeed,
    eolTargetTime,
    eolTargetSpeed,
    eolTargetDistance,
    eolDriveDistance,
    eolDriveAvgSpeed,
    targetDistanceComplete,
    driveDistanceComplete,
    actualDistanceComplete,
    distanceComplete,
    distanceRemaining,
    eolDistanceRemaining,
    targetTimeComplete,
    timeComplete,
    time,
    averageSpeed,
    adjustedAverageSpeed,
    speedDelta,
    eolSpeedDelta,
    timeDelta,
    startTs,
    endTs
  );
  logFile.write(buffer, strlen(buffer));
  logFile.sync();
  logFile.close();

  doSPIUnlock();
  free(buffer);
}

void resetSettings(void) {
  bool ret;
  if(sdCard.exists("orrc/system/settings.yml")) {
    Serial.println("clearing saved settings");
    doSPILock();
    ret=sdCard.remove("orrc/system/settings.yml");
    doSPIUnlock();
    if(ret) {
      Serial.println("success");
    } else {
      Serial.println("fail");
      sdCard.errorPrint(&Serial);
      printSdErrorText(&Serial, sdCard.sdErrorCode());
    }
  }
}