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
#include "common.h"
#include "motor.h"
#include "pwm.h"
#include "i2c.h"
#include "timer-b.h"
#include "flash430/flash.h"
#include "flash430/i2c-flash.h"
#include "leds.h"
#include <signal.h>
#include "adc-10.h"
#include "control.h"

static int i = 0;

/* Initialise the GPIO ports */
void init_gpio( void );

/* Initialise everything */
void init( void );

int main( void )
{
	i = 0;

	/* Disable the watchdog timer */
	WDTCTL = WDTHOLD | WDTPW;

	init();

	while(1) {
		if( i2c_flash_received )
			flash_switchover();

		adc10_proc();
	}
}

void init( void )
{
	/* Run at 16 MHz */
	DCOCTL = CALDCO_16MHZ;
	BCSCTL1 = CALBC1_16MHZ;

	init_gpio();
	leds_init();
	h_bridge_init();
	pwm_init();
	motor_init();
	timer_b_init();
	flash_init();
	i2c_flash_init();
	i2c_init();
	adc10_init();
	control_init();

	eint();
}

void init_gpio( void )
{
	/* Set all to outputs */
	P1DIR = P2DIR = P3DIR = P4DIR = 0xFF;

	/* Configure all as IO initially */
	P1SEL = P3SEL = P4SEL = 0;

	/* Crystal inputs */
	P2SEL |= 0xC0;

	/* Feedbacak inputs */
	P2DIR &= 0xf0;

	/* Current Sense initialisation TODO: Move into serparate header?*/
	/* set up MAX4069 chips with GSEL signal to give gain of 50 */
	P4DIR |= 0x60;
	P4OUT &= ~0x60;

	/* Debug light off */
	FLAG_OFF();
}
