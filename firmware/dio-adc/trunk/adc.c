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

#define BLOCK_SIZE 8


int block[BLOCK_SIZE];		//8 bit buffer for DTC block

void adc_init(void){



			//Setup ADC10CTL0 register 
	ADC10CTL0 = SREF_AVCC_AVSS ;	//use vcc/vss as references
	ADC10CTL0 |= ADC10SHT_DIV64 ;	//sample and hold time of 64clk cycles
	ADC10CTL0 |= ADC10SR;		//use lowest sample rate
	ADC10CTL0 |= ADC10ON;		//enable ADC 
	ADC10CTL0 |= ENC;		//enable conversion
	ADC10CTL0 |= ADC10SC;		//start conversion
	ADC10CTL0 |= ADC10IE;		//enable interrupts
	ADC10CTL0 |= MSC;		//multiple sample conversion mode

			//Setup ADC10CTL1 register
	ADC10CTL1 = ADC10DIV_7;		// divide clock by 8 for conversion
	ADC10CTL1 |= INCH_7; 		// convert channels A7 through A0
	ADC10CTL1 |= CONSEQ_3;		// repeated sequence of conversion mode
		
			//Setup inputs
	ADC10AE = 0xFF;		//Enable all inputs as Analogue Inputs
	P2SEL |= 0x1F;			//Set A0 - A4 as Input Pins	
	
			//Setup Data Transfer Controller
	ADC10DTC0 |= 0x00;		//Single block mode
	ADC10DTC0 |= ADC10CT;		//Continuous data transfer
	
	ADC10DTC1 = 0x0F;		//8 transfers per block, (1 transfer per input)
	
	ADC10SA = (int) &block;		//start address of block 
}

int *  adc_sample(void){			
	return &block[0];			//returns pointer to the first conversion in the buffer
}

interrupt (ADC10_VECTOR) intr_adc( void ){
	
}

