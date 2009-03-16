#include "railmon.h"
#include <signal.h>

void railmon_init( void )
{
	/* Configure pin as input  */
	P2DIR &= ~RAIL_MONITOR_PIN;

	/* Enable high->low interupt on VCC detect pin */
	P2IES |= RAIL_MONITOR_PIN;
	P2IE |= RAIL_MONITOR_PIN;

	/* clear all flags that might have been set */
	P2IFG = 0x00;
}

/* ISR for IO interrupt */
interrupt (PORT2_VECTOR) isr_port2(void)
{
	/* The rail has dropped - set the outputs to be low. */
	P4OUT = 0x00;
	P2IFG = 0x00;
}
