#ifndef __RACE__
#define __RACE__

#include "timer.h"
#include <list>

void raceSetup(void);

typedef class racePoint racePoint_t;
typedef class raceLeg raceLeg_t;
typedef class race race_t;
typedef class raceData raceData_t;

class racePoint {
  public:
    float miles;
    unsigned int turn;
    bool turnDir;
    float speed;
    char *descr;
    unsigned int id;
};

class raceLeg {
  public:
    float speed;
    float distance;
    unsigned int id;
    char *descr;
    unsigned int start;
    bool inProgress;
  private:
    std::list<racePoint_t *> points;
};

class race {
  public:
    char *descr;
    float speed;
    float distance;
    bool inProgress;
  private:
    std::list<raceLeg_t *> raceLegs;

};

class raceData {
  public:
    //target speed for the current race or leg.  This will be in meters/second;
    //Value is loaded from the race definition when the race is selected. For race
    //legs it is loaded when the leg is entered.
    float targetSpeed;
    //running average speed for the leg/race.  This is actively updated from GPS
    //data during the race
    float averageSpeed;
    //difference between target and average.  We calculate once and store, when 
    //averageSpeed is updated, rather than calculating each time it is used elsewhere.
    float speedDelta;
    //Defines the range, +/-, in meters/s around the targetSpeed that is considered 
    //within the target speed range.  Used to control the LED color feedback during a
    //race 
    float speedTargetBand;
    //Total distance of the race/leg in meters.  For a race, value is loaded from the
    //race definition when the race is selected.  For legs, it is loaded from the leg
    //definition when the leg is entered.
    unsigned int totalDistance;
    //zero for a race leg.  For the active race, this is the sum of the distance of 
    //completed legs.  Needed to calculate the whole race average speed.
    unsigned int distanceComplete;
    //zero for a race leg.  For the active race, this is the sum of the times for all
    //completed legs.  Needed to calculate the whole race average speed.
    float timeComplete;
    //distance traveled for the race/leg.  For a race, this will be the sum of distanceComplete
    //and the distance traveled in the current leg.
    unsigned int distance;
    unsigned int distanceRemaining;
    //zero for a race.  For a leg, this will be the value of the GPS odometer at the start
    //of the leg.
    unsigned int distanceOffset;
    //start and end GPS timestamps for the leg.  Not used in the race context
    timeStamp_t startTs;
    timeStamp_t endTs;
    //difference between target time and current time, based on distance traveled.  Like speedDelta
    //we calculate when GPS data changes and save, rather than calculating every time it is 
    //used.
    float timeDelta;
    //signify the race/leg is being actively timed.
    bool inProgress;
    //For a race, this points to the raceData structure for the current leg.  For a leg, this
    //is NULL
    raceData_t *legData;
    //race definition loaded from storage
    race_t *activeRace;
    //active leg within the race definition;
    raceLeg_t *activeLeg;
};

extern raceData_t race;

#endif 