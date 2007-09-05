#include "common.h"
#include "msp430/usci.h"
#include <signal.h>
#include "i2c.h"

interrupt (USCIAB0TX_VECTOR) usci_tx_isr( void )
{
}

interrupt (USCIAB0RX_VECTOR) usci_rx_isr( void )
{
	/* Did we just receive a START? */
	if( UCB0STAT & UCSTTIFG )
	{
		
	}
     
}

void i2c_init( void )
{
    /* After power-up, USCI is held in reset (UCSWRST set) */
    /* Set UCSWRST just in case */
    UCB0CTL1 |= UCSWRST;

    UCB0CTL0 = /* UCA10: 7 bit address */
	    /* USCLA10: Don't care about slave addressing */
	    /* UCMM: Don't care about multi-master */
	    /* UCMST: Slave mode */
	    UCMODE_I2C		/* I2C mode */
	    | UCSYNC;		/* Synchronous mode */

    /* Don't want to touch UCSWRST whilst fiddling with this register */
    UCB0CTL1 &= UCSWRST;
    UCB0CTL1 |= UCSSEL_SMCLK	/* Clock source is SMCLK */
	    /* UCTR: receive mode for now */
	    /* UCTXNACK: ACK normally */
	    /* UCTXSTP: No STOP now */
	    /* UCTXSTT: No START now */;

    UCB0I2COA = I2C_ADDRESS;
    
    /* Enable the receive and transmit interrupts */
    IE2 |= UCB0TXIE | UCB0RXIE;
    
    /* Enable all of the state interrupts */
    UCB0I2CIE |= UCNACKIE	/* NACK */
	    | UCSTPIE		/* STOP */
	    | UCSTTIE		/* START */
	    | UCALIE;		/* Arbitration lost */

    /* Release from reset */
    UCB0CTL1 &= ~UCSWRST;
}
