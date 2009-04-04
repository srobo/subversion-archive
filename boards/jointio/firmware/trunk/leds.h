#ifndef __LEDS_H
#define __LEDS_H
#include "common.h"
#include <stdint.h>

void leds_init( void );

void leds_set( uint8_t v );

#endif	/* __LEDS_H */
