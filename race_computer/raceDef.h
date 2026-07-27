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


#endif