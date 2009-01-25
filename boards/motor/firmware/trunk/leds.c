#include "leds.h"

void leds_init( void )
{
	P1OUT &= ~0xF0;
	P1DIR |= 0xF0;
}

