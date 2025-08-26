#ifndef __STORAGE__
#define __STORAGE__

#include <SD.h>

#include "bsp.h"
#include "helpers.h"

//setup global objects on the board.
// set up variables using the SD utility library functions:
extern Sd2Card sdCard;
extern SdVolume volume;
extern SdFile root;

void storageSetup(void);

#endif