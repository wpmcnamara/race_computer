#include <SPI.h>
#include "gps.h"
#include "timer.h"
#include "event.h"
#include "race.h"
#include "keypad.h"
#include "state_machine.h"
#include "display.h"

//GPS
SFE_UBLOX_GNSS gps;

//global variables
UBX_TIM_TM2_data_t startStopTimeStamp;
UBX_TIM_TM2_data_t timeStamp;

std::vector<double>::iterator speedListInsert;
std::vector<double> speedList;
struct gpsDataStruct gpsData;
event_t *gpsUpdateEvent;

void gpsSetup(void) {
  uint8_t flags;         // Odometer/Low-speed COG filter flags
  uint8_t odoCfg;        // Odometer filter settings
  uint8_t cogMaxSpeed;   // Speed below which course-over-ground (COG) is computed with the low-speed COG filter : m/s * 0.1
  uint8_t cogMaxPosAcc;  // Maximum acceptable position accuracy for computing COG with the low-speed COG filter
  uint8_t velLpGain;     // Velocity low-pass filter level
  uint8_t cogLpGain;     // COG low-pass filter level

  for (int i=0; i<AVG_SPEED_VALUES; i++) {
    speedList.push_back(0.0);
  }
  speedListInsert=speedList.begin();
  //Initialize the global GPS data structure to known values
  gpsData.lat = 0;
  gpsData.lon = 0;
  gpsData.siv = 0;
  gpsData.fix = 0;
  gpsData.gpsTime.hour = 0;
  gpsData.gpsTime.minute = 0;
  gpsData.gpsTime.second = 0;
  gpsData.speed = 0;
  gpsData.distance = 0;

  pinMode(GPS_INT, OUTPUT);
  digitalWrite(GPS_INT, HIGH);
  pinMode(GPS_RESET, OUTPUT);

  //reset GPS module
  digitalWrite(GPS_RESET, LOW);
  delay(50);
  digitalWrite(GPS_RESET, HIGH);
  delay(250);

  if (gps.begin(SPI, GPS_CS, SPI_SPEED) == false)  //Connect to the u-blox module using Wire port
  {
    Serial.println("u-blox GNSS not detected on SPI bus. Please check wiring. Freezing.");
    displayError("GPS not detected");
    while (1);
  }
  Serial.print("NEO-8MU protocol version:");

  gps.getProtocolVersion();
  Serial.print(gps.getProtocolVersionHigh());
  Serial.print(".");
  Serial.println(gps.getProtocolVersionLow());

  gps.getPowerSaveMode(false);
  gps.enableGNSS(true, SFE_UBLOX_GNSS_ID_GPS);
  gps.enableGNSS(true, SFE_UBLOX_GNSS_ID_SBAS);
  gps.enableGNSS(true, SFE_UBLOX_GNSS_ID_GLONASS);
  gps.setPortOutput(COM_PORT_SPI, COM_TYPE_UBX);  //Set the SPI port to output UBX only (turn off NMEA noise)
  gps.setNavigationFrequency(20);                 //Set output to 20 times a second
  gps.setDynamicModel(DYN_MODEL_AUTOMOTIVE);
  gps.setAutoTIMTM2callbackPtr(&TIMTM2dataCallback);


    // Disable the jamming / interference monitor
  UBX_CFG_ITFM_data_t jammingConfig; // Create storage for the jamming configuration
  if (gps.getJammingConfiguration(&jammingConfig)) // Read the jamming configuration
  {
    Serial.print("The jamming / interference monitor is ");
    if (jammingConfig.config.bits.enable == 0) // Check if the monitor is already enabled
      Serial.print("not ");
    Serial.println("enabled");

    if (jammingConfig.config.bits.enable == 1) // Check if the monitor is already enabled
    {
      Serial.print("Disabling the jamming / interference monitor: ");
      (jammingConfig.config.bits.enable = 0); // Enable the monitor
      if (gps.setJammingConfiguration(&jammingConfig)) // Set the jamming configuration
        Serial.println("success");
      else
        Serial.println("failed!");
    }
  }


  // Create storage for the time pulse parameters
  UBX_CFG_TP5_data_t timePulseParameters;

  // Get the time pulse parameters
  if (gps.getTimePulseParameters(&timePulseParameters) == false) {
    Serial.println("getTimePulseParameters failed! Freezing...");
    displayError("TimePulse Param Error");
    while (1);  // Do nothing more
  }

  // Print the CFG TP5 version
  Serial.print("UBX_CFG_TP5 version: ");
  Serial.println(timePulseParameters.version);
  if (gps.setDynamicModel(DYN_MODEL_AUTOMOTIVE) == false)  // Set the dynamic model to PORTABLE
  {
    Serial.println("*** Warning: setDynamicModel failed ***");
  } else {
    Serial.println("Dynamic platform model changed successfully!");
  }

  //By default, the odometer is disabled. We need to enable it.
  //We can enable it using the default settings:
  gps.enableOdometer();

  if (gps.getOdometerConfig(&flags, &odoCfg, &cogMaxSpeed, &cogMaxPosAcc, &velLpGain, &cogLpGain)) {
    flags = UBX_CFG_ODO_USE_ODO;                                                            // Enable the odometer
    odoCfg = UBX_CFG_ODO_CAR;                                                               // Use the car profile (others are RUN, CYCLE, SWIM, CUSTOM)
    Serial.print("cogMaxSpeed: ");
    Serial.println(cogMaxSpeed);
    gps.setOdometerConfig(flags, odoCfg, cogMaxSpeed, cogMaxPosAcc, velLpGain, cogLpGain);  // Set the configuration
  } else {
    Serial.println("Could not read odometer config!");
  }
  gps.resetOdometer();
  gps.setAutoNAVODOcallbackPtr(&gpsODOcallback);  // Enable automatic NAV ODO messages with callback to printODOdata


  timePulseParameters.tpIdx = 1;  // Or we could select the TIMEPULSE2 pin instead, if the module has one

  // We can configure the time pulse pin to produce a defined frequency or period
  // Here is how to set the frequency:

  // While the module is _locking_ to GNSS time, turn off output
  timePulseParameters.freqPeriod = 0;              // Set the frequency/period to 0Hz
  timePulseParameters.pulseLenRatio = 0x55555555;  // Set the pulse ratio to 1/3 * 2^32 to produce 33:67 mark:space

  // When the module is _locked_ to GNSS time, make it generate 1kHz
  timePulseParameters.freqPeriodLock = PPS_FREQUENCY;  // Set the frequency/period to 1kHz
  timePulseParameters.pulseLenRatioLock = 0x80000000;  // Set the pulse ratio to 1/2 * 2^32 to produce 50:50 mark:space

  timePulseParameters.flags.bits.active = 1;          // Make sure the active flag is set to enable the time pulse. (Set to 0 to disable.)
  timePulseParameters.flags.bits.lockedOtherSet = 1;  // Tell the module to use freqPeriod while locking and freqPeriodLock when locked to GNSS time
  timePulseParameters.flags.bits.isFreq = 1;          // Tell the module that we want to set the frequency (not the period)
  timePulseParameters.flags.bits.isLength = 0;        // Tell the module that pulseLenRatio is a ratio / duty cycle (* 2^-32) - not a length (in us)
  timePulseParameters.flags.bits.polarity = 1;        // Tell the module that we want the rising edge at the top of second. (Set to 0 for falling edge.)
  timePulseParameters.flags.bits.lockGnssFreq = 1;
  timePulseParameters.flags.bits.gridUtcGnss = 0;  //Time pulse timestamps on UTC timegrid;
  // Now set the time pulse parameters
  if (gps.setTimePulseParameters(&timePulseParameters) == false) {
    Serial.println("setTimePulseParameters failed!");
  } else {
    Serial.println("Success!");
  }

  gps.setAutoPVTcallbackPtr(&gpsNAVcallback);
  gps.setAutoPVT(true);  //Tell the GNSS to "send" each solution
  gpsUpdateEvent=new event_t(gpsUpdate, eventRepeat, true, false, 0, 10, &Serial, "gpsUpdate");
}

