/*   Copyright (C) 2007 Robert Spanton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */
#include "common.h"
#include "msp430/usci.h"
#include <signal.h>
#include "i2c.h"


static uint8_t cmd;
static uint8_t pos = 0;
static uint8_t buf[10];

#define MODULE_IDENTITY 0x0201
#define FIRMWARE_REV 0x0304
static const uint8_t i2c_identity[] = { (MODULE_IDENTITY >> 8) & 0xFF,
					MODULE_IDENTITY & 0xFF, 
					(FIRMWARE_REV >> 8) & 0xFF,
					FIRMWARE_REV & 0xFF };
/* Just received a byte */
void byte_rx( uint8_t pos, uint8_t b );

/* Need to send a byte */
uint8_t byte_tx( uint8_t pos );

interrupt (USCIAB0TX_VECTOR) usci_tx_isr( void )
{
	if( IFG2 & UCB0RXIFG )
	{
		uint8_t tmp = UCB0RXBUF;

		byte_rx( pos, tmp );
		pos++;
	}

	if( IFG2 & UCB0TXIFG )
	{
		UCB0TXBUF = byte_tx(pos);
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

	switch(cmd)
	{
	case M_OUTPUT:
		/* Write data to port */
		P1OUT = b;
		break;

	}
}

/* Need to send a byte */
uint8_t byte_tx( uint8_t pos )
{
	const uint8_t lengths[] = {4,0};

	if( cmd >= M_LAST_COMMAND )
		return 0;

	if( pos > lengths[cmd] )
		return 0;	/* CRC to go here! */

	switch(cmd)
	{
	case M_IDENTIFY:
		return i2c_identity[pos];
		break;
	} 

	return 0;
}
