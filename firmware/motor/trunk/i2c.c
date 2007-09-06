#include "common.h"
#include "msp430/usci.h"
#include <signal.h>
#include "i2c.h"
#include "motor.h"

static uint8_t cmd;
static uint8_t pos = 0;

static uint8_t buf[10];

/* Just received a byte   */
void byte_rx( uint8_t pos, uint8_t b );

/* Need to send a byte */
uint8_t byte_tx( uint8_t pos );

interrupt (USCIAB0TX_VECTOR) usci_tx_isr( void )
{
	if( IFG2 & UCB0RXIFG )
	{
		uint8_t tmp = UCB0RXBUF;

		//byte_rx( pos, tmp );
		buf[pos] = tmp;
		pos++;
	}
}

/* start/stop/nack/arb-lost interrupts */
interrupt (USCIAB0RX_VECTOR) usci_rx_isr( void )
{
	/* Start? */
	if( UCB0STAT & UCSTTIFG )
	{
		/* Reset to beginning of register */
		pos = 0;
		FLAG();

		/* Clear the flag */
		UCB0STAT &= ~UCSTTIFG;
	}

	/* Stop? */
	if( UCB0STAT & UCSTPIFG )
	{
		UCB0STAT &= ~UCSTPIFG;
		FLAG_OFF();
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
    
    /* Enable all of the state interrupts */
    UCB0I2CIE = UCNACKIE	/* NACK */
	    | UCSTPIE		/* STOP */
	    | UCSTTIE		/* START */
	    | UCALIE;		/* Arbitration lost */

    /* Clear the interrupt flags */
    IFG2 &= ~( UCB0TXIFG | UCB0RXIFG );

    /* Release from reset */
    UCB0CTL1 &= ~UCSWRST;

    /* Enable the receive and transmit interrupts */
    IE2 |=  UCB0RXIE | UCB0TXIE;
}

void byte_rx( uint8_t pos, uint8_t b )
{
	/* Command byte? */
	if( pos == 0 ) {
		cmd = b;
		return;
	}

	if( pos < 11 )
		buf[pos-1] = b;
	return;


	switch(cmd)
	{
	case M_CONF:
		/* Set motor speed */
		if( pos == 1 )
			buf[0] = b;
		else if( pos == 2 )
		{
			uint8_t channel;
			speed_t speed;
			motor_state_t state;

			buf[1] = b;

			/* Format: bits:
			 * 15-12: Unused
			 *    11: Channel number
			 *  10-9: Mode
			 *   8-0: PWM Ratio */

			channel = (buf[0]&0x08)?1:0;
			speed = ((uint16_t)buf[1]) 
				| ((uint16_t)(buf[0]&1) << 8);
			state = (buf[0] >> 1) & 0x3;


			motor_set( channel, speed, state );
		}
		break;

	}
}

/* Need to send a byte */
uint8_t byte_tx( uint8_t pos )
{
	return 0;
}