void TIMTM2dataCallback(UBX_TIM_TM2_data_t *ubxDataStruct) {
  double ts;
  unsigned int startDelay;
  unsigned int mark;
  ts= (ubxDataStruct->wnF * 604800) + ubxDataStruct->towMsF;
  if (ubxDataStruct->flags.bits.newFallingEdge) {
    if (!race.legData->inProgress) {
      keysLocked=true;
      //save the start time stamp.  We may adjust this later if we are delaying start to
      //align with a timing mark.
      race.legData->startTs=ts;
      //check to see if we are aligning the start to a timing mark.
      //if not then we just start the race timing on button push.  If we
      //are aligning timing the we have to check the various scenarios to
      //figure out how long to wait before beginning timing.
      if (race.legData->startMark == 0) {
        race.legData->timerOffset = 0;
        raceLegStart();
      } else {
        mark = fmod(ts,race.legData->startMark);
        //calculate the delay before next starting mark, in milliseconds.  This will be invalid if the button
        //was pushed exactly on the current starting mark, but that will be handled in a special case.
        startDelay = race.legData->startMark - mark;
        //calculate the possible timing delay based on whether the button push was
        //exactly on the second or not.
        if (mark == 0) {
          //We managed to push the start button exactly on the timing mark, so start
          //the race.
          raceLegStart();
        } else {
          //delay the start of timing until the next timing mark.  Adjust the start timestamp
          //to align with that mark;
          race.legData->startTs += startDelay;
          race.legData->delayedStart = true;
          stateMachine.status.flags.delayedStart=true;
          race.legData->timerOffset = startDelay;
          //delay routine seems to run about 1.3 to 1.4x slow due to things being done in it.  We scale the
          //start delay appropriately, to get it close to correct.  Actual timing will be correct, we just
          //want the display to look right-ish
          delayedStartEvent->setDelay(startDelay / 13);
          delayedStartEvent->active = true;
        }
      }
    } else {
      raceLegStop();
      race.legData->endTs=ts;
      keysLocked=false;
    }
  }
}

