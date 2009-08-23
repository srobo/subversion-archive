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
#include <msp430/usci.h>
#include <signal.h>
#include <bool.h>
#include "i2c.h"
#include "motor.h"
#include "smbus_pec.h"
#include "timer-b.h"
#include "flash430/i2c-flash.h"

#define I2C_BUF_LEN 32
#define MODULE_IDENTITY 0x0201
#define FIRMWARE_REV 0x0304

static const uint8_t i2c_identity[] = { (MODULE_IDENTITY >> 8) & 0xFF,
					MODULE_IDENTITY & 0xFF, 
					(FIRMWARE_REV >> 8) & 0xFF,
					FIRMWARE_REV & 0xFF };

typedef struct
{
	/* The receive size - 0 if receive not supported*/
	uint8_t rx_size;

	/* Receive function - processes buf data */
	void (*rx) ( uint8_t* buf );

	/* Transmit function - fills buf with data.
	   Returns length of the data. */
	uint8_t (*tx) ( uint8_t* buf );
} i2c_cmd_t;

/* Transmit (read) functions */
static uint8_t i2cr_identity( uint8_t *buf );

/* send back the feedback info */
static uint8_t i2cr_motor_fback(uint8_t *buf);

const i2c_cmd_t cmds[] = 
{
	/* Send the identity to the master */
	{ 0, NULL, i2cr_identity },

	/* Firmware version */
	{ 0, NULL, i2c_flashr_fw_ver },
	/* Firmware chunk reception, and next-address transmission */
	{ 20, i2c_flashw_fw_chunk, i2c_flashr_fw_next },
	/* Firmware CRC transmission and confirmation */
	{ 4, i2c_flashw_confirm, i2c_flashr_crc },

	/* Feedback info */
	{0, NULL, i2cr_motor_fback}
}; 

/* The current command */
static const i2c_cmd_t *cmd = NULL;
/* Whether we just got a start bit */
static bool at_start = FALSE;

static uint8_t pos = 0;
static uint8_t buf[I2C_BUF_LEN];
static uint8_t checksum;

interrupt (USCIAB0TX_VECTOR) usci_tx_isr( void )
{
	if( IFG2 & UCB0RXIFG )
	{
		uint8_t tmp = UCB0RXBUF;

		/* Command? */
		if( at_start )
		{
			if( tmp < sizeof(cmds) )
				cmd = &cmds[tmp];
			else
				cmd = NULL;

			checksum = crc8( I2C_ADDRESS << 1 );
			checksum = crc8( checksum ^ tmp );

			at_start = FALSE;
		}
		else if( cmd != NULL && cmd->rx != NULL )
		{
			if( pos < cmd->rx_size )
			{
				buf[pos] = tmp;
				checksum = crc8( checksum ^ tmp );
			}
			
			pos++;
			
			if( pos == cmd->rx_size + (USE_CHECKSUMS?1:0) )
			{
				if( !USE_CHECKSUMS || checksum == tmp )
					cmd->rx( buf );
			}
		}
	}

	if( IFG2 & UCB0TXIFG )
	{
		static uint8_t size = 0;
		uint8_t tmp = 0;

		if( cmd != NULL && cmd->tx != NULL )
		{
			if( pos == 0 ) 
			{
				size = cmd->tx( buf );
				checksum = crc8( checksum ^ ((I2C_ADDRESS << 1)|1) );
			}
	
			if( pos < size )
				tmp = buf[ pos ];

			if( USE_CHECKSUMS )
			{
				if( pos == size )
					tmp = checksum;
				else
					checksum = crc8( checksum ^ tmp );
			}

			/* Random high number to avoid overflow situations */
			if( pos < 128 )
				pos++;
		}
		
		UCB0TXBUF = tmp;
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
		at_start = TRUE;
		FLAG();

		/* Start the reset generator */
		timer_b_start();

		/* Clear the flag */
		UCB0STAT &= ~UCSTTIFG;
	}

	/* Stop? */
	if( UCB0STAT & UCSTPIFG )
	{
		UCB0STAT &= ~UCSTPIFG;
		FLAG_OFF();

		/* We don't need to reset things */
		timer_b_stop();
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

    /* Let the peripheral use the pins */
    P3SEL |= 6;

    /* Release from reset */
    UCB0CTL1 &= ~UCSWRST;

    /* Enable the receive and transmit interrupts */
    IE2 |=  UCB0RXIE | UCB0TXIE;

    FLAG_OFF();
}

static uint8_t i2cr_identity( uint8_t *buf )
{
	uint8_t i;

	for(i=0; i<4; i++)
		buf[i] = i2c_identity[i];

	return 4;
}

/* send back the feedback info */
static uint8_t i2cr_motor_fback(uint8_t *buf)
{
	buf[0] = P2IN & 0x0f;

	return 1;
}

void i2c_reset( void )
{
	i2c_init();
}

