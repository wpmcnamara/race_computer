#ifndef __STORAGE__
#define __STORAGE__


#include "bsp.h"
#include <string.h>
#include "helpers.h"
#include <SdFat.h>
#include <ArduinoYaml.h>
#include <YAMLDuino.h>
#include "raceData.h"

#define SD_FAT_TYPE 1


extern SdFat32 sdCard;
extern bool sdCardPresent;

void storageSetup(void);
void saveSettings(void);
void loadSettings(void);
void resetSettings(void);
void logRace(raceData_t *race, uint8_t type);

#endif