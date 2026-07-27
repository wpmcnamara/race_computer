#ifndef __RACE_LEG_DEF__
#define __RACE_LEG_DEF__
#include <Arduino.h>
#include <list>
#include <string>
#include <vector>
#include <stdint.h>
#include "racePoint.h"

typedef class raceLegDef raceLegDef_t;
extern std::vector<raceLegDef_t *>::iterator selectedRaceLeg;

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



#endif