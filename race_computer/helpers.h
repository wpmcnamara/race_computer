#ifndef __HELPERS__
#define __HELPERS__

#include "bsp.h"

#define PRREG(x) Serial.print(#x" 0x"); Serial.println(x,HEX)

#define IMR_INDEX   5
#define ISR_INDEX   6

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