#include "timer-b.h"
#include "common.h"
#include "i2c.h"
#include <signal.h>

#define MC 0x30
#define MC_UP 0x10

#define timer_b_dis() do { TBCTL &= ~MC; } while(0)
#define timer_b_en() do { TBCTL |= MC_UP; } while(0)

void timer_b_init( void )
{
	/* Up mode */
	TBCTL = 		/* TBCLx = 0 - independent */
		CNTL_0		/* 16-bit timer mode */
		| TBSSEL_ACLK	/* ACLK clock source */
		/* ID = 0 - Don't divide the clock */
		/* Stop mode for now */
		/* TBCLR = 0 - Don't reset the timer */
		| TBIE;		/* Interrupt enabled */

	TBCCR0 = 16000;

	timer_b_dis();
}

/* FFF8 - TBIFG */
interrupt (TIMERB1_VECTOR) timer_b_isr( void )
{
	i2c_reset();
	timer_b_stop();
}

/* FFFA - TBCCR0 CCIFG */
interrupt (TIMERB0_VECTOR) timer_b_isr2( void )
{
	nop();
}

/* Start the timer */
void timer_b_start( void )
{
	TBR = 0;
	timer_b_en();
}

/* Stop the timer */
void timer_b_stop( void )
{
	timer_b_dis();
}
