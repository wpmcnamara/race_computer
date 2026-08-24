#ifndef __RACE_POINT__
#define __RACE_POINT__
#include <Arduino.h>
#include <string>
#include "helpers.h"

typedef class racePoint racePoint_t;

class racePoint {
  public:
    racePoint();

    //unit: mm  distance from the start of the leg to this point.
    double distance(units_t units);
    void distance(double value, units_t units);

    //unit: mm/ms  target speed at this point.
    double speed(units_t units);
    void speed(double value, units_t units);

    int turn(void);
    void turn(int value);

    bool turnDir(void);
    void turnDir(bool value);

    String descrLine1(void);
    void descrLine1(String value);

    String descrLine2(void);
    void descrLine2(String value);

    //unit: ms  target time to reach this point from the start of the leg.
    double timeToPoint(units_t units);
    void timeToPoint(double value, units_t units);

    //unit: mm  distance remaining to this point from the current position.
    double distToPoint(units_t units);
    void distToPoint(double value, units_t units);

    unsigned int id(void);
    void id(unsigned int value);

  private:
    //unit: mm
    double mDistance;
    //unit: mm/ms
    double mSpeed;
    int mTurn;
    bool mTurnDir;
    String mDescrLine1;
    String mDescrLine2;
    //unit: ms
    double mTimeToPoint;
    //unit: mm
    double mDistToPoint;
    unsigned int mId;
};

#endif