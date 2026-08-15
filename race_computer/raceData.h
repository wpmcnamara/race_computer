#ifndef __RACE_DATA__
#define __RACE_DATA__

#include "timer.h"
#include <list>
#include <string>
#include <stdint.h>
#include <YAMLDuino.h>
#include "racePoint.h"
#include "raceDef.h"

typedef class raceData raceData_t;

void raceSetup(void);
void raceLegStart(void);
void raceLegStop(void);
void loadRaces(void);
void setRace(raceDef_t *);
void setLeg(raceLegDef_t *);
void prepRace(void);
void updateRace(void);
//void dumpRaceData(raceData_t *data);
void raceCheckPoint(void);
void loadRaceCheckPoint(void);
void loadRacePoints(raceLegDef_t *raceLeg);
void clearRacePoints(raceLegDef_t *raceLeg);
void loadDefaultRaces(void);
void computeRace(raceDef_t *raceDefinition);

extern bool autoAdjustLegTime;
extern event_t *delayedStartEvent;


//distances are stored in millimeters, 64bit double
//Max distance displayed is 999.999 miles
//times are stored in milliseconds, 64bit double -- generally integer, 52 bits equivelent.
//Max time displayed is 99h 59m 59.999s
//speed are stored as mm/ms, signed 64bit double
//Max speed displayed is 999.999imperial 
//time stamps are stored in milliseconds since GPS epoch of January 6, 1980, 64 bit double.



//all critical values should be kept in internal representation (mm, mm/ms, ms) and only converted at
//input and output.  Use macros for conversion.

//We don't rely on int, long int, etc.  Specify exactly the data size we want to use
//int32_t, double, etc, for anything we care about the precision of.  Normal counters 
//can use int.  All distances, times, speeds, are signed values, even for race definitions
//so we don't have to worry about signed/unsigned conversions and casts.

//We need a wrapper function to use in the event definition for delayed start.  
void raceLegStartEvent(void);


class raceData {
  public:
    raceData();


    //race definition loaded from storage
    raceDef_t *activeRace;
    //active leg within the race definition;
    raceLegDef_t *activeLeg;
    std::vector<racePoint_t *>::iterator activePoint;

    double legTargetTime(units_t units);
    double legAdjustedTargetSpeed(units_t units);
    double legAverageSpeed(units_t units);
    double legSpeedDelta(units_t units);
    double raceSpeedDelta(units_t units);
    double legDistanceRemaining(units_t units);
    double legTimeDelta(units_t units);
    double raceAverageSpeed(units_t units);
    double raceLegEndSpeedDelta(units_t units);
    double raceDistanceComplete(units_t units);
    double raceTargetDistanceComplete(units_t units);    
    double raceDriveDistanceComplete(units_t units);
    double raceActualDistanceComplete(units_t units);
    double raceTimeComplete(units_t units);
    double raceTargetTimeComplete(units_t units);
    double legDistanceComplete(units_t units);
    double legTime(units_t units);
    double raceTime(units_t units);  
    double raceDistanceRemaining(units_t units);
    double raceLegEndDistanceRemaining(units_t units);        
    double distanceOffset(units_t units);
    double startTs(units_t units);
    double endTs(units_t units);
    double startMark(units_t units);
    bool delayedStart(void);
    bool raceInProgress(void);
    bool legInProgress(void);
    double timerOffset(units_t units);
    double raceTimeDelta(units_t units);
    double legAdjustedAverageSpeed(units_t units);
    double raceAdjustedAverageSpeed(units_t units);
    
    void startTs(double ts, units_t units);
    void endTs(double ts, units_t units);
    void delayedStart(bool state);
    void timerOffset(double ts, units_t units);
    void updateRunning(double elapsedTime, double distance);
    void raceInProgress(bool state);
    void adjustedTargetSpeed(double speed, units_t units);
    void startMark(double ts, units_t units);
    void legAdjustedTargetSpeed(double speed, units_t units);
    void legTargetTime(double ts, units_t units);

    void raceLegStart(void);   
    void raceLegStop(void);

