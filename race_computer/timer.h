#ifndef __TIMER__
#define __TIMER__

#include <IntervalTimer.h>
#include "bsp.h"
#include "helpers.h"

extern volatile bool timer_run;
extern volatile bool refresh;
extern unsigned long tick;
extern IntervalTimer it1;
extern IMXRT_TMR_t * TMRx;

void timerSetup(void);
void timerUpdate(void);
unsigned long getTick(void);
int getCnt(void);

#endif
