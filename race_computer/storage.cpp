#include "storage.h"
#include "display.h"
#include <ArduinoJson.h>
//setup global objects on the board.
// set up variables using the SD utility library functions:
Sd2Card sdCard;
SdVolume volume;
bool sdCardPresent;

void storageSetup(void) {
  if (!sdCard.init(SPI_SPEED, SDCARD_CS)) {    
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    sdCardPresent=false;
    return;
  } else {
   Serial.println("Wiring is correct and a card is present.");
   sdCardPresent=true;
  }

  // print the type of card
  Serial.print("\nCard type: ");
  switch(sdCard.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(sdCard)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    sdCardPresent=false;
    return;
  } else {
    // print the type and size of the first FAT-type volume
    uint32_t volumesize;
    Serial.print("\nVolume type is FAT");
    Serial.println(volume.fatType(), DEC);
    Serial.println();
    
    volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
    volumesize *= volume.clusterCount();       // we'll have a lot of clusters
    if (volumesize < 8388608ul) {
      Serial.print("Volume size (bytes): ");
      Serial.println(volumesize * 512);        // SD card blocks are always 512 bytes
    }
    Serial.print("Volume size (Kbytes): ");
    volumesize /= 2;
    Serial.println(volumesize);
    Serial.print("Volume size (Mbytes): ");
    volumesize /= 1024;
    Serial.println(volumesize);
  }
  SD.begin(SDCARD_CS);
}

void saveSettings(void) {
  JsonDocument doc;
  JsonArray oledDisp;
  File settings;
  if(!sdCardPresent) {
    Serial.println("No SD card.  Unable to checkpoint race");
    return;
  }
  //disable display and GPS use of the SPI bus to prevent collisions
  doSPILock();
  if(!SD.exists("orc")) {
    doSPIUnlock();
    return;
  }

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
  if(SD.exists("orc/settings.dat")) {
    Serial.println("clearing saved settings");
    SD.remove("orc/settings.dat");
  }
  settings=SD.open("orc/settings.dat", FILE_WRITE);
  if(!settings) {
    Serial.println("Settings open failed");
    doSPIUnlock();
    return;
  }
  serializeJsonPretty(doc, settings);
  serializeJsonPretty(doc, Serial);
  Serial.println();
  settings.close();
  doSPIUnlock();

};
void loadSettings(void) {
  JsonDocument doc;
  JsonArray oledDisp;
  File settings;
  DeserializationError error;
  if(!sdCardPresent) {
    Serial.println("No SD card.  Unable to load saved settings");
    return;
  }
  //disable display and GPS use of the SPI bus to prevent collisions
  doSPILock();
  if(!SD.exists("orc/settings.dat")) {
    doSPIUnlock();
    Serial.println("No settings file found.");
    return;
  }  
  Serial.println("Found settings file");
  settings=SD.open("orc/settings.dat");
  error=deserializeJson(doc, settings);
  settings.close();
  doSPIUnlock();
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
