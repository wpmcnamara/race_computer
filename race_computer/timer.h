#ifndef __TIMER__
#define __TIMER__

#include <IntervalTimer.h>
#include "bsp.h"
#include "helpers.h"
#include "event.h"


extern volatile bool timer_run;
extern volatile uint32_t timerSeconds;
extern IMXRT_TMR_t * TMRx;


struct orcTime {
  int hour;
  int minute;
  int second;
  int millis;
};
typedef struct orcTime orcTime_t;

void timerSetup(void);
void eventTimerStop(void);
double getTimeStamp(void);

void intervalTimerAddCallback(event_t *eventPtr);
void intervalTimerDelCallback(event_t *eventPtr);

#endif