void gpsUpdate(void) {
  if(SPILock) {
    Serial.println("gpsUpdate: SPI Collision");
    return;
  }
  gps.checkUblox();
  gps.checkCallbacks();
}

void gpsODOcallback(UBX_NAV_ODO_data_t *ubxDataStruct) {
  double targetTime;
  double elapsedTime;
  //GPS reports distance in meters.  We convert to millimeters for internal use.
  gpsData.distance = (double)ubxDataStruct->distance*1000.0L;
  //Since we have an updated distamce, if we have a race in progress, then we should update
  //the average race stats since everything is based on time and distance travelled.
  //distance is in meters.  Time is in seconds.
  if (race.legData->inProgress) {
    elapsedTime = getTimeStamp() - race.legData->timerOffset;
    race.legData->timeComplete=elapsedTime;
    race.legData->distance = gpsData.distance - race.legData->distanceOffset;
    race.legData->distanceRemaining = race.legData->driveDistance - race.legData->distance;
    race.legData->averageSpeed = race.legData->distance / elapsedTime;
    //we use the adjustedTargetSpeed here as there isn't a good way to scale things from drive distance
    //to leg distance.  We could technically do it, but the scaling factor is tiny (0.1% as an example)
    //and ensuring no loss of precision isn't worth the hassle.
    race.legData->speedDelta = race.legData->averageSpeed - race.legData->adjustedTargetSpeed;    
    //Calculate how long it should have taken for us to travel the distance we have, at the target speed 
    //for the race.
    targetTime=race.legData->distance / race.legData->adjustedTargetSpeed;
    //Our time delta is the difference between how long it should have taken and how long it did take.
    //Delta will be negative if we are faster, positive if we are slower.
    race.legData->timeDelta=targetTime-elapsedTime;

    //we use the drive values here as it wouldn't make sense to use published values for 
    //the race distance when the leg distance is actual distance driven.  Race values, using
    //published distance are only valid at the end of a leg.
    race.distance = race.actualDistanceComplete + race.legData->distance;
    race.distanceRemaining = race.driveDistance - race.distance;
    race.averageSpeed = race.distance / (race.timeComplete + elapsedTime);
    race.speedDelta = race.averageSpeed - race.activeLeg->raceSpeed;
    //redo the same time delta calculations as above, only for the entire race distance instead of 
    //just the current leg.
    targetTime=race.distance / race.activeLeg->raceSpeed;
    race.timeDelta=targetTime-(race.timeComplete + elapsedTime);

    //This sets the color for the keypad buttons based on our speed delta.  Green if we are in-band.
    //Blue if we are slow and red if we are fast.  Current only works on the leg speedDelta.  Will
    //eventually make it a configuration setting to use either leg, or race.
    if((race.legData->speedDelta)<(race.legData->speedTargetBand*-1.0)) {
      stateMachine.status.flags.buttonColor=1;
    } else if ((race.legData->speedDelta)>race.legData->speedTargetBand) {
      stateMachine.status.flags.buttonColor=2;
    } else {
      stateMachine.status.flags.buttonColor=3;
    }

    if(race.legData->activePoint!=race.activeLeg->points.end()) {
      if(race.legData->distance > (*(race.legData->activePoint))->distance) {
        race.legData->activePoint++;
      }
    }
  }
}

