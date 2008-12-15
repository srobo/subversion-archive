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
#include "flash.h"
#include "i2c-flash.h"
#include <signal.h>

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

	motor_set( 0, 0, M_OFF );
	motor_set( 1, 0, M_OFF );

	while(1) {
		if( i2c_flash_received ) {
				flash_switchover();
		}
	}
}

void init( void )
{
	init_gpio();
	pwm_init();
	motor_init();
	timer_b_init();
	flash_init();
	i2c_flash_init();
	i2c_init();

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

	/* Debug light off */
	FLAG_OFF();
}
