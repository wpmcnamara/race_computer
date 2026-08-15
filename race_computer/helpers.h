#ifndef __HELPERS__
#define __HELPERS__

#include "bsp.h"
#include <math.h>
#include <Timezone.h>

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)
#define ROUND3(x) ((round((x)*1000.0)/1000.0))

#define SPEED_INTERNAL_TO_MPH(x) (ROUND3((x)*2.23693629207))
#define SPEED_MPH_TO_INTERNAL(x) ((x)*0.44704)
#define SPEED_INTERNAL_TO_KPH(x) (ROUND3((x)*3.6))
#define SPEED_KPH_TO_INTERNAL(x) ((x)*0.277777778)

#define DISTANCE_INTERNAL_TO_MILES(x) (ROUND3((x)*0.000000621371192237334))
#define DISTANCE_MILES_TO_INTERNAL(x) ((x)*1609344)
#define DISTANCE_INTERNAL_TO_KILOMETERS(x) (ROUND3((x)*0.000001))
#define DISTANCE_KILOMETERS_TO_INTERNAL(x) ((x)*10000000)


#define TIME_INTERNAL_TO_SECONDS(x) (ROUND3((x)/1000.0))
#define TIME_SECONDS_TO_INTERNAL(x) ((double)round(((double)(x))*1000.0))

enum units_enum {
	imperial=0,
	metric,
	seconds,
	milliseconds,
	miles,
	kilometers,
	km,
	mph,
	kph,
	internal
};

typedef enum units_enum units_t;

#define IMR_INDEX   5  
#define ISR_INDEX   6

#define VERSION_STRING "FW Version: 0.13.1"

inline void disableInterrupt(uint8_t pin) {
	if (pin >= CORE_NUM_DIGITAL) return;
	volatile uint32_t *gpio = portOutputRegister(pin);
	uint32_t mask = digitalPinToBitMask(pin);
	gpio[IMR_INDEX] &= ~mask;
}

inline void enableInterrupt(uint8_t pin) {
	if (pin >= CORE_NUM_DIGITAL) return;
	volatile uint32_t *gpio = portOutputRegister(pin);
	uint32_t mask = digitalPinToBitMask(pin);
  	gpio[ISR_INDEX] = mask;  // clear any prior pending interrupt
	gpio[IMR_INDEX] |= mask; // enable interrupt
}

#endif