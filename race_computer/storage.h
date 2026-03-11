#ifndef __STORAGE__
#define __STORAGE__


#include "bsp.h"
#include <string.h>
#include "helpers.h"
#include <SdFat.h>

#define SD_FAT_TYPE 1
#define SD_CONFIG SdSpiConfig(SDCARD_CS, SHARED_SPI, SPI_SPEED)

extern SdFat32 sdCard;
extern bool sdCardPresent;

void storageSetup(void);
void saveSettings(void);
void loadSettings(void);

#endif