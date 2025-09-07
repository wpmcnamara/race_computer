#ifndef __GPS__
#define __GPS__

#include <SPI.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> 
#include "bsp.h"
#include "helpers.h"

#define PPS_FREQUENCY 1000

//GPS
extern SFE_UBLOX_GNSS gps;

struct gpsDataStruct {
  double lat;
  double lon;
  int siv;
  uint8_t fix;
  int hour;
  int minute;
  int second;
  long speed;
  double avgSpeed;
  unsigned long distance;
};

void gpsSetup(void);
void TIMTM2dataCallback(UBX_TIM_TM2_data_t *);
void gpsUpdate(void);
struct gpsDataStruct *getGpsData(void);
UBX_TIM_TM2_data_t *getGpsTimestamp(void);
void gpsODOcallback(UBX_NAV_ODO_data_t *);
void gpsNAVcallback(UBX_NAV_PVT_data_t *ubxDataStruct);

#endif