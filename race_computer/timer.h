#ifndef __TIMER__
#define __TIMER__

#include <IntervalTimer.h>
#include "bsp.h"
#include "helpers.h"
#include "event.h"

extern volatile bool timer_run;
extern unsigned long tick;
extern IMXRT_TMR_t * TMRx;


void timerSetup(void);
unsigned long getTick(void);
int getCnt(void);
double getTime(void);
void intervalTimerAddCallback(event_t *eventPtr);
void intervalTimerDelCallback(event_t *eventPtr);

#endif
