#ifndef __RACE_POINT__
#define __RACE_POINT__
#include <Arduino.h>
#include <string>

typedef class racePoint racePoint_t;

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

#endif