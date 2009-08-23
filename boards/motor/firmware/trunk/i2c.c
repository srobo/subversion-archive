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
#include "types/bool.h"

#define I2C_BUF_LEN 32
#define MODULE_IDENTITY 0x00000001

typedef struct {
	uint32_t id;
} i2c_info_t;

static const i2c_info_t i2c_info = { .id = 0x12345678 };

/* send back the feedback info */
static uint8_t i2cr_motor_fback(uint8_t *buf);

static const i2c_setting_t cmds_bank_0[] = 
{
	/* 0: Send the identity to the master */
	I2C_DESC_SETTING( ST_U32, i2c_info_t, id ),

	/* 1: Firmware version */
	I2C_DESC_FUNC( 0, NULL, i2c_flashr_fw_ver ),

	/* 2: Firmware chunk reception, and next-address transmission */
	I2C_DESC_FUNC( 20, i2c_flashw_fw_chunk, i2c_flashr_fw_next ),

	/* 3: Firmware CRC transmission and confirmation */
	I2C_DESC_FUNC( 4, i2c_flashw_confirm, i2c_flashr_crc ),

	/* 4: Feedback info */
	I2C_DESC_FUNC( 0, NULL, i2cr_motor_fback )
};

i2c_bank_entry_t i2c_banks[7] = 
{
	/* 0: 0-15: General/Misc */
	{ cmds_bank_0, sizeof(cmds_bank_0)/sizeof(i2c_setting_t), (void*)&i2c_info },

	/* 1: 16-31: Controller 0 */
	{ NULL, 0, NULL },
	/* 2: 32-47: Sensor 0 */
	{ NULL, 0, NULL },
	/* 3: 48-63: Control loop 0 */
	{ NULL, 0, NULL },

	/* 4: 64-79: Controller 1 */
	{ NULL, 0, NULL },
	/* 5: 80-95: Sensor 1 */
	{ NULL, 0, NULL },
	/* 6: 96-102: Control loop 1 */
	{ NULL, 0, NULL },
};

#define N_BANKS (sizeof(i2c_banks) / sizeof(i2c_bank_entry_t))
#define CMD_GET_BANK(n) ( n >> 4 )
#define CMD_GET_OFFSET(n) ( n & 0x0f )
#define CMD_GET_BASE(n) ( i2c_banks[ CMD_GET_BANK(n) ].base )

/* The current command */
static const i2c_setting_t *cmd = NULL;
uint8_t cmd_n = 0;

/* Whether we just got a start bit */
static bool at_start = FALSE;

static uint8_t pos = 0;
static uint8_t buf[I2C_BUF_LEN];
static uint8_t checksum;

/* Finds settings structure representing the given command */
/* NULL if the command doesn't exist */
static const i2c_setting_t *find_cmd( uint8_t n )
{
	uint8_t bank, off;
	const i2c_bank_entry_t *entry;

	/* Chop off lower bits to find bank */
	bank = CMD_GET_BANK(n);
	if( bank > N_BANKS )
		return NULL;

	entry = &i2c_banks[bank];

	/* Use lower bits for offset in second table */
	off = CMD_GET_OFFSET(n);
	if( off > entry->tblen )
		return NULL;

	return &entry->settings[off];
}

interrupt (USCIAB0TX_VECTOR) usci_tx_isr( void )
{
	if( IFG2 & UCB0RXIFG )
	{
		uint8_t tmp = UCB0RXBUF;
		static uint8_t rx_size = 0;

		/* Command? */
		if( at_start )
		{
			cmd = find_cmd(tmp);
			cmd_n = tmp;
			rx_size = i2c_desc_get_rx_size( cmd );

			checksum = crc8( I2C_ADDRESS << 1 );
			checksum = crc8( checksum ^ tmp );

			at_start = FALSE;
		}
		else if( cmd != NULL && rx_size != 0 )
		{
			if( pos < rx_size )
			{
				buf[pos] = tmp;
				checksum = crc8( checksum ^ tmp );
			}
			
			pos++;
			
			if( pos == rx_size + (USE_CHECKSUMS?1:0)
			    && ( !USE_CHECKSUMS || checksum == tmp ) )
				i2c_desc_write( buf,
						cmd,
						CMD_GET_BASE(cmd_n) );
		}
	}

	if( IFG2 & UCB0TXIFG )
	{
		static uint8_t size = 0;
		uint8_t tmp = 0;

		if( cmd != NULL )
		{
			if( pos == 0 ) 
			{
				size = i2c_desc_read( buf,
						      cmd,
						      CMD_GET_BASE(cmd_n) );
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

