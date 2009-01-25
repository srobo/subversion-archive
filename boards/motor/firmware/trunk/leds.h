/* Routines for controlling the LEDs */
#ifndef __LEDS_H
#define __LEDS_H
#include "common.h"

/* Initialise the LED routines */
void leds_init( void );

/* Set LED number led to be on or off (1 or 0) */
#define leds_set(led, val) do { P1OUT &= ~(0x10<<led); P1OUT |= val?(0x10<<led):0; } while (0)

/* Toggle LED number led */
#define leds_toggle(led) do { P1OUT ^= 0x10<<led; }  while (0)

#endif	/* __LEDS_H */
