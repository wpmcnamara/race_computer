#include "storage.h"

//setup global objects on the board.
// set up variables using the SD utility library functions:
Sd2Card sdCard;
SdVolume volume;
SdFile root;

void storageSetup(void) {
if (!sdCard.init(1000000, SDCARD_CS)) {    
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    //return;
  } else {
   Serial.println("Wiring is correct and a card is present.");
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
    //return;
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


}