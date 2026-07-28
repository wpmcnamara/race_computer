#ifndef __RACE_DEF__
#define __RACE_DEF__
#include <list>
#include <string>
#include <stdint.h>
#include "helpers.h"
#include "raceLegDef.h"


typedef class raceDef raceDef_t;
extern std::list<raceDef_t *>::iterator selectedRace;
extern std::list<raceDef_t *>::iterator selectedRaceSave;
extern std::list<raceDef_t *> races; 

class raceDef {
  public:
    String fileName;
    String descr;
    //unit: mm/ms  Race target speed

    std::vector<raceLegDef_t *> raceLegs;

    double speed(units_t units);
    double driveSpeed(units_t units);
    double speedRange(units_t units);
    double distance(units_t units);
    double driveDistance(units_t units);
    double targetTime(units_t units);
    double mark(units_t units);
  private:
    double mSpeed;
    //unit: mm/ms  target speed, scaled for driving distance of the race.  The will be the actual speed goal when
    //driving the race.
    double mDriveSpeed;    
    //unit: mm/ms  How close to target is considered to be "on target". Applies to all legs unless
    //overidden in a leg definition 
    double mSpeedRange;
    //unit: mm  Total race distance
    double mDistance;
    //unit : mm Total driving distance of the race
    double mDriveDistance;
    //unit: ms  target completion time for the entire race
    double  mTargetTime;
    bool mInProgress;
    //unit: ms  timing mark for GPS synchronized start.  Applys to all legs unless overidden
    //in a leg definition
    double mMark;

};


#endif