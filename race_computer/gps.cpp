#include <SPI.h>
#include "gps.h"
#include "timer.h"

//GPS
SFE_UBLOX_GNSS gps;

//global variables
UBX_TIM_TM2_data_t *startStopTimeStamp=NULL;
UBX_TIM_TM2_data_t timeStamp;

unsigned int speedSamples=0;
unsigned long speedSum=0;

struct gpsDataStruct gpsData;

void gpsSetup(void) {
  pinMode(GPS_INT, OUTPUT);
  digitalWrite(GPS_INT, HIGH);
  pinMode(GPS_RESET, OUTPUT);
  //reset GPS modules
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
  gps.setNavigationFrequency(20); //Set output to 10 times a second
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

  gps.setAutoPVT(true); //Tell the GNSS to "send" each solution
  
}

void TIMTM2dataCallback(UBX_TIM_TM2_data_t *ubxDataStruct)
{
  startStopTimeStamp=ubxDataStruct;
  Serial.println("TIMTM2dataCallback");
  Serial.println();

  Serial.print(F("newFallingEdge: ")); // 1 if a new falling edge was detected
  Serial.print(ubxDataStruct->flags.bits.newFallingEdge);

  Serial.print(F(" newRisingEdge: ")); // 1 if a new rising edge was detected
  Serial.print(ubxDataStruct->flags.bits.newRisingEdge);

  Serial.print(F(" Rising Edge Counter: ")); // Rising edge counter
  Serial.print(ubxDataStruct->count);

  Serial.print(F(" towMsR: ")); // Time Of Week of rising edge (ms)
  Serial.print(ubxDataStruct->towMsR);

  Serial.print(F(" towSubMsR: ")); // Millisecond fraction of Time Of Week of rising edge in nanoseconds
  Serial.print(ubxDataStruct->towSubMsR);

  Serial.print(F(" towMsF: ")); // Time Of Week of falling edge (ms)
  Serial.print(ubxDataStruct->towMsF);

  Serial.print(F(" towSubMsF: ")); // Millisecond fraction of Time Of Week of falling edge in nanoseconds
  Serial.println(ubxDataStruct->towSubMsF);

}

void gpsUpdate(void) {
  gps.checkUblox();
  gps.checkCallbacks();
  //if (gps.getPVT() && (gps.getInvalidLlh() == false))
  if (gps.getPVT())
  {
    gps.getTIMTM2();
    gpsData.lat=(gps.getLatitude())/10000000.0;
    gpsData.lon = (gps.getLongitude())/10000000.0;
    gpsData.siv = gps.getSIV();
    gpsData.fix = gps.getFixType();
    gpsData.hour = gps.getHour();
    gpsData.minute = gps.getHour();
    gpsData.second = gps.getSecond();
    gpsData.speed = gps.getGroundSpeed();

    speedSum += gpsData.speed;
    speedSamples++;
    gpsData.avgSpeed=(double)speedSum/(double)speedSamples;
    memcpy(&timeStamp,&gps.packetUBXTIMTM2->data,sizeof(UBX_TIM_TM2_data_t));
  }
}

struct gpsDataStruct *getGpsData(void) { return(&gpsData); }
UBX_TIM_TM2_data_t *getGpsTimestamp(void) { return(&timeStamp); }