void gpsNAVcallback(UBX_NAV_PVT_data_t *ubxDataStruct) {
  double avgSpeed=0;
  gpsData.fix = ubxDataStruct->fixType;
  gpsData.siv = ubxDataStruct->numSV;
  gpsData.lat = ubxDataStruct->lat / 10000000.0;
  gpsData.lon = ubxDataStruct->lon / 10000000.0;
  gpsData.fixValid=ubxDataStruct->flags.bits.gnssFixOK;
  gpsData.gpsTime.hour = ubxDataStruct->hour;
  gpsData.gpsTime.minute = ubxDataStruct->min;
  gpsData.gpsTime.second = ubxDataStruct->sec;
  gpsData.gpsTime.millis = ubxDataStruct->nano / 1000000;
  gpsData.timeValid=ubxDataStruct->valid.bits.validTime;
  //speed is reported in mm/s.  This is the same value as us/ms that is used for internal
  //speed representation, so we don't have to do any scaling or other conversions.
  //If we aren't actively tracking a leg, then force the speed to zero.  This keeps
  //the speed display from bouncing around at small values due to GPS wander.
  //We maintain a list of the last <n> speed values and calculate the avarage as the
  //current speed.  This is a tradeoff between absolute accurracy and rapid fluctuation of
  //displayed speed due to GPS variation.
  if (race.legData->inProgress) {
    //GPS reports speed in integer mm/s. We need to convert to double mm/ms  
    (*speedListInsert)=((double)ubxDataStruct->gSpeed)/1000.0;
    speedListInsert++;
    if(speedListInsert==speedList.end()) {
      speedListInsert=speedList.begin();
    }
    for (int i=0; i<AVG_SPEED_VALUES; i++) {
      avgSpeed+=speedList[i];
    }
    avgSpeed/=AVG_SPEED_VALUES;
    gpsData.speed = avgSpeed;
    Serial.printf("avg speed: %f\n", gpsData.speed);
  } else {
    gpsData.speed = 0;
  }
  if(gpsData.fix==3 || gpsData.fix==4) {
    stateMachine.status.flags.gpsReady=true;
  } else {
    stateMachine.status.flags.gpsReady=false;
  }
}

struct gpsDataStruct *getGpsData(void) {
  return (&gpsData);
}
orcTime_t *getGpsTime(void) {
  return (&gpsData.gpsTime);
}
