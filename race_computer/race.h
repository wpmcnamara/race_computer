#ifndef __RACE__
#define __RACE__

#include "timer.h"
#include <list>
#include <string>
#include <stdint.h>

#include <YAMLDuino.h>

typedef class racePoint racePoint_t;
typedef class raceLegDef raceLegDef_t;
typedef class raceDef raceDef_t;
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




extern std::list<raceDef_t *>::iterator selectedRace;
extern std::list<raceDef_t *>::iterator selectedRaceSave;
extern std::vector<raceLegDef_t *>::iterator selectedRaceLeg;
extern std::list<raceDef_t *> races; 
extern raceData_t tmpLeg;


//distances are stored in millimeters, 64bit double
//Max distance displayed is 999.999 miles
//times are stored in milliseconds, 64bit double -- generally integer, 52 bits equivelent.
//Max time displayed is 99h 59m 59.999s
//speed are stored as mm/ms, signed 64bit double
//Max speed displayed is 999.999mph 
//time stamps are stored in milliseconds since GPS epoch of January 6, 1980, 64 bit double.



//all critical values should be kept in internal representation (mm, mm/ms, ms) and only converted at
//input and output.  Use macros for conversion.

//We don't rely on int, long int, etc.  Specify exactly the data size we want to use
//int32_t, double, etc, for anything we care about the precision of.  Normal counters 
//can use int.  All distances, times, speeds, are signed values, even for race definitions
//so we don't have to worry about signed/unsigned conversions and casts.

class racePoint {
  public:
    double distance;
    int turn;
    bool turnDir;
    double speed;
    String descrLine1;
    String descrLine2;
    double timeToPoint;
    double distToPoint;
    unsigned int id;
};

class raceLegDef {
  public:
    //unit: mm/ms  target speed for this leg, based on published leg speed.
    double speed;
    //unit: mm/ms  target speed, scaled for driving distance of the leg.  The will be the actual speed goal when
    //driving the leg.
    double driveSpeed;
    //unit: mm/ms  target race average speed at the end of the leg.  Calculated from all preceding legs.  Allows us to
    //properly calculate race time deltas during a race where leg target speeds vary.
    double raceLegEndAvgSpeed;
    //unit: mm/ms  target race average speed, adjusted to driving distance at the end of the current leg;
    double raceLegEndDriveAvgSpeed;
    //unit: mm  running total of published target distance for the race at the end of the current leg.
    double raceLegEndTargetDistance;
    //unit: mm  running total of the driving distance for the race at the end of the current leg.
    double raceLegEndDriveDistance;
    //unit ms  running total for the on pace target race time at the end of the current leg.
    double raceLegEndTargetTime;
    //unit: mm/ms  How close to target is considered to be "on target".  
    double speedRange;
    //unit: mm  published leg distance
    double distance;
    //unit: mm  actual distance driven for leg.  Used to properly calculated target speed.
    double driveDistance;
    //unit: ms  target time to complete the leg.
    double  targetTime;
    unsigned int id;
    String descr;
    String pointsFile;
    //unit: ms  timing mark for GPS synchronized start.
    double mark;
    bool inProgress;
    bool complete;
    std::vector<racePoint_t *> points;
  private:

};

class raceDef {
  public:
    String fileName;
    String descr;
    //unit: mm/ms  Race target speed
    double speed;
    //unit: mm/ms  target speed, scaled for driving distance of the race.  The will be the actual speed goal when
    //driving the race.
    double driveSpeed;    
    //unit: mm/ms  How close to target is considered to be "on target". Applies to all legs unless
    //overidden in a leg definition 
    double speedRange;
    //unit: mm  Total race distance
    double distance;
    //unit : mm Total driving distance of the race
    double driveDistance;
    //unit: ms  target completion time for the entire race
    double  targetTime;
    bool inProgress;
    //unit: ms  timing mark for GPS synchronized start.  Applys to all legs unless overidden
    //in a leg definition
    double mark;
    std::vector<raceLegDef_t *> raceLegs;

    double getSpeed(units_t units);
    double getDriveSpeed(units_t units);
    double getSpeedRange(units_t units);
    double getDistance(units_t units);
    double getDriveDistance(units_t units);
    double getTargetTime(void);
    double getMark(void);
  private:


};

