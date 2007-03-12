#include "pwm.h"
#include "common.h"

void pwm_init( void )
{
	/* Configure the timer peripheral */
	/* Clock off crystal */
	/* Frequency ~200Hz */

	/* TODO: Finish */
	/* Set all channels to on for the moment */
	P1OUT &= ~(0x0C);

}

void pwm_set( uint8_t channel, pwm_ratio_t r )
{

}

pwm_ratio_t pwm_get( uint8_t channel )
{

}
 
