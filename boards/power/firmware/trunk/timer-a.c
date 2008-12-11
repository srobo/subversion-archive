#include "device.h"
#include "timer-a.h"
#include <signal.h>


interrupt (TIMERA1_VECTOR) timer_a_service( void )
{

	uint8_t taiv_l = TAIV;	/* buffer so only one read as per datasheet */

	switch( taiv_l )
	{
	case TAIV_CCR1:
		/* Disable this interrupt */
		TACCTL1 &= ~CCIE;
		/* do something here */
	}
}


void timera_init( void )
{

	TACTL &= ~( MC_1 | MC_0 );	/* stop timer so can config */

	TACTL = TASSEL2		/* SMClock, TA Interrupt off */
		| TACLR;			/* Clear timer, divider and direction */

	TACTL |= ID_3;		/* Prescale - div/8  */

	/* capture control regs */
	TACCTL0 = 0;
	TACCTL1 = 0;
	TACCTL2 = 0;

	TACTL |= MC_2;			/* start timer */
}
