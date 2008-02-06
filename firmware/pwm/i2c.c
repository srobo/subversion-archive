/****
I2c Section, implements the i2c hardware code and the SMBUS protols
SMBUS protols supported:
Write word protocol
Read word protocol
Block read protocol
**/
#include "hardware.h"
#include "i2c.h"
#include "servo.h"
#include "smbus_pec.h"
#include "i2c-watchdog.h"

typedef enum
{
	state_idle = 0,
	state_rx_address,
	state_check_address,
	state_rx_command,
	state_check_command,
	state_rx_data,
	state_check_data,
	state_prep_for_start
}state_t;

/* Command lengths */
static const uint8_t command_len[] = 
{
	0,			/* COMMAND_IDENTIFY */
	2,			/* COMMAND_SET */
	0			/* COMMAND_READ */
};

/* Buffer for I2C data */
uint8_t i2c_data[32];

/* The I2C transmission state machine state */
state_t I2C_State = state_idle;

/* The number of bytes that have currently been received */
static uint8_t data_pos = 0;

/* The number of bytes to be received */
static uint8_t num_bytes = 0;

static uint8_t checksum;

/*** Device Macros ***/
/* Set SDA to an output */
#define sda_output() do { USICTL0 |= USIOE; } while (0)
/* Set SDA to an input */
#define sda_input() do { USICTL0 &= ~USIOE; } while (0)

void i2c_init(void)
{
	/* Hold the I2C device in reset */
	USICTL0 |= USISWRST;

	USICTL0 = USIPE6	/* USI SDO/SCL Port Enabled */
		| USIPE7;	/* USI SDI/SDA Port Enabled */
				/* USILSB = 0 - MSB First */
				/* USIMST = 0 - Slave Mode */
				/* USIGE = 0 - Output latch depends on shift clock */
				/* USIOE = 0 - Output disabled */
				/* USISWRST = 0 - Device out of reset mode */

	USICTL1 = 		/* USICKPH = 0 - Data on first SCLK edge. */
		USII2C;		/* I2C Mode */
				/* USISTTIE = 0 - No interrupt on START */

	USICKCTL = USICKPL;	/* Inactive Clock state is high */

	USICNT |= USIIFGCC;	/* USIIFG not cleared automatically */
}

void i2c_enable(void)
{
	USICTL0 &= ~USISWRST;                // Enable USI
	USICTL1 &= ~USIIFG;                  // Clear pending flag
}

inline void isr_usi (void)
{
	/* START Received */
	if (USICTL1 & USISTTIFG)
	{
		/* Move into the state machine */
		I2C_State = state_rx_address;

		i2c_watchdog_start();
	}

	if( USICTL1 & USISTP )
	{
		i2c_watchdog_stop();

		/* Clear the STOP interrupt flag */
		USICTL1 &= ~USISTP;
	}

	switch(I2C_State)
	{
		/* Idly doing nothing */
	case state_idle:
		break;

		/* Receive the address */
	case state_rx_address:
		sda_input();

		USICNT = (USICNT & 0xE0) | 0x08;

		/* Clear the START flag */
		USICTL1 &= ~USISTTIFG;

		I2C_State = state_check_address;
		break;

		/* Check the received address */
	case state_check_address:
		/* Our Address? */
		if ( (USISRL & 0xFE) == (ADDRESS << 1) )
		{
			sda_output();

			data_pos = 0;

			/* Initialise the checksum */
			checksum = crc8( USISRL );

			/* Send ACK */
			USISRL = 0x00;
			/* ACK is 1 bit */
			USICNT |= 0x01; 

			/* Start receiving things */
			I2C_State = state_rx_command;
		}
		else
			I2C_State = state_idle;
		break;

		/* Prepare to receive first data byte */
	case state_rx_command:
		sda_input();
		/* 8-bits to receive */
		USICNT |=  0x08;

		I2C_State = state_check_command;
		break;

		/* Process a command byte */
	case state_check_command:
		sda_output();

		/* Process the command */
		if( USISRL < sizeof( command_len ) )
		{
			num_bytes = command_len[ USISRL ];

			checksum = crc8( checksum ^ USISRL );

			I2C_State = state_rx_data;
		}
		else
			/* Command is out of range */
			I2C_State = state_prep_for_start;

		/* Send ACK */
		USISRL = 0x00;
		USICNT |= 0x01;
		break;

		/* Receive data */
	case state_rx_data:

		/* When using checksums we need to receive an additional byte */
		if( data_pos < num_bytes + (USE_CHECKSUMS?1:0) )
		{ 
			/* More data to receive */
			sda_input();

			USICNT |=  0x08;

			I2C_State = state_check_data;
		}
		else
		{
			servo_set_pwm( i2c_data[0],
				       (MIN_PULSE + ((MAX_PULSE-MIN_PULSE)/255)*(uint16_t)i2c_data[1]));
			data_pos = 0;

			/* START condition will happen next */
			sda_input();
			I2C_State = state_idle;
		}
		break;
		
		/* Store the received data */
	case state_check_data:
		sda_output();

		if( !USE_CHECKSUMS || data_pos != num_bytes )
		{
			i2c_data[data_pos] = USISRL;

			checksum = crc8( checksum ^ USISRL );
		}
		else if( checksum != USISRL )
			I2C_State = state_prep_for_start;
		
		data_pos++;
		
		/* Send ACK */
		USISRL = 0x00;
		USICNT |= 0x01;

		/* Receive the next byte */
		I2C_State = state_rx_data;
		break;
	
		/* Prepare for a START bit */
	case state_prep_for_start:
		sda_input();

		I2C_State = state_idle;
		break;
	}

	USICTL1 &= ~USIIFG;
}

