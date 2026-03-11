#ifndef __TIMER__
#define __TIMER__

#include <IntervalTimer.h>
#include "bsp.h"
#include "helpers.h"
#include "event.h"


#define TS_TO_FLOAT(ts) ((float)ts.seconds+((float)ts.millis/1000.0))
#define TSPTR_TO_FLOAT(ts) ((float)ts->seconds+((float)ts->millis/1000.0))
typedef struct timeStamp timeStamp_t;

extern volatile bool timer_run;
extern volatile timeStamp_t timerVal;
extern IMXRT_TMR_t * TMRx;

struct timeStamp {
  unsigned long seconds;
  unsigned int millis;
};


struct orcTime {
  int hour;
  int minute;
  int second;
  int millis;
};
typedef struct orcTime orcTime_t;

void timerSetup(void);
void eventTimerStop(void);
unsigned long getTick(void);
int getCnt(void);
double getTime(void);
volatile timeStamp_t * getTimeStamp(void);

void intervalTimerAddCallback(event_t *eventPtr);
void intervalTimerDelCallback(event_t *eventPtr);

#endif
