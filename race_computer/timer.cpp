#include "timer.h"
#include "gps.h"
#include <list>
#include "event.h"

void timerUpdate(void);

volatile bool timer_run=false;

volatile uint32_t timerSeconds;

IntervalTimer it1;

struct intervalTimerCallbackEntry {
  void (*callbackPtr)(void);
  unsigned long tickCount;
};

IMXRT_TMR_t * TMRx = (IMXRT_TMR_t *)&IMXRT_TMR4;
std::list<event_t *> intervalTimerCallbackList;

void it1cb() {
  //if(timer_run) {
    //Serial.printf("%02d:%02d:%02d.%03d\n", gpsData.gpsTime.hour, gpsData.gpsTime.minute, gpsData.gpsTime.second, gpsData.gpsTime.millis);
  //  Serial.printf("%f\n", getTimeStamp());
  //}
  for (std::list<event_t *>::iterator it=intervalTimerCallbackList.begin(); it != intervalTimerCallbackList.end(); ++it) {
      ((*it)->exec)();
  }
}

void intervalTimerAddCallback(event_t *eventPtr) {
  intervalTimerCallbackList.push_back(eventPtr);
}

void intervalTimerDelCallback(event_t *eventPtr) {
  intervalTimerCallbackList.remove(eventPtr);
}

//ppsInterrupt is driven from the GPS frequency output and internal counter compare 
//The GPS frequency output drives counter and a roll over interrupt is generated once per second
void ppsInterrupt() {
  //we don't want any other interrupts firing while we are dealing with the timer update.
  __disable_irq();
  //we just update the seconds count here, as atomically as possible. 
  timerSeconds++;
  // we can clear the timer overflow flag to signal that the 
  //timer overflow is represented in the seconds count.  This also prevents the interrupt handler from 
  //immediately getting called again.
  TMRx->CH[2].SCTRL  &= ~(TMR_SCTRL_TCF);  // clear
  TMRx->CH[2].CSCTRL &= ~(TMR_CSCTRL_TCF1);
  __enable_irq();
}

void timerSetup(void) {
  timerSeconds=0;
  *(portConfigRegister(GPS_PPS)) = 1;  // ALT 1
  CCM_CCGR6 |= CCM_CCGR6_QTIMER3(CCM_CCGR_ON);
  TMRx->CH[2].CTRL = 0x000; 
  attachInterruptVector(IRQ_QTIMER4, ppsInterrupt);
  TMRx->CH[2].CNTR = 0;
  TMRx->CH[2].LOAD = 0;  // start val after compare
  TMRx->CH[2].COMP1 = PPS_FREQUENCY - 1;  // count up to this val and start again
  TMRx->CH[2].CMPLD1 = PPS_FREQUENCY - 1;
  TMRx->CH[2].SCTRL = TMR_SCTRL_TCFIE;  // enable interrupts;
  NVIC_ENABLE_IRQ(IRQ_QTIMER4);
  it1.begin(it1cb, 10000);  // microseconds
}

double getTimeStamp(void) {
  uint32_t secondCapture;
  uint32_t milliCapture;
  double timeStamp;
  uint32_t timerCtrl;
  //We don't want the seconds count to be updated while we are reading the values so we need
  //to turn of interrupts for just a second, to capture the seconds and milliseconds counts.
  //we read the value of the milliseconds count first, in case it rolls over while interrupts are off.
  //There is no way to absolutely guarantee we don't glitch though.  If we roll over the timer at the
  //same instant interrupts are disabled, the timer capture could read 0, but the seconds tick has
  //not yet been updated.  We check for this special case and account for it by checking the timer
  //interrupt flag after reading the timer count.  If it is set, then we know the timer has rolled over and
  //the interrupt to update the seconds count is pending.  If the milliseconds count is zero, then 
  //we increment our seconds by one to match the pending interrupt.  If its none zero, then we 
  //disabled interrupts just before the rollover happened, but our seconds count will be correct.
  __disable_irq();
  milliCapture=TMRx->CH[2].CNTR;
  timerCtrl=TMRx->CH[2].CSCTRL; 
  secondCapture=timerSeconds;
  __enable_irq();
  //We rolled over just before disabling interrupts but got interrupts disabled before the interrupt handler
  //could execute so we need add a second to account for the rollover.
  if((timerCtrl & TMR_CSCTRL_TCF1)) {
    secondCapture++;
  }
  //this can generate negative timestamps.  The time stamp returned is relative to the start of the 
  //current leg.  We actually depend on the timestamp being negative, in the case of a delay start.
  //The start countdown displays the timestamp, which is negative and counts to zero as the leg
  //starts.
  timeStamp=(double)(((double)secondCapture*1000.0)+milliCapture)-race.timerOffset(milliseconds);
  return timeStamp;
}
  
void eventTimerStop(void) {it1.end();};
