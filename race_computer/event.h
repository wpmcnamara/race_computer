#ifndef __EVENT__
#define __EVENT__

#include <Arduino.h>

enum eventTypeEnum {
  eventSingle,
  eventMultiple,
  eventRepeat,
  eventOneShot
};

typedef enum eventTypeEnum eventType_t;
typedef class event event_t;

class event {
  public:
    event(void (*eventAction)(void), eventType_t eventType, bool active, bool eventRemove, unsigned long eventRepeat, unsigned long eventDelay, class usb_serial_class * serialPortPtr=NULL, const char *namePtr=NULL);
    ~event();
    void exec(void);
    bool active;
  private:
    void (*action)(void);
    bool repeat;
    bool remove;
    unsigned long count;
    unsigned long delay;
    unsigned long delayReload;
    unsigned long countReload;
    const char *name;
    class usb_serial_class *serialPort;
};

#endif