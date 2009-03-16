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

#include "servo.h"
#include "hardware.h"
#include <stdint.h>
#include <signal.h>
#include "sweep.h"
#include "common.h"
#include "timer-b.h"
#include "i2c.h"
#include "flash430/flash.h"
#include "flash430/i2c-flash.h"
#include "lcd.h"
#include "railmon.h"
#include "watchdog.h"
#include "led.h"

#define USE_WATCHDOG 0

static uint8_t i = 0;

void init(void);

int main( void )
{
	i  = 1;
	init();

	servo_set_pwm(0, MIDDLE_PULSE);
	servo_set_pwm(1, MIDDLE_PULSE);		

	while(1)
	{
		if( i2c_flash_received )
				flash_switchover();
		lcd_service();
	}	
}

void init(void)
{
	/* Disable the watchdog timer */
	WDTCTL = WDTPW | WDTHOLD;
	nop();
	nop();
	nop();
	/* Configure DCOCTL to be 12 MHz */
	DCOCTL = CALDCO_12MHZ;
	BCSCTL1 = CALBC1_12MHZ;

	/* Default to inputs */
	P1DIR = P2DIR = P3DIR = P4DIR = 0;
	P1OUT = P2OUT = P3OUT = P4OUT = 0;
	P1SEL = P2SEL = P4SEL = 0x00;

	/* Pull-ups */
	P3REN |= 0;

	/* Disable interupts on port 1 (unconnected) */
	P1IES = 0;
	P1IE = 0;

	if( USE_WATCHDOG )
	{
		/* Source ACLK from VLOCLK (12 KHz)*/
		BCSCTL3 = (BCSCTL3 & ~LFXT1S_3) | LFXT1S_2;

		/* Enable the WDT - Source clock from ACLK(VLOCLK) */
		/* WDT needs resetting every 12KHz/512 = 43 ms */
		WDTCTL = WDTPW | WATCHDOG_SETTINGS | WDTCNTCL;
	}

	servo_init();
	led_init();
	flash_init();
	i2c_flash_init();
	i2c_init();
	timer_b_init();
	railmon_init();
	lcd_init();
	eint();
}

interrupt (NOVECTOR) IntServiceRoutine(void)
{

}
