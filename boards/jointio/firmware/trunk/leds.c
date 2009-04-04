#include "leds.h"

void leds_init( void )
{
	/* Turn outputs off before enabling them */
	P1OUT &= ~0xf0;
	P4OUT &= ~0x0f;

	P1DIR |= 0xf0;
	P4DIR |= 0x0f;
}

void leds_set( uint8_t v )
{
	P1OUT = (P1OUT & 0x0f) | ((v & 0x0f) << 4);
	P4OUT = (P4OUT & 0xf0) | ((v & 0xf0) >> 4);
}

