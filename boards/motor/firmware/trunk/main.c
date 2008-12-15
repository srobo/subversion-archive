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
#include <msp430/adc10.h>

static int i = 0;
static uint8_t adc_channel = 0;
static uint16_t currents [2] = {0, 0};
 
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

	/* start conversion */
	adc_channel = 1;	
	ADC10CTL1 &= 0x0FFF;
	ADC10CTL1 |= 0xC000;	/* start with motor 0 current sense */
	ADC10CTL0 |= 0x0019;	/* Turn on ADC & enable conversion, start */

	
	while(1) {
		if( i2c_flash_received ) {
				flash_switchover();
		}
		if(ADC10BUSY)
		{
			currents[adc_channel] = ADC10MEM >> 6;
			if(adc_channel)
			{
				adc_channel = 0x01;	//change the channel
				ADC10CTL1 &= 0xF000;
				ADC10CTL1 |= 0xD000;
				ADC10CTL0 |= 0x0019;	//start next conversion
			}
			else
			{
				adc_channel = 0x00;	//change the channel
				ADC10CTL1 &= 0xF000;
				ADC10CTL1 |= 0xC000;
				ADC10CTL0 |= 0x0019;	//start next conversion
			}
				
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

	/* Current Sense initialisation TODO: Move into serparate header?*/
	/* set up MAX4069 chips with GSEL signal to give gain of 50 */
	P4DIR |= 0x60;
	P4OUT &= ~0x60;

	/* Enable A12 and A13 as analogue inputs */
	ADC10AE1 |= 0x30;
	/* Setup up ADC Control register - do not start it*/
	ADC10CTL0 =  0x1480;
	/* ADC Control Register - channel, hold time, non-invert,
 	ADC clock source & division, single channel-single convert.*/
	ADC10CTL1 |= 0xC0E0;
	/*Disable Data Transfer Control Reg. TODO: perhaps use this in future */
	ADC10DCTL1 = 0x00;

	/* End Current Sense Initialisation */
	/* Debug light off */
	FLAG_OFF();
}
