#include "h-bridge.h"
#include "common.h"

void h_bridge_init( void )
{
	/* Set up the output values first */
	h_bridge_set( 0, M_OFF );
	h_bridge_set( 1, M_OFF );

	/* Bits 3, 4, 5 and 6 outputs */
	P3DIR |= 0x78;
}

void h_bridge_set( uint8_t channel, h_bridge_state_t state )
{
	uint8_t v = 0;
	uint8_t p;

	switch( state )
	{
	case M_OFF:
		v = 3; break;
	case M_FORWARD:
		v = 0; break;
	case M_BACKWARD:
		v = 1; break;
	default:
	case M_BRAKE:
		v = 2; break;
	}

	/* Calculate the shift necessary */
	if( channel == 0 )
		p = 3;
	else
		p = 5;

	P3OUT &= ~( 3 << p );
	P3OUT |= v << p;
}
