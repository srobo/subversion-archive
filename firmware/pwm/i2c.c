/****
I2c Section, implements the i2c hardware code and the SMBUS protols
SMBUS protols supported:
Write word protocol
Read word protocol
Block read protocol
**/
#include "hardware.h"
#include "i2c.h"

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

/* Buffer for I2C data */
char i2c_data[32];

/* The I2C transmission state machine state */
state_t I2C_State = state_idle;

/* Whether a transmission is complete */
char i2c_session_complete = 0;

/* The number of bytes that have currently been received */
char new_i2c_data = 0;

/* The number of bytes to be received */
char i2c_data_number = 0;

/*** Device Macros ***/
/* Set SDA to an output */
#define sda_output() do { USICTL0 |= USIOE; } while (0)
/* Set SDA to an input */
#define sda_input() do { USICTL0 &= ~USIOE; } while (0)

/* Process an I2C command.
 * Returns the new state for the I2C state machine. */
state_t smbus_parse(char command);

char available_i2c_data(void)
{
	if(i2c_session_complete)
		return i2c_data_number;

	return 0;
}

char * get_i2cData(void)
{
	new_i2c_data = 0; 

	return i2c_data;
}

void initialise_i2c(void)
{
	USICTL0 = USICTL0|USISWRST;
	USICTL0 = USIPE6|USIPE7;// Port & USI mode setup
//  USICTL1 = USII2C|USIIE|USISTTIE;     // Enable I2C mode & USI interrupts
	USICTL1 = USII2C;		// Enable I2C mode
	USICKCTL = USICKPL;		// Setup clock polarity
	USICNT |= USIIFGCC;		// Disable automatic clear control
}

void enable_i2c(void)
{
	USICTL0 &= ~USISWRST;                // Enable USI
	USICTL1 &= ~USIIFG;                  // Clear pending flag
}

inline void isr_usi (void)
{
	/* START Received */
	if (USICTL1 & USISTTIFG)
		/* Move into the state machine */
		I2C_State = state_rx_address;

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
		I2C_State = smbus_parse(USISRL);

		/* Send ACK */
		USISRL = 0x00;
		USICNT |= 0x01;
		break;

		/* Receive data */
	case state_rx_data:

		if( new_i2c_data++ < i2c_data_number )
		{ 
			/* More data to receive */
			sda_input();

			USICNT |=  0x08;

			I2C_State = state_check_data;
		}
		else
		{
			/* All data received */
			i2c_session_complete = 1;

			/* START condition will happen next */
			sda_input();
			I2C_State = state_idle;
		}
		break;
		
		/* Store the received data */
	case state_check_data:
		sda_output();

		i2c_data[new_i2c_data-1] = USISRL;

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

state_t smbus_parse(char command)
{
	char state = state_check_data;

	switch(command){
		/* Send identifier to master */
		case COMMAND_IDENTIFY:
			/* Not implemented */
			break;

			/* Get the position of a servo from the master */
		case COMMAND_SET:
			/* 2 bytes to be received */
			i2c_data_number = 2;
			new_i2c_data = 0;
			i2c_session_complete = 0;

			state = state_rx_data;
			break;

			/* Send servo states to master */
		case COMMAND_READ:
			/* Not implemented */
			break;

		default:
			/* Unknown command - go back to the start */
			state = state_prep_for_start;
	}
	return state;
}
