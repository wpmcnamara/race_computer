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
    event(void (*eventAction)(void), eventType_t eventType, bool active, bool eventRemove, unsigned long eventRepeat, unsigned long eventDelay, class usb_serial_class * serialPortPtr, const char *namePtr);
    ~event();
    void exec(void);
    void setDelay(unsigned long newDelay);
    void stagger(unsigned long staggerAmount);
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