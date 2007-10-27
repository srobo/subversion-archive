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
#include "i2c.h"
#include "init.h"
#include <msp430/adc10.h>

static int i = 0;
void adc_init(void);

void UNEXPECTED()
{
}

int main( void )
{
	i = 0;
	unsigned int adresult[8] ;
	
	/* Disable the watchdog timer */
	WDTCTL = WDTHOLD | WDTPW;

	init();
	adc_init();
	adresult[0] = 0;
	
	while(1){
		if(! (ADC10CTL1 & ADC10BUSY)){
			adresult[0] = ADC10MEM;
			ADC10CTL0  |= ADC10SC;	
			
		}
	};
}

void adc_init(void){
			//Use Vcc and Vss as referance voltages, 
	ADC10CTL0 = SREF_AVCC_AVSS| ADC10SHT_DIV64 | ADC10SR | ADC10ON | ENC | ADC10SC;	
	ADC10CTL1 = ADC10DIV_7;
	ADC10AE = 0xFF;		//Enable all inputs as Analogue Inputs
	P2SEL |= 0x1F;		//Set A0 - A4 as Input Pins
	
	
}
