#include "timer.h"
#include "gps.h"
#include <list>
#include "event.h"

void timerUpdate(void);

volatile bool timer_run=false;
volatile timeStamp_t timerVal;
volatile timeStamp_t timerTs;
//unsigned long intervalTimerCount=0;

IntervalTimer it1;
//event_t timerUpdateEvent(timerUpdate, eventRepeat, false, 0, 1);

struct intervalTimerCallbackEntry {
  void (*callbackPtr)(void);
  unsigned long tickCount;
};

IMXRT_TMR_t * TMRx = (IMXRT_TMR_t *)&IMXRT_TMR4;
std::list<event_t *> intervalTimerCallbackList;

void it1cb() {
  for (std::list<event_t *>::iterator it=intervalTimerCallbackList.begin(); it != intervalTimerCallbackList.end(); ++it) {
      ((*it)->exec)();
  }
}

void intervalTimerAddCallback(event_t *eventPtr) {
  intervalTimerCallbackList.push_back(eventPtr);
}

void intervalTimerDelCallback(event_t *eventPtr) {
  for (std::list<event_t *>::iterator it=intervalTimerCallbackList.begin(); it != intervalTimerCallbackList.end(); ++it) {
    if(*it==eventPtr) {
      intervalTimerCallbackList.erase(it);
    }
  }  
}

void ppsInterrupt() {
  TMRx->CH[2].SCTRL  &= ~(TMR_SCTRL_TCF);  // clear
  TMRx->CH[2].CSCTRL &= ~(TMR_CSCTRL_TCF1);
  timerVal.seconds++;
  timerTs.seconds=timerVal.seconds;
  timerTs.millis=TMRx->CH[2].CNTR;
}

void timerSetup(void) {
  timerTs.millis=0;
  timerTs.seconds=0;
  timerVal.millis=0;
  timerVal.seconds=0;
  *(portConfigRegister(GPS_PPS)) = 1;  // ALT 1
  CCM_CCGR6 |= CCM_CCGR6_QTIMER3(CCM_CCGR_ON);
  TMRx->CH[2].CTRL = 0x000; 
  attachInterruptVector(IRQ_QTIMER4, ppsInterrupt);
  TMRx->CH[2].CNTR = 0;
  TMRx->CH[2].LOAD = 0;  // start val after compare
  TMRx->CH[2].COMP1 = PPS_FREQUENCY - 1;  // count up to this val and start again
  TMRx->CH[2].CMPLD1 = PPS_FREQUENCY - 1;
  TMRx->CH[2].SCTRL = TMR_SCTRL_TCFIE;  // enable interrupts;
  //TMRx->CH[2].CSCTRL = TMR_CSCTRL_TCF1EN;
  NVIC_ENABLE_IRQ(IRQ_QTIMER4);
  //TMRx->CH[2].CTRL = TMR_CTRL_CM(1) | TMR_CTRL_PCS(2) | TMR_CTRL_LENGTH;
  it1.begin(it1cb, 10000);  // microseconds
  new event_t (timerUpdate, eventRepeat, true, false, 0, 1, &Serial, "timerUpdate");
}

void timerUpdate(void) {
  timerTs.seconds=timerVal.seconds;
  timerTs.millis=TMRx->CH[2].CNTR;
}

unsigned long getTick(void) { return timerTs.seconds; };
timeStamp_t * getTimeStamp(void) { return &timerTs; };
double getTime(void) { return ((double)timerTs.seconds+((double)timerTs.millis/1000.0));}
int getCnt(void) { return timerTs.millis; };
