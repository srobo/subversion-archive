/****
I2c Section, implements the i2c hardware code and the SMBUS protols
SMBUS protols supported:
Write word protocol
Read word protocol
Block read protocol
**/
#include "hardware.h"
#include "i2c.h"

/* Process an I2C command */
char smbus_parse(char command);

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
char SLV_Addr = ADDRESS;

/* The I2C transmission state machine state */
state_t I2C_State = state_idle;

/* Whether a transmission is complete */
char i2c_session_complete = 0;

/* The number of bytes that have currently been received */
char new_i2c_data = 0;

/* The number of bytes to be received */
char i2c_data_number = 0;

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

	case state_rx_address: // RX Address
		USICTL0 &= ~USIOE;					// SDA = input
		USICNT = (USICNT & 0xE0) + 0x08; //  (Keep previous setting, make sure counter is 0, then add 8)Bit counter = 8, RX address
		USICTL1 &= ~USISTTIFG;   // Clear start flag
		I2C_State = state_check_address;           // Go to next state: check address
		break;

	case state_check_address: // Process Address and send Ack
		if (USISRL & 0x01){	// If read... This is done so that the addresses can be compared directly
			SLV_Addr++;		// Save R/W bit
		}	
		if (USISRL == SLV_Addr){	// Address match?
			USICTL0 |= USIOE;		// SDA = output ??? can it switch so fast
			USISRL = 0x00;			// Send Ack
			I2C_State = state_rx_command;	// Go to next state: RX data
			USICNT |= 0x01;			//  Bit counter = 1, send Ack bit
		}else{ //Not correct address, reset to idle
			SLV_Addr = ADDRESS;         // Reset slave address
			I2C_State = state_idle;     // Reset state machine
		}
		break;

	case state_rx_command: // prep to Receive data byte 1
		USICTL0 &= ~USIOE;	// SDA = input
		USICNT |=  0x08;	// Bit counter = 8, RX data
		I2C_State = state_check_command;// Go to next state: Test data
		break;

	case state_check_command:// Check Data & TX (N)Ack and understand command
		USICTL0 |= USIOE;        // SDA = output
		if (1){  // If data valid... ALWAYS VALID
			I2C_State = smbus_parse(USISRL);  // Prep for Start condition;
			USISRL = 0x00;         // Send Ack
		}else{
			USISRL = 0xFF;         // Send NAck
		}
		USICNT |= 0x01;          // Bit counter = 1, send (N)Ack bit
		break;

	case state_rx_data: //Write word , prepping to recieve data
		if(new_i2c_data++ < i2c_data_number){ // Receive data byte
			USICTL0 &= ~USIOE;       // SDA = input
			USICNT |=  0x08;         // Bit counter = 8, RX data
			I2C_State = state_check_data;          // Go to next state: Test data and (N)Ack
		}else{ // Prep for Start condition
			i2c_session_complete =1;
			USICTL0 &= ~USIOE;       // SDA = input
			SLV_Addr = ADDRESS;         // Reset slave address
			I2C_State = state_idle;           // Reset state machine
		}
		break;
		
	case state_check_data: //store data
		USICTL0 |= USIOE;        // SDA = output
		i2c_data[new_i2c_data-1] = USISRL; //store data in aray
		USISRL = 0x00;         // Send Ack
		I2C_State = state_rx_data; 	//go back and set up for next bit if there is one
		USICNT |= 0x01;          // Bit counter = 1, send Ack bit
		break;
	
	case state_prep_for_start: // Prep for Start condition
		USICTL0 &= ~USIOE;       // SDA = input
		SLV_Addr = ADDRESS;         // Reset slave address
		I2C_State = state_idle;           // Reset state machine
		break;
	}
	USICTL1 &= ~USIIFG;                  // Clear pending flags
}

char smbus_parse(char command){
	char state =6;
	switch(command){
		case COMMAND_IDENTIFY:
			//send identifier back to master
			break;
		case COMMAND_SET:
			state = state_rx_data;
			i2c_data_number = 2; //number of bytes to be recieved
			new_i2c_data = 0;
			i2c_session_complete = 0;
			//get servo position from master
			break;
		case COMMAND_READ:
			//send back block of code with PWM status
			break;
		default:
			state = state_prep_for_start; //unknown data, prep for start 
	}
	return state;
}
