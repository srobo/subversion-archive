#include "adc.h"
#include "device.h"
#include <signal.h>
#include <stdio.h>
#include "led.h"

uint16_t voltage=0;
uint16_t current=0;

interrupt (ADC12_VECTOR) adc_service( void )
{

	uint8_t adc12v_l = ADC12IV;
	/* We only care about the interrupt relating to the last in 
	   the conversion sequence - which is the only one that we enabled */
	if ( adc12v_l == 0x08)
	{
		toga;
		current = ADC12MEM0;
		voltage = ADC12MEM1;
		ADC12IFG &= ~0x02; /* clear flag */
	}

	else
	{
		togc;
		/* possible error handler from other misc iv's */
		ADC12IFG = 0; /* clear everything else */
	}
}


void adc_init( void )
{
	P6DIR &= ~0x03;		/* 6.0 + 6.1 as inputs */
	P6SEL |= 0x03;		/* disable dio */


	ADC12CTL0 = ADC12ON;
	ADC12CTL0 |= SHT0_0 | SHT1_0 /* sample and hold fastest for now to avoid trip hazard, */
	       	| MSC		     /* multi sample */
		| REF2_5V	     /* ref voltage 2.5v */
		| REFON		     /* ref generator on */
		| ADC12ON;	     /* adc module on */
		/* overflows not on */
		/* enc set later */


	ADC12CTL1 = CSTARTADD_0	/* start at 0 */
		| SHS_ADC12SC	/* conversion start from timera cc1 */
		| SHP
		/* no ISSH? */
		| ADC12DIV_0	/* no adc divider */
		| ADC12SSEL_0	/* clock from adcclock */
		| CONSEQ_REPEAT_SEQUENCE;	/* single sequence of channels */

	ADC12MCTL0 = SREF_1	/* use internal referance */
		| INCH_0;	/* source = ch0 = current sense out*/
		
	ADC12MCTL1 = EOS  	/* last memory in sequence */
		| SREF_1	/* use internal referance */
		| INCH_1;	/* source = ch0 = voltage sense out*/

//	ADC12IFG = 0;		/* clear any erroneous flags */
	ADC12IE = 0x2;		/* set interrupt on last conversion */



	/* movethis to a post warm up timer */
	       
	ADC12CTL0 |= ENC;
	/* Start the conversion */
	ADC12CTL0 |= ADC12SC;

		
}
