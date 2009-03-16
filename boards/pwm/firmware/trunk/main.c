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

#define USE_WATCHDOG 0

/* ACLK, 512 counts */
#define WATCHDOG_SETTINGS (WDTSSEL | WDTIS_2)

#if (USE_WATCHDOG)
#define watchdog_clear() do { WDTCTL = WATCHDOG_SETTINGS | WDTCNTCL | WDTPW; } while (0)
#else
#define watchdog_clear() do {} while(0)
#endif

static uint8_t i = 0;

/* The current_servo */
uint8_t current_servo;

/* Set the output to the given value */
inline void set_p1out(uint8_t p1);

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

	/* default output pins to 0 , default to GPIO function*/
	P1OUT = P2OUT = P3OUT = P4OUT = 0x00;
	P1SEL = P2SEL = P4SEL = 0x00;

	/* Port 2: 2.4 =  vcc-detect; 2.1 = status LED */
	P2DIR = 0xEF;
	
	/* Port 3: 3.1 = I2C SDA; 3.2 = I2C SCL */
	P3DIR = 0xFD;
	P3SEL = 0x06;

	/* Port 4: 4.x 0<x<5 are servo outputs */
	P4DIR = 0xFF;

	/* Pull-ups */
	P3REN |= 0;

	/* Disable interupts on port 1 (unconnected) */
	P1IES = 0;
	P1IE = 0;

	servo_init();

	//TACCR0 interrupts after 20ms
	TACCR0 = PERIOD;
	//TACCR1 interrupts at end of pulse for servo 0
	TACCR1 = servo_get_pwm(0);

	TACCTL0 = CCIE; //turn on interrupts for ccp module
	TACCTL1 = CCIE; //turn on interrupts for ccp module

	//setting timer A to count up to TACCR0, and also turn on interrupts
	TACTL = TASSEL_SMCLK	/* SMCLK clock source */
		| MC_UPTO_CCR0	/* Count up to the value in CCR0 */
		| ID_DIV4	/* Divide the frequency by 4 */
		| TAIE;		/*  */

	current_servo = 0;

	if( USE_WATCHDOG )
	{
		/* Source ACLK from VLOCLK (12 KHz)*/
		BCSCTL3 = (BCSCTL3 & ~LFXT1S_3) | LFXT1S_2;

		/* Enable the WDT - Source clock from ACLK(VLOCLK) */
		/* WDT needs resetting every 12KHz/512 = 43 ms */
		WDTCTL = WDTPW | WATCHDOG_SETTINGS | WDTCNTCL;
	}

	flash_init();
	i2c_flash_init();
	i2c_init();
	timer_b_init();
	railmon_init();
	lcd_init();
	eint();
}

/* ISR for TACCR0. 
 * Called at the end of the pulse period ~20ms,
 * it resets TIMERA, resets current_servo, and put servo0 pin high. */
interrupt (TIMERA0_VECTOR) isr_TACR0(void)
{
//	i2c_watchdog_check();
	watchdog_clear();

	current_servo = 0;
	set_p1out(0xFE);
}

inline void set_p1out(uint8_t p1)
{
	/* Only change the output if the rail is up */
	if( railmon_is_up() )
		P4OUT = p1;
}

/* ISR for TACCR1, TACCR2 (not available on F2012) and overflow. 
 * It is called at the end of every pulse ~0.8-2.2ms.
 * Will set the current servo pin low and set the next servo pin high. */
interrupt (TIMERA1_VECTOR) isr_TAIV(void)
{ 
	watchdog_clear();

	switch( TAIV )
	{
	case  2: // CCR1
		current_servo++;
		//checks that there is a next servo!
		if(current_servo<SERVO_NUMBER)
		{
			//get servo# pulse width, increment cc1 by its width
			TACCR1 += servo_get_pwm(current_servo);
			set_p1out(~(0x1<<current_servo)); //set servo# output high
		} 
		else
		{
			set_p1out(0xFF);
			TACCR1 = servo_get_pwm(0); //reset CC1 to pulse Servo0
		}
		break;

	case  4: break;       // CCR2 is not present on the F2013
	case 10: break;       // overflow, will occur when
	}
}

interrupt (NOVECTOR) IntServiceRoutine(void)
{

}
