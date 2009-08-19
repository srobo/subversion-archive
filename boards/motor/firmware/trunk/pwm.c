/*   Copyright (C) 2007 Robert Spanton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */
#include "pwm.h"
#include "common.h"
#include "control.h"
#include <signal.h>

void pwm_init( void )
{
	/* Configure the timer peripheral */
	/* Clock off crystal */
	/* Frequency ~200Hz */

	/* Clear the timer */
	TACTL |= TACLR;

	/* Set all channels to on for the moment */
	P1OUT &= ~(0x0C);

	/* Select ACLK (watch crystal) as clock source */
	TACTL &= ~TASSEL_3;
	TACTL = TASSEL_ACLK;

	/* No input divider */
	TACTL &= ~ID_3;
	TACTL |= ID_DIV1;

	/* No timer interrupt */
	TACTL |= TAIE;
	
	/*** Configure TACCTL0 ***/
	/* Compare mode */
	TACCTL0 &= ~CAP;
	TACCTL0 &= ~CCIE;

	/*** Configure TACCTLs ***/
	/* Compare mode */
	TACCTL1 = CCIS_3 | OUTMOD_SET_RESET;
	TACCTL2 = CCIS_3 | OUTMOD_SET_RESET;

	TACCR0 = PWM_MAX;
	TACCR1 = 0;
	TACCR2 = 0;

	/* Enable the outputs */
	P1SEL |= 0x0C;

	/* Up mode - enables the timer */
	TACTL &= ~MC_3;
	TACTL |= MC_UPTO_CCR0;
}

interrupt (TIMERA1_VECTOR) timer_a_isr1( void )
{
	uint16_t _taiv = TAIV;
	if( _taiv == TAIV_OVERFLOW )
		control_step();
}

void pwm_set( uint8_t channel, pwm_ratio_t r )
{
	if( channel == 0 )
		TACCR2 = r;
	else
		TACCR1 = r;
}

pwm_ratio_t pwm_get( uint8_t channel )
{
	if( channel == 0 )
		return TACCR2;
	else
		return TACCR1;
}
 
