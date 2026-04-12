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
    //unit: mm/ms  target race average speed at the end of the leg.  Calculated from all preceding legs.  Allows us to
    //properly calculate race time deltas during a race where leg target speeds vary.
    double raceSpeed;
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
    //unit: mm/ms  How close to target is considered to be "on target". Applies to all legs unless
    //overidden in a leg definition 
    double speedRange;
    //unit: mm  Total race distance
    double distance;
    bool inProgress;
    //unit: ms  timing mark for GPS synchronized start.  Applys to all legs unless overidden
    //in a leg definition
    double mark;
    std::vector<raceLegDef_t *> raceLegs;
  private:


};

class raceData {
  public:
    //unit: ms  time to complete the race/leg.  Currently calculated from distance and
    //speed.  Not loaded from definition files.
    double time;
    //unit: mm/ms  target speed for the current race or leg.  
    //Value is loaded from the race definition when the race is selected. For race
    //legs it is loaded when the leg is entered.
    double targetSpeed;
    //unit: mm/ms  Actual target speed for the current race or leg. 
    //Value is calculated from target time and driving distance.  This is the actual target
    //speed to hit when driving a leg.
    double adjustedTargetSpeed;
    //unit: mm/ms  running average speed for the leg/race.  This is actively updated from GPS
    //data during the race
    double averageSpeed;
    //unit: mm/ms  difference between target and average.  We calculate once and store, when 
    //averageSpeed is updated, rather than calculating each time it is used elsewhere.
    double speedDelta;
    //unit: mm/ms  Defines the range, +/-, around the targetSpeed that is considered 
    //within the target speed range.  Used to control the LED color feedback during a
    //race 
    double speedTargetBand;
    //unit: mm  Total distance of the race/leg.  For a race, value is loaded from the
    //race definition when the race is selected.  For legs, it is loaded from the leg
    //definition when the leg is entered.
    double totalDistance;
    //unit: mm  Total drive distance of the course.  For a race, this will be computed as the
    //sum of all leg drive distances.  For a leg, this will be the actual distance to drive 
    //for the leg.  It will be used to calculate the actual target speed and time deltas, in 
    //combination with the totalDistance value.
    double driveDistance;
    //unit: mm  For the active race, this is the sum of the distance of completed legs
    //For a leg, this will be the leg distance.
    //Needed to calculate the whole race average speed.
    double distanceComplete;
    //unit: mm  For the active race, this is the sum of the drive distance for completed legs.
    //For a leg, this will be the defined drive distance one once the leg is complete.
    double driveDistanceComplete;
    //unit: mm  For the active race, this is the sum of the disance driven for completed legs based 
    //on the actual distance driven for each leg.  For a leg, this will be the actual driven distance.
    //we use the actual distance driven, rather than the defined distance for more accurate 
    //timing.  Defined drive distance is a forward looking guess.  This is historical reality.
    double actualDistanceComplete;
    //unit: ms  For the active race, this is the sum of the times for all
    //completed legs.  Needed to calculate the whole race average speed. For a leg this will be
    //the same as leg time.
    double timeComplete;
    //unit: mm  For an active race this is the sum of the target time for all completed legs.  This will be the
    //desired target time at the end of each leg and can be used to calculate how far off the pace we are.  For
    //a leg, it will be the original leg target time before any time adjustments.
    double targetTimeComplete;
    //unit: mm  For race our leg, this the base target time, calculated from published distance and speed.  This
    //will not be adjusted during a race.
    double targetTime;
    //unit: mm  distance traveled for the race/leg.  For a race, this will be the sum of actualDistanceComplete
    //and the distance traveled in the current leg.
    double distance;
    //unit: mm  distance remaining in the current race/leg.  This is based on driveDistance to provide
    //accurate distance to the finish line.
    double distanceRemaining;
    //unit: mm  zero for a race.  For a leg, this will be the value of the GPS odometer at the start
    //of the leg.
    double distanceOffset;
    //unit: us  start GPS timestamps for the leg.  Not used in the race context
    double startTs;
    //unit: us  end GPS timestamp for the leg.  Not used in the race context
    double endTs;
    //unit: ms  Holds the modulus value for the start time.  If set, will delay start of race time after 
    //button press until the next startMark second mark.  Allows precise alignment of race timing.
    double startMark;
    //flag signifies whether we will be doing a GPS synchronized start.
    bool delayedStart;
    //unit: ms  When we have a delayed start, there will be an offset between the internal timer and the 
    //start of timing that we need to account for.
    double timerOffset;
    //unit: ms  difference between target time and current time, based on distance traveled.  Like speedDelta
    //we calculate when GPS data changes and save, rather than calculating every time it is 
    //used.  
    double timeDelta;
    //signify the race/leg is being actively timed.
    bool inProgress;
    //For a race, this points to the raceData structure for the current leg.  For a leg, this
    //is NULL
    raceData_t *legData;
    //race definition loaded from storage
    raceDef_t *activeRace;
    //active leg within the race definition;
    raceLegDef_t *activeLeg;
    std::vector<racePoint_t *>::iterator activePoint;
};

extern raceData_t race;

#endif 