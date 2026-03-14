#ifndef __RACE__
#define __RACE__

#include "timer.h"
#include <list>
#include <string>

#include <YAMLDuino.h>

typedef class racePoint racePoint_t;
typedef class raceLeg raceLeg_t;
typedef class race race_t;
typedef class raceData raceData_t;

void raceSetup(void);
void raceLegStart(void);
void raceLegStop(void);
void loadRaces(void);
void setRace(race_t *, raceLeg_t *);
void prepRace(void);
void updateRace(void);
void dumpRaceData(raceData_t *data);
void raceCheckPoint(void);
void loadRaceCheckPoint(void);
void loadRacePoints(raceLeg_t *raceLeg);
void clearRacePoints(raceLeg_t *raceLeg);
void loadDefaultRaces(void);

extern event_t *delayedStartEvent;




extern std::list<race_t *>::iterator selectedRace;
extern std::list<race_t *>::iterator selectedRaceSave;
extern std::vector<raceLeg_t *>::iterator selectedRaceLeg;
extern std::list<race_t *> races; 
extern raceData_t tmpLeg;

class racePoint {
  public:
    int distance;
    int turn;
    bool turnDir;
    double speed;
    String descrLine1;
    String descrLine2;
    long int timeToPoint;
    int distToPoint;
    unsigned int id;
};

class raceLeg {
  public:
    //Race target speed for this leg, based on public leg distance
    float speed;
    //How close to target is considered to be "on target"
    float speedRange;
    //publish leg distance
    float distance;
    //actual distance driven for leg.  Used to properly calculated target
    //speed.
    float driveDistance;
    //target time to complete the leg -- in seconds.
    float targetTime;
    unsigned int id;
    String descr;
    String pointsFile;
    unsigned int mark;
    bool inProgress;
    bool complete;
    std::vector<racePoint_t *> points;
  private:

};

class race {
  public:
    String fileName;
    String descr;
    float speed;
    float speedRange;
    float distance;
    bool inProgress;
    unsigned int mark;
    std::vector<raceLeg_t *> raceLegs;
  private:


};

class raceData {
  public:
    //time to complete the race/leg, in seconds.  Currently calculated from distance and
    //speed.  Not loaded from definition files.
    double time;
    //target speed for the current race or leg.  This will be in meters/second;
    //Value is loaded from the race definition when the race is selected. For race
    //legs it is loaded when the leg is entered.
    double targetSpeed;
    //Actual target speed for the current race or leg.  This will be in meters/second.
    //Value is calculated by from the ratio of publish distance to actual driving distance.
    double adjustedTargetSpeed;
    //running average speed for the leg/race.  This is actively updated from GPS
    //data during the race
    double averageSpeed;
    //difference between target and average.  We calculate once and store, when 
    //averageSpeed is updated, rather than calculating each time it is used elsewhere.
    double speedDelta;
    //Defines the range, +/-, in meters/s around the targetSpeed that is considered 
    //within the target speed range.  Used to control the LED color feedback during a
    //race 
    double speedTargetBand;
    //Total distance of the race/leg in meters.  For a race, value is loaded from the
    //race definition when the race is selected.  For legs, it is loaded from the leg
    //definition when the leg is entered.
    int totalDistance;
    //Total drive distance of the course in meters.  For a race, this will be zero and not
    //used.  For a leg, this will be the actual distance to drive for the leg.  It will be used 
    //to calculate the actual target speed and time deltas, in combination with the 
    //totalDistance value.
    int driveDistance;
    //For the active race, this is the sum of the distance of completed legs.  For a leg,
    //this will be 0.  Needed to calculate the whole race average speed.
    int distanceComplete;
    //For the active race, this is the sum of the disance driven for completed legs based on the
    //legs defined drive distance, not the actual distance driven.  For a leg, this will 0.
    int driveDistanceComplete;
    //zero for a race leg.  For the active race, this is the sum of the times for all
    //completed legs.  Needed to calculate the whole race average speed.
    timeStamp_t timeComplete;
    //distance traveled for the race/leg.  For a race, this will be the sum of distanceComplete
    //and the distance traveled in the current leg.
    int distance;
    int distanceRemaining;
    //zero for a race.  For a leg, this will be the value of the GPS odometer at the start
    //of the leg.
    int distanceOffset;
    //start and end GPS timestamps for the leg.  Not used in the race context
    timeStamp_t startTs;
    timeStamp_t endTs;
    //Holds the modulus value for the start time.  If set, will delay start of race time after 
    //button press until the next startMark second mark.  Allows precise alignment of race timing.
    unsigned int startMark;
    bool delayedStart;
    //When we have a delayed start, there will be an offset between the internal timer and the 
    //start of timing that we need to account for.
    timeStamp_t timerOffset;
    //difference between target time and current time, based on distance traveled.  Like speedDelta
    //we calculate when GPS data changes and save, rather than calculating every time it is 
    //used.  Value is in milliseconds
    long int timeDelta;
    //signify the race/leg is being actively timed.
    bool inProgress;
    //For a race, this points to the raceData structure for the current leg.  For a leg, this
    //is NULL
    raceData_t *legData;
    //race definition loaded from storage
    race_t *activeRace;
    //active leg within the race definition;
    raceLeg_t *activeLeg;
    std::vector<racePoint_t *>::iterator activePoint;
};

extern raceData_t race;

#endif 