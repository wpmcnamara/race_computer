#include <SPI.h>
#include "gps.h"
#include "timer.h"

//GPS
SFE_UBLOX_GNSS gps;

//global variables
UBX_TIM_TM2_data_t startStopTimeStamp;
UBX_TIM_TM2_data_t timeStamp;

unsigned int speedSamples=0;
unsigned long speedSum=0;

struct gpsDataStruct gpsData;

void gpsSetup(void) {
  uint8_t flags;        // Odometer/Low-speed COG filter flags
  uint8_t odoCfg;       // Odometer filter settings
  uint8_t cogMaxSpeed;  // Speed below which course-over-ground (COG) is computed with the low-speed COG filter : m/s * 0.1
  uint8_t cogMaxPosAcc; // Maximum acceptable position accuracy for computing COG with the low-speed COG filter
  uint8_t velLpGain;    // Velocity low-pass filter level
  uint8_t cogLpGain;    // COG low-pass filter level

  //Initialize the global GPS data structure to known values
  gpsData.lat=0;
  gpsData.lon=0;
  gpsData.siv=0;
  gpsData.fix=0;
  gpsData.hour=0;
  gpsData.minute=0;
  gpsData.second=0;
  gpsData.speed=0;
  gpsData.avgSpeed=0;
  gpsData.distance=0;

  pinMode(GPS_INT, OUTPUT);
  digitalWrite(GPS_INT, HIGH);
  pinMode(GPS_RESET, OUTPUT);

  //reset GPS module
  digitalWrite(GPS_RESET, LOW);
  delay(50);
  digitalWrite(GPS_RESET, HIGH);
  delay(250);

  if (gps.begin(SPI, GPS_CS, 1000000) == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected on SPI bus. Please check wiring. Freezing."));
    while (1);
  }
  Serial.print(F("NEO-8MU protocol version:"));

  gps.getProtocolVersion();
  Serial.print(gps.getProtocolVersionHigh());
  Serial.print(F("."));
  Serial.println(gps.getProtocolVersionLow());

  gps.getPowerSaveMode();
  
  gps.enableGNSS(true, SFE_UBLOX_GNSS_ID_GPS);
  gps.enableGNSS(true, SFE_UBLOX_GNSS_ID_SBAS);
  gps.enableGNSS(true, SFE_UBLOX_GNSS_ID_GLONASS);
  gps.setPortOutput(COM_PORT_SPI, COM_TYPE_UBX); //Set the SPI port to output UBX only (turn off NMEA noise)
  gps.setNavigationFrequency(20); //Set output to 20 times a second
  gps.setAutoTIMTM2callbackPtr(&TIMTM2dataCallback);
  gps.logTIMTM2(); // Enable TIM TM2 data logging

  // Create storage for the time pulse parameters
  UBX_CFG_TP5_data_t timePulseParameters;

  // Get the time pulse parameters
  if (gps.getTimePulseParameters(&timePulseParameters) == false)
  {
    Serial.println(F("getTimePulseParameters failed! Freezing..."));
    while (1) ; // Do nothing more
  }

  // Print the CFG TP5 version
  Serial.print(F("UBX_CFG_TP5 version: "));
  Serial.println(timePulseParameters.version);
  if (gps.setDynamicModel(DYN_MODEL_AUTOMOTIVE) == false) // Set the dynamic model to PORTABLE
  {
    Serial.println(F("*** Warning: setDynamicModel failed ***"));
  }
  else
  {
    Serial.println(F("Dynamic platform model changed successfully!"));
  }

  //By default, the odometer is disabled. We need to enable it.
  //We can enable it using the default settings:
  gps.enableOdometer();

  if (gps.getOdometerConfig(&flags, &odoCfg, &cogMaxSpeed, &cogMaxPosAcc, &velLpGain, &cogLpGain)) {
    flags = UBX_CFG_ODO_USE_ODO; // Enable the odometer
    odoCfg = UBX_CFG_ODO_CAR; // Use the car profile (others are RUN, CYCLE, SWIM, CUSTOM)
    gps.setOdometerConfig(flags, odoCfg, cogMaxSpeed, cogMaxPosAcc, velLpGain, cogLpGain); // Set the configuration
  } else {
    Serial.println("Could not read odometer config!");
  }
  gps.resetOdometer(); 
  gps.setAutoNAVODOcallbackPtr(&gpsODOcallback); // Enable automatic NAV ODO messages with callback to printODOdata


  timePulseParameters.tpIdx = 1; // Or we could select the TIMEPULSE2 pin instead, if the module has one

  // We can configure the time pulse pin to produce a defined frequency or period
  // Here is how to set the frequency:

  // While the module is _locking_ to GNSS time, turn off output
  timePulseParameters.freqPeriod = 0; // Set the frequency/period to 0Hz
  timePulseParameters.pulseLenRatio = 0x55555555; // Set the pulse ratio to 1/3 * 2^32 to produce 33:67 mark:space

  // When the module is _locked_ to GNSS time, make it generate 1kHz
  timePulseParameters.freqPeriodLock = PPS_FREQUENCY; // Set the frequency/period to 1kHz
  timePulseParameters.pulseLenRatioLock = 0x80000000; // Set the pulse ratio to 1/2 * 2^32 to produce 50:50 mark:space

  timePulseParameters.flags.bits.active = 1; // Make sure the active flag is set to enable the time pulse. (Set to 0 to disable.)
  timePulseParameters.flags.bits.lockedOtherSet = 1; // Tell the module to use freqPeriod while locking and freqPeriodLock when locked to GNSS time
  timePulseParameters.flags.bits.isFreq = 1; // Tell the module that we want to set the frequency (not the period)
  timePulseParameters.flags.bits.isLength = 0; // Tell the module that pulseLenRatio is a ratio / duty cycle (* 2^-32) - not a length (in us)
  timePulseParameters.flags.bits.polarity = 1; // Tell the module that we want the rising edge at the top of second. (Set to 0 for falling edge.)
  timePulseParameters.flags.bits.lockGnssFreq =1;
  // Now set the time pulse parameters
  if (gps.setTimePulseParameters(&timePulseParameters) == false)
  {
    Serial.println(F("setTimePulseParameters failed!"));
  }
  else
  {
    Serial.println(F("Success!"));
  }

  gps.setAutoPVTcallbackPtr(&gpsNAVcallback); 
  gps.setAutoPVT(true); //Tell the GNSS to "send" each solution
  
}

