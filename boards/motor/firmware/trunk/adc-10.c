#include "common.h"
#include "adc-10.h"
#include <msp430/adc10.h>
#include <stdint.h>

/* adc buffer, pointer */
uint8_t adc_channel;
uint16_t currents[2]; 

void adc10_init( void )
{
	/* Enable A12 and A13 as analogue inputs */
	ADC10CTL0 |= 	SREF_0
		| ADC10SHT_DIV64
		| ADC10ON
		| REF2_5V;

	ADC10CTL1 |=  INCH_12
		| ADC10DIV_0
		| ADC10SSEL_0
		| CONSEQ_0;

	ADC10AE1 |= 0x30;

	ADC10DTC1 = 0x00;

	/* End Current Sense Initialisation */

	/* start conversion */
	adc_channel = 0;	
	ADC10CTL0 |= ENC 
		| ADC10SC;	/* Turn on ADC & enable conversion, start */
}

void adc10_proc( void )
{
	if(ADC10BUSY)
	{
		currents[adc_channel] = ADC10MEM;
		if(adc_channel)
		{
			adc_channel = 0x00;	//change the channel
			ADC10CTL1 &= 0xF000;
			ADC10CTL1 |= INCH_12;
			ADC10CTL0 |= ENC 
				| ADC10SC;	/* Turn on ADC & enable conversion, start */
		}
		else
		{
			adc_channel = 0x01;	//change the channel
			ADC10CTL1 &= 0xF000;
			ADC10CTL1 |= INCH_13;
			ADC10CTL0 |= ENC 
				| ADC10SC;	/* Turn on ADC & enable conversion, start */
		}
				
	}
}
