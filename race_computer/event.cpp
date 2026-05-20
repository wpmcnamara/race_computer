#include "event.h"
#include "timer.h"
#include <Print.h>

event::event(void (*eventAction)(void), eventType_t eventType, bool eventActive, bool eventRemove, unsigned long eventCount, unsigned long eventDelay, class usb_serial_class * serialPortPtr=NULL, const char *namePtr=NULL) {
  action=eventAction;
  active=eventActive;
  serialPort=serialPortPtr;
  name=namePtr;
  remove=eventRemove;
  switch(eventType) {
    case eventSingle:
      repeat=false;
      delay=eventDelay;
      count=0;     
      break;
    case eventMultiple:
      repeat=false;
      delay=eventDelay;
      count=eventCount;
      break;
    case eventRepeat:
      repeat=true;
      delay=eventDelay;
      count=0;
      break;
  }
  delayReload=delay;
  countReload=count;
  if(serialPort != NULL && name != NULL) {
    serialPort->print("event creation: ");
    serialPort->println(name);
  }
  intervalTimerAddCallback(this);
}

event::~event() {
  if(serialPort != NULL && name != NULL) {
    serialPort->print("event deletion: ");
    serialPort->println(name);
  }
  intervalTimerDelCallback(this);  
}

void event::exec(void) {
//  serialPort->print("event: ");
//  serialPort->println(name);
  if(!active) {
    return;
  }
  if(delay==0 || delay==1) {
    //if(timer_run) {
    //  serialPort->println(name);
    //}
    action();
  } else {
    delay--;
    return;
  }
  if(repeat) {
    delay=delayReload;
  } else {
    if(count) {
      delay=delayReload;
      count--;
    } else {
      if(remove) {
        delete(this);
      } else {
        active=false;
        delay=delayReload;
        count=countReload;
      }
      return;
    }
  }
}

void event::setDelay(unsigned long newDelay) {
  delay=newDelay;
  delayReload=newDelay;
}

void event::stagger(unsigned long staggerAmount) {
  delay+=staggerAmount;
}
