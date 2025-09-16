#ifndef __EVENT__
#define __EVENT__

#include <Arduino.h>

enum eventTypeEnum {
  eventSingle,
  eventMultiple,
  eventRepeat
};

typedef enum eventTypeEnum eventType_t;
typedef class event event_t;

class event {
  public:
    event(void (*eventAction)(void), eventType_t eventType, bool active, unsigned long eventRepeat, unsigned long eventDelay, class usb_serial_class * serialPortPtr, const char *namePtr);
    ~event();
    void exec(void);
    bool active;
  private:
    void (*action)(void);
    bool repeat;
    unsigned long count;
    unsigned long delay;
    unsigned long delayReload;
    const char *name;
    class usb_serial_class *serialPort;
};

#endif