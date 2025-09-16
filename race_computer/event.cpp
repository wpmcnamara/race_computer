#include "event.h"
#include "timer.h"
#include <Print.h>

event::event(void (*eventAction)(void), eventType_t eventType, bool eventActive, unsigned long eventCount, unsigned long eventDelay, class usb_serial_class * serialPortPtr, const char *namePtr) {
  action=eventAction;
  active=eventActive;
  serialPort=serialPortPtr;
  name=namePtr;
  switch(eventType) {
    case eventSingle:
      repeat=false;
      delay=eventDelay;
      count=0;     
      delayReload=0;
      break;
    case eventMultiple:
      repeat=false;
      delay=eventDelay;
      delayReload=eventDelay;
      count=eventCount;
      break;
    case eventRepeat:
      repeat=true;
      delay=eventDelay;
      delayReload=eventDelay;
      count=0;
      break;
  }
  serialPort->print(F("event creation: "));
  serialPort->println(name);
  intervalTimerAddCallback(this);
}

event::~event() {
      serialPort->print("event deletion: ");
      serialPort->println(name);
      intervalTimerDelCallback(this);  
}

void event::exec(void) {
//  serialPort->print("event: ");
//  serialPort->println(name);
  if(!active) {
    return;
  }
  if(delay==0 || delay==1) {
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
      delete(this);
      return;
    }
  }
}
