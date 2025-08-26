#include "timer.h"
#include "gps.h"

volatile bool timer_run=false;
volatile bool refresh=false;
unsigned long tick=0;
unsigned long tickHold;
int cntHold;

IMXRT_TMR_t * TMRx = (IMXRT_TMR_t *)&IMXRT_TMR4;
IntervalTimer it1;

void it1cb() {
  refresh = true;
}

void ppsInterrupt() {
  TMRx->CH[2].SCTRL  &= ~(TMR_SCTRL_TCF);  // clear
  TMRx->CH[2].CSCTRL &= ~(TMR_CSCTRL_TCF1);
  tick++;
}

void timerSetup(void) {
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
  
}

void timerUpdate(void) {
  cntHold=TMRx->CH[2].CNTR;
  tickHold=tick;
}

unsigned long getTick(void) { return tickHold; };
int getCnt(void) { return cntHold; };
