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
    
#include "adc.h"
#include "common.h"
#include <msp430/adc10.h>
#include <signal.h>

uint16_t adc_buffer[8];

void adc_init(void){

	/* Setup ADC10CTL0 register */
	ADC10CTL0 = SREF_AVCC_AVSS ;	//use vcc/vss as references
	ADC10CTL0 |= ADC10SHT_DIV64 ;	//sample and hold time of 64clk cycles
	ADC10CTL0 |= ADC10SR;		//use lowest sample rate
	ADC10CTL0 |= ADC10ON;		//enable ADC 

	/* Setup ADC10CTL1 register */
	ADC10CTL1 = ADC10DIV_7;		// divide clock by 8 for conversion
	ADC10CTL1 |= INCH_0; 		// convert channels A7 through A0
	ADC10CTL1 |= CONSEQ_0;		// repeated sequence of conversion mode
		
	/* Setup inputs */
	ADC10AE = 0xFF;			//Enable all inputs as Analogue Inputs
	P2SEL |= 0x1F;			//Set A0 - A4 as Input Pins 
}

void adc_sample( void )
{
	uint8_t y;

	ADC10CTL0 |= ENC | ADC10SC;	//start and enable conversion

	for(y=0;y<8;y++){
	
		ADC10CTL0 &= ~ENC;
		ADC10CTL1 &= 0x0FFF;
		ADC10CTL1 |= (y << 12);
		ADC10CTL0 |= ENC;
		ADC10CTL0 |= ADC10SC;

		while(ADC10BUSY & ADC10CTL1);

		adc_buffer[y] = ADC10MEM;
	}
}

uint8_t adc_digitise( void )
{
	uint8_t i,b;

	b=0;
	for(i=0; i<8; i++)
		if( adc_buffer[i] > INPUT_THRESHOLD )
			b |= 1<<i;

	return b;
}
