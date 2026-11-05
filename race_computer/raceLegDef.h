#ifndef __RACE_LEG_DEF__
#define __RACE_LEG_DEF__
#include <Arduino.h>
#include <list>
#include <string>
#include <vector>
#include <stdint.h>
#include "helpers.h"
#include "racePoint.h"

typedef class raceLegDef raceLegDef_t;
extern std::vector<raceLegDef_t *>::iterator selectedRaceLeg;

class raceLegDef {
  public:
    raceLegDef();

    //unit: mm/ms  target speed for this leg, based on published leg speed.
    double speed(units_t units);
    void speed(double value, units_t units);

    //unit: mm/ms  target speed, scaled for driving distance of the leg.  The will be the actual speed goal when
    //driving the leg.
    double driveSpeed(units_t units);
    void driveSpeed(double value, units_t units);

    //unit: mm/ms  target race average speed at the end of the leg.  Calculated from all preceding legs.  Allows us to
    //properly calculate race time deltas during a race where leg target speeds vary.
    double raceLegEndAvgSpeed(units_t units);
    void raceLegEndAvgSpeed(double value, units_t units);

    //unit: mm/ms  target race average speed, adjusted to driving distance at the end of the current leg;
    double raceLegEndDriveAvgSpeed(units_t units);
    void raceLegEndDriveAvgSpeed(double value, units_t units);

    //unit: mm  running total of published target distance for the race at the end of the current leg.
    double raceLegEndTargetDistance(units_t units);
    void raceLegEndTargetDistance(double value, units_t units);

    //unit: mm  running total of the driving distance for the race at the end of the current leg.
    double raceLegEndDriveDistance(units_t units);
    void raceLegEndDriveDistance(double value, units_t units);

    //unit ms  running total for the on pace target race time at the end of the current leg.
    double raceLegEndTargetTime(units_t units);
    void raceLegEndTargetTime(double value, units_t units);

    //unit: mm/ms  How close to target is considered to be "on target".  
    double speedRange(units_t units);
    void speedRange(double value, units_t units);

    //unit: mm  published leg distance
    double distance(units_t units);
    void distance(double value, units_t units);

    //unit: mm  actual distance driven for leg.  Used to properly calculated target speed.
    double driveDistance(units_t units);
    void driveDistance(double value, units_t units);

    //unit: ms  target time to complete the leg.
    double targetTime(units_t units);
    void targetTime(double value, units_t units);

    unsigned int id(void);
    void id(unsigned int value);

    String descr(void);
    void descr(String value);

    String pointsFile(void);
    void pointsFile(String value);

    //unit: ms  timing mark for GPS synchronized start.
    double mark(units_t units);
    void mark(double value, units_t units);

    bool inProgress(void);
    void inProgress(bool value);

    bool complete(void);
    void complete(bool value);

    std::vector<racePoint_t *> points;

  private:
    double mSpeed;
    double mDriveSpeed;
    double mRaceLegEndAvgSpeed;
    double mRaceLegEndDriveAvgSpeed;
    double mRaceLegEndTargetDistance;
    double mRaceLegEndDriveDistance;
    double mRaceLegEndTargetTime;
    double mSpeedRange;
    double mDistance;
    double mDriveDistance;
    double mTargetTime;
    unsigned int mId;
    String mDescr;
    String mPointsFile;
    double mMark;
    bool mInProgress;
    bool mComplete;
};

#endif