#include "i2c.h"
#include "i2c_reg.h"
#include "types.h"
#include "device.h"
#include "smbus_pec.h"
#include <signal.h>
//#include "gum-watchdog.h"

typedef enum 
{
	S_I2C_IDLE, 
	S_I2C_ADDRESSED,
	S_I2C_GOT_COMMAND,
	S_I2C_GOT_LEN
} i2c_state_t;

/* The buffer for sending/receiving */
static uint8_t i2c_buf[32];

static uint8_t checksum;

interrupt (USART0TX_VECTOR) i2c_service( void )
{
	uint8_t d;
	/* Position within current register */
	static uint16_t rpos, rsize;
	uint8_t i2civ_l;

	static i2c_state_t state = S_I2C_IDLE;
	static uint8_t command;

	/* Store this locally, since accesses cause it to change value :-D */
	i2civ_l = I2CIV;

	switch (i2civ_l)
	{
	case I2CIV_AL:
		/* Arbitration lost */
		/* Doesn't happen in slave mode */

		I2CIFG &= ~ALIFG;
		break;

	case I2CIV_NACK:
		/* No acknowledgement */
		/* Doesn't happen in slave mode */

		I2CIFG &= ~NACKIFG;
		break;

	case I2CIV_OA:
		/* Own Address received */
		rpos = 0;
		state = S_I2C_ADDRESSED;

		I2CIFG &= ~OAIFG;
		break;

	case I2CIV_ARDY:
		/* STOP Detected */
		/* Avoid using this, because of CPU bug I2C7 */
		I2CIFG &= ~ARDYIFG;
		break;

	case I2CIV_RXRDY:
		/* Just received some data */
		/* Will either be address or register data */

		d = I2CDRB;		/* Grab data */

		if( state == S_I2C_ADDRESSED )
		{
			/* Byte is the command */
			command = d;

			/* Initialise the checksum */
			/* (each transaction begins with a reception) */
			checksum = crc8(I2C_ADDRESS<<1);
			checksum = crc8( checksum ^ d );			

			if( command < I2C_NUM_COMMANDS )
			{
				state = S_I2C_GOT_COMMAND;

				rpos = 0;
				rsize = dev_regs[command].size();
			}
			else
				state = S_I2C_IDLE;
		}
		else if( state == S_I2C_GOT_COMMAND )
		{
			/* This is the length byte */
			/* Which we can ignore */
			checksum = crc8( checksum ^ d );

			state = S_I2C_GOT_LEN;
		}
		else if( state == S_I2C_GOT_LEN )
		{
			if( rpos < rsize )
			{
				if( rpos < 32 ) {
					i2c_buf[rpos] = d;
					rpos++;
				}
				
				checksum = crc8( checksum ^ d );
/* 				if(rpos == rsize) /\* breakpoint rtest *\/ */
/* 					rpos = rsize; */
			}
			else if( rpos == rsize)
			{
				if (checksum == d)
				{
					/* Checksum is valid */
					//gum_watchdog_clear();
					
					if( dev_regs[command].write != NULL )
						dev_regs[command].write( i2c_buf, rsize );
				}
			}
		}

		I2CIFG &= ~RXRDYIFG;
		break;

	case I2CIV_TXRDY:
		/* New data required for transmission */
		/* Register value must have been set */
		
		if( state == S_I2C_IDLE )
			d = 0;
		else
		{
			/* First byte is the register size */
			if( rpos == 0 )
			{
				if( dev_regs[command].read != NULL )
					rsize = dev_regs[command].read( i2c_buf );
				else
					rsize = 0;

				d = rsize;

				/* Read address */
				/* Because of the block read funniness, we have to clear the
				   checksum here. */
				checksum = crc8( (I2C_ADDRESS<<1)|1 );
			}
			else if( rpos == 1 )
				/* This is required (see README) */
				d = command;
			else if( rpos - 2 < rsize )
				d = i2c_buf[rpos-2];
			else if( rpos - 2 == rsize )
				d = checksum;
			else 
				d = 0;

			checksum = crc8( checksum ^ d );
			rpos ++;
		}

		I2CDRB = d;
		//gum_watchdog_clear();

		I2CIFG &= ~TXRDYIFG;
		break;
      
	case I2CIV_GC:
		/* General call address detected */

		I2CIFG &= ~GCIFG;
		break;

	case I2CIV_STT:
		/* Start condition detected */

		I2CIFG &= ~STTIFG;
		break;
	}  
}

void i2c_init( void )
{
  /* Switch to I2C Mode */
	// U0CTL |= SYNC + I2C;
  U0CTL = SYNC | I2C;

  /* Clear I2CEN for configuration */
  U0CTL &= ~I2CEN;

  /* Slave mode, 7-bit addressing, non-loopback */
  U0CTL &= ~(XA | LISTEN | MST);


  I2CTCTL = 0;
  /* byte mode */
  I2CTCTL &= ~I2CWORD;
  
  /* SMCLK clock source */
  I2CTCTL &= ~I2CSSEL0;
  I2CTCTL |= I2CSSEL1;
  /* Number of bytes transmitted not hardware controlled */
  I2CTCTL |= I2CRM;

  /* No clock prescaling */
  I2CPSC = 0;			

  /* Don't care about I2CSCLRH or I2CSCLL, since we're
     in slave mode. */

  /* Set own address */
  I2COA = I2C_ADDRESS;

  /*** Interrupts ***/
  I2CIE = STTIE | TXRDYIE | RXRDYIE | ARDYIE | OAIE;

  /* set I2CEN to enable peripheral */
  U0CTL |= I2CEN;
}


