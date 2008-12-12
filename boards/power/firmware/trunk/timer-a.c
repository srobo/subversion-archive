#include "device.h"
#include "timer-a.h"
#include <signal.h>
#include "led.h"

#define PERIOD2 60240		/* this gives interupts every 0.1 sec appx(DCO) */
/* timer tics at appx 1.66us per tick don;y appx because derived from dco */
interrupt (TIMERA1_VECTOR) timera_service( void )
{

	uint8_t taiv_l = TAIV;	/* buffer so only one read as per datasheet */
	toga;
	switch( taiv_l )
	{
	case TAIV_CCR1:	     
		togc;
		/* do something here */
		break;
	case TAIV_CCR2:
		TACCR2 += PERIOD2;
		togb;
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
	TACCTL1 = 0;
	TACCTL2 = CM_0 |		/* no capture */
		CCIS_3 |	/* no in or out pins connected */
		SCS |		/* sychronise capture - superflous? */
		/* no CAP so in compare mode */
		OUTMOD_0|	/* normal output, poss irrellevant */
		CCIE ;		/* cc interupt en */
		
	TACCR2 = PERIOD2;

	TACTL |= MC_2;			/* start timer */
}