class raceData {
  public:
    //unit: ms  time to complete the current leg.  We initally load this from the leg definition, but we can't
    //just use the leg definition directly because we might adjust the leg time to correct for the overall
    //race time delta at the beginning of the leg.
    double legTargetTime;
    //unit: mm/ms  Actual target speed for the current leg. Value is calculated from adjusted leg target time 
    //and driving distance.  This is the actual target speed to hit when driving a leg.
    double legAdjustedTargetSpeed;
    //unit: mm/ms  running average speed for the leg .  This is actively updated from GPS
    //data during the race
    double legAverageSpeed;
    //unit: mm/ms  running average speed for the race.  This is actively updated from GPS
    //data during the race    
    double raceAverageSpeed;
    //unit: mm/ms  difference between target and average.  We calculate once and store, when 
    //averageSpeed is updated, rather than calculating each time it is used elsewhere.
    double legSpeedDelta;
    //unit: mm/ms  difference between target and average.  We calculate once and store, when 
    //averageSpeed is updated, rather than calculating each time it is used elsewhere.
    double raceSpeedDelta;    
    //unit: mm/ms  difference between target for the end of the leg and the current average.  
    //We calculate once and store, when     //averageSpeed is updated, rather than calculating 
    //each time it is used elsewhere.
    double raceLegEndSpeedDelta;
    //unit: mm  For the active race, this is the sum of the distance of completed legs pluss the distance completed
    //in the current leg. 
    double raceDistanceComplete;
    //unit: mm  For the active race, this is the sum of the distance of completed legs.  Only updated at the
    //end of leg. eeded to calculate the whole race average speed.
    double raceTargetDistanceComplete;    
    //unit: mm  this is the sum of the drive distance for completed legs. 
    double raceDriveDistanceComplete;
    //unit: mm  this is the sum of the disance driven for completed legs based 
    //on the actual distance driven for each leg.  Defined drive distance is a forward looking guess.  
    //This is historical reality.
    double raceActualDistanceComplete;
    //unit: ms  For the active race, this is the sum of the times for all completed legs.  Needed to calculate 
    //the whole race average speed. Only updated at the end of a leg.
    double raceTimeComplete;
    //unit: mm  For an active race this is the sum of the target time for all completed legs.  This will be the
    //desired target time at the end of each leg and can be used to calculate how far off the pace we are.  This
    //is only adjusted at the end of a leg.
    double raceTargetTimeComplete;
    //unit: mm  distance traveled for the leg.  This is updated from the GPS odometer
    double legDistanceComplete;
    //unit: ms  running time for the current leg.  This will be updated from the internal timer each time we update
    //the GPS odometer.  At the end of the leg, it will be adjusted from the start and end GPS timestamps.
    double legTime;
    //unit: ms  running time for the race.  This will be updated from the internal timer each time we update
    //the GPS odometer.  At the end of the leg, it will be adjusted from the start and end GPS timestamps.
    double raceTime;  
    //unit: mm  distance remaining in the current race.  This is based on total drive distance to provide
    //accurate distance to the finish line.
    double raceDistanceRemaining;
    //unit: mm  Distance remaining in cumulative race distance to the end of the current leg.  This is based on 
    //total drive distance to provide accurate distance to the finish line.
    double raceLegEndDistanceRemaining;    
    //unit: mm  distance remaining in the current leg.  This is based on driveDistance to provide
    //accurate distance to the finish line.
    double legDistanceRemaining;    
    //unit: mm  This will be the value of the GPS odometer at the start of the leg.
    double distanceOffset;
    //unit: ms  start GPS timestamps for the leg.  
    double startTs;
    //unit: ms  end GPS timestamp for the leg. 
    double endTs;
    //unit: ms  Holds the modulus value for the start time.  If set, will delay start of race time after 
    //button press until the next startMark second mark.  Allows precise alignment of race timing.
    double startMark;
    //flag signifies whether we will be doing a GPS synchronized start.
    bool delayedStart;
    //unit: ms  When we have a delayed start, there will be an offset between the internal timer and the 
    //start of timing that we need to account for.
    double timerOffset;
    //unit: ms  difference between end of leg target time and current time, based on distance traveled.  
    //Like speedDelta we calculate when GPS data changes and save, rather than calculating every time it is 
    //used.  
    double raceTimeDelta;
    //unit: ms  difference between target time and current time, based on distance traveled.  Like speedDelta
    //we calculate when GPS data changes and save, rather than calculating every time it is 
    //used.  
    double legTimeDelta;    
    //signify the race/leg is being actively timed.
    bool legInProgress;
    bool raceInProgress;
     //race definition loaded from storage
    raceDef_t *activeRace;
    //active leg within the race definition;
    raceLegDef_t *activeLeg;
    std::vector<racePoint_t *>::iterator activePoint;
    double getLegTargetTime(void);
    double getLegAdjustedTargetSpeed(units_t units);
    double getLegAverageSpeed(units_t units);
    double getLegDistanceTraveled(units_t units);
    double getLegSpeedDelta(units_t units);
    double getRaceDeltaSpeed(units_t units);
    double getLegDistanceRemaining(units_t units);
    double getLegTimeDelta(void);
    

};

extern raceData_t race;

#endif 