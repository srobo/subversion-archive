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

void adc_init(void){
			//Use Vcc and Vss as referance voltages, 
	ADC10CTL0 = SREF_AVCC_AVSS| ADC10SHT_DIV64 | ADC10SR | ADC10ON | ENC | ADC10SC;	
	ADC10CTL1 = ADC10DIV_7;	//Set to single conversion mode
	ADC10AE = 0xFF;		//Enable all inputs as Analogue Inputs
	P2SEL |= 0x1F;		//Set A0 - A4 as Input Pins	
}