void TIMTM2dataCallback(UBX_TIM_TM2_data_t *ubxDataStruct)
{
  //startStopTimeStamp=ubxDataStruct;
  memcpy(&startStopTimeStamp, ubxDataStruct, sizeof(UBX_TIM_TM2_data_t));
}

void gpsUpdate(void) {
  gps.checkUblox();
  gps.checkCallbacks();
  if (gps.getPVT())
  {

    gpsData.siv = gps.getSIV();
    gpsData.fix = gps.getFixType();

    memcpy(&timeStamp,&gps.packetUBXTIMTM2->data,sizeof(UBX_TIM_TM2_data_t));
  }
}

void gpsODOcallback(UBX_NAV_ODO_data_t *ubxDataStruct) {
  double tickHold;
  gpsData.distance = ubxDataStruct->distance; 
  //Since we have an updated difference, if we have a running timer, then we should update 
  //the average speed.
  //we want ticks in seconds rather than milliseconds since distance is in meters and we 
  //can calculate speed in m/s then convert to mph.
  tickHold=(double)getTick()/1000.0;
  if(tickHold!=0) {
    gpsData.avgSpeed=(double)gpsData.distance/tickHold;
  }
}

void gpsNAVcallback(UBX_NAV_PVT_data_t *ubxDataStruct) {
  gpsData.lat=ubxDataStruct->lat/10000000.0;
  gpsData.lon=ubxDataStruct->lon/10000000.0;
  gpsData.hour = ubxDataStruct->hour;
  gpsData.minute = ubxDataStruct->min;
  gpsData.second = ubxDataStruct->sec;
  gpsData.speed = ubxDataStruct->gSpeed;
}

struct gpsDataStruct *getGpsData(void) { return(&gpsData); }
UBX_TIM_TM2_data_t *getGpsTimestamp(void) { return(&timeStamp); }
