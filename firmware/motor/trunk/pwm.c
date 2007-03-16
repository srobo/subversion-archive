#include "pwm.h"
#include "common.h"

#define PWM_MAX 328

inline void taccr_conf( uint8_t* reg )
{
	/*** Configure TACCTL1 ***/
	/* Compare mode */
}

void pwm_init( void )
{
	/* Configure the timer peripheral */
	/* Clock off crystal */
	/* Frequency ~200Hz */

	/* Clear the timer */
	TACTL |= TACLR;

	/* Set all channels to on for the moment */
	P1OUT &= ~(0x0C);

	/* Select ACLK (watch crystal) as clock source */
	TACTL &= ~TASSEL_3;
	TACTL = TASSEL_ACLK;

	/* No input divider */
	TACTL &= ~ID_3;
	TACTL |= ID_DIV1;

	/* No timer interrupt */
	TACTL &= ~TAIFG;
	
	/*** Configure TACCTL0 ***/
	/* Compare mode */
	TACCTL0 &= ~CAP;
	TACCTL0 &= ~CCIE;


	/*** Configure TACCTLs ***/
	/* Compare mode */
	TACCTL1 = CCIS_3 | OUTMOD_SET_RESET;
	TACCTL2 = CCIS_3 | OUTMOD_SET_RESET;

	TACCR0 = PWM_MAX;
	TACCR1 = PWM_MAX/2;
	TACCR2 = PWM_MAX - (PWM_MAX/3);

	/* Up/down mode - enables the timer*/
	TACTL &= ~MC_3;
	TACTL |= MC_UPTO_CCR0;
}

void pwm_set( uint8_t channel, pwm_ratio_t r )
{

}

pwm_ratio_t pwm_get( uint8_t channel )
{

}
 
