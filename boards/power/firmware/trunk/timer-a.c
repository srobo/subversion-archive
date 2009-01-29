#include "device.h"
#include "timer-a.h"
#include <signal.h>
#include "led.h"
#include "timed.h"
#define PERIOD1 30000
#define PERIOD2 60240		/* this gives interupts every 0.1 sec appx(DCO) */

/* timer tics at appx 1.66us per tick don;y appx because derived from dco */
interrupt (TIMERA1_VECTOR) timera_service( void )
{
	uint8_t taiv_l = TAIV;	/* buffer so only one read as per datasheet */
	switch( taiv_l )
	{
	case TAIV_CCR1:	     
		/* do something here */
		TACCR1 += PERIOD1;
		ADC12CTL0 &= ~ENC; /* wibble the enable - datasheet says so */
		ADC12CTL0 |= ENC;
		break;
	case TAIV_CCR2:
		TACCR2 += PERIOD2;
		alive_service();
		safe_service();
		break;
		
	}
}

void timera_init( void )
{
	TACTL &= ~( MC_1 | MC_0 );	/* stop timer so can config */

	TACTL |= TASSEL_2;		/* SMClock, TA Interrupt off */
	TACTL |= TACLR;			/* Clear timer, divider and direction */

	TACTL |= ID_3;		/* Prescale - div/8  */

	/* capture control regs */
	TACCTL0 = 0;
	/* cc1 triggers adc reading */
	TACCTL1 = CM_0 | 	/* no capture */
		CCIS_3 |	/* no in, out triggers adc12 */
		SCS |		/* sychronise capture - superflous? */
		/* no CAP so in compare mode */
		OUTMOD_0|	/* normal output, poss irrellevant */
		CCIE ;		/* cc interupt en */
		

	/* cc2 generates 0.1s ish tick */
	TACCTL2 = CM_0 |		/* no capture */
		CCIS_3 |	/* no in or out pins connected */
		SCS |		/* sychronise capture - superflous? */
		/* no CAP so in compare mode */
		OUTMOD_0|	/* normal output, poss irrellevant */
		CCIE ;		/* cc interupt en */
		
	TACCR1 = PERIOD1;	
	TACCR2 = PERIOD2;

	TACTL |= MC_2;			/* start timer */
}