    void setRace(raceDef_t *selectedRace);
    void setLeg(raceLegDef_t *selectedRaceLeg);
    void prepRace(void);
    void updateRace(void);
    void raceCheckPoint(void);
    void loadRaceCheckPoint(void);
  private:
    //unit: ms  time to complete the current leg.  We initally load this from the leg definition, but we can't
    //just use the leg definition directly because we might adjust the leg time to correct for the overall
    //race time delta at the beginning of the leg.
    double mLegTargetTime;
    //unit: mm/ms  Actual target speed for the current leg. Value is calculated from adjusted leg target time 
    //and driving distance.  This is the actual target speed to hit when driving a leg.
    double mLegAdjustedTargetSpeed;
    //unit: mm/ms  running average speed for the leg .  This is actively updated from GPS
    //data during the race
    double mLegAverageSpeed;
    //unit: mm/ms  running average speed for the race.  This is actively updated from GPS
    //data during the race    
    double mRaceAverageSpeed;
    //unit: mm/ms  difference between target and average.  We calculate once and store, when 
    //averageSpeed is updated, rather than calculating each time it is used elsewhere.
    double mLegSpeedDelta;
    //unit: mm/ms  difference between target and average.  We calculate once and store, when 
    //averageSpeed is updated, rather than calculating each time it is used elsewhere.
    double mRaceSpeedDelta;    
    //unit: mm/ms  difference between target for the end of the leg and the current average.  
    //We calculate once and store, when     //averageSpeed is updated, rather than calculating 
    //each time it is used elsewhere.
    double mRaceLegEndSpeedDelta;
    //unit: mm  For the active race, this is the sum of the distance of completed legs pluss the distance completed
    //in the current leg. 
    double mRaceDistanceComplete;
    //unit: mm  For the active race, this is the sum of the distance of completed legs.  Only updated at the
    //end of leg. eeded to calculate the whole race average speed.
    double mRaceTargetDistanceComplete;    
    //unit: mm  this is the sum of the drive distance for completed legs. 
    double mRaceDriveDistanceComplete;
    //unit: mm  this is the sum of the disance driven for completed legs based 
    //on the actual distance driven for each leg.  Defined drive distance is a forward looking guess.  
    //This is historical reality.
    double mRaceActualDistanceComplete;
    //unit: ms  For the active race, this is the sum of the times for all completed legs.  Needed to calculate 
    //the whole race average speed. Only updated at the end of a leg.
    double mRaceTimeComplete;
    //unit: mm  For an active race this is the sum of the target time for all completed legs.  This will be the
    //desired target time at the end of each leg and can be used to calculate how far off the pace we are.  This
    //is only adjusted at the end of a leg.
    double mRaceTargetTimeComplete;
    //unit: mm  distance traveled for the leg.  This is updated from the GPS odometer
    double mLegDistanceComplete;
    //unit: ms  running time for the current leg.  This will be updated from the internal timer each time we update
    //the GPS odometer.  At the end of the leg, it will be adjusted from the start and end GPS timestamps.
    double mLegTime;
    //unit: ms  running time for the race.  This will be updated from the internal timer each time we update
    //the GPS odometer.  At the end of the leg, it will be adjusted from the start and end GPS timestamps.
    double mRaceTime;  
    //unit: mm  distance remaining in the current race.  This is based on total drive distance to provide
    //accurate distance to the finish line.
    double mRaceDistanceRemaining;
    //unit: mm  Distance remaining in cumulative race distance to the end of the current leg.  This is based on 
    //total drive distance to provide accurate distance to the finish line.
    double mRaceLegEndDistanceRemaining;    
    //unit: mm  distance remaining in the current leg.  This is based on driveDistance to provide
    //accurate distance to the finish line.
    double mLegDistanceRemaining;    
    //unit: mm  This will be the value of the GPS odometer at the start of the leg.
    double mDistanceOffset;
    //unit: ms  start GPS timestamps for the leg.  
    double mStartTs;
    //unit: ms  end GPS timestamp for the leg. 
    double mEndTs;
    //unit: ms  Holds the modulus value for the start time.  If set, will delay start of race time after 
    //button press until the next startMark second mark.  Allows precise alignment of race timing.
    double mStartMark;
    //flag signifies whether we will be doing a GPS synchronized start.
    bool mDelayedStart;
    //unit: ms  When we have a delayed start, there will be an offset between the internal timer and the 
    //start of timing that we need to account for.
    double mTimerOffset;
    //unit: ms  difference between end of leg target time and current time, based on distance traveled.  
    //Like speedDelta we calculate when GPS data changes and save, rather than calculating every time it is 
    //used.  
    double mRaceTimeDelta;
    //unit: ms  difference between target time and current time, based on distance traveled.  Like speedDelta
    //we calculate when GPS data changes and save, rather than calculating every time it is 
    //used.  
    double mLegTimeDelta;    
    //unit: mm/ms  at the end of a race leg, this will be the average speed based on the published leg distance
    //rather than the actual distance traveled.  
    double mLegAdjustedAverageSpeed;
    //unit: mm/ms  at the end of a race leg, this will be the average race speed based on the cumulative 
    //published leg distances run, rather than the actual distance traveled.  
    double mRaceAdjustedAverageSpeed;
    //signify the race/leg is being actively timed.
    bool mLegInProgress;
    bool mRaceInProgress;

};

extern raceData_t race;

#endif 