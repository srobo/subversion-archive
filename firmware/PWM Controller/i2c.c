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
  state_idle = 0, //wait for a start condition
  state_rx_address,
  state_check_address,
  state_rx_command,
  state_check_command,
  state_rx_data,
  state_check_data,
  state_prep_for_start
}state_t;

char i2c_data[32];		// i2c data, array can contain a maximum of 32 values to be sent or read as per SMBUS specification
char SLV_Addr = ADDRESS;		// Address is 0x48, LSB  for R/W << CORECT 0x70??
state_t I2C_State = state_idle;		//i2c transmition states
char i2c_session_complete = 0;		//set when transmission is complete, guess could be replaced by reading one of the states
char new_i2c_data = 0;		// number of data already recieved
char i2c_data_number = 0;	//number of data bytes to be recieved

/**
This function will return the number of available bytes of data. If called before an i2c session is complete it will return 0.
**/
char available_i2c_data(void){
	if(i2c_session_complete){
		return i2c_data_number;
	}else{
		return 0;
	}
}

/**
Returns a pointer to the first byte of data
**/
char * get_i2cData(void){
	new_i2c_data = 0; 
	return i2c_data;
}

void initialise_i2c(void){
	USICTL0 = USICTL0|USISWRST;
  USICTL0 = USIPE6|USIPE7;    // Port & USI mode setup
  USICTL1 = USII2C|USIIE|USISTTIE;     // Enable I2C mode & USI interrupts
  USICKCTL = USICKPL;                  // Setup clock polarity
  USICNT |= USIIFGCC;                  // Disable automatic clear control
}

void enable_i2c(void){
	USICTL0 &= ~USISWRST;                // Enable USI
	USICTL1 &= ~USIIFG;                  // Clear pending flag
}

//******************************************************************************
// USI interrupt service routine
//******************************************************************************
inline void isr_usi (void){
  if (USICTL1 & USISTTIFG)             // Start entry?
  {
    I2C_State = state_rx_address;                     // Enter 1st state on start
  }

  switch(I2C_State){
    case state_idle: // Idle, will only be here after resetting state machine
        break;

	case state_rx_address: // RX Address
		USICNT = (USICNT & 0xE0) + 0x08; //  (Keep previous setting, make sure counter is 0, then add 8)Bit counter = 8, RX address
		USICTL1 &= ~USISTTIFG;   // Clear start flag
		I2C_State = state_check_address;           // Go to next state: check address
		break;

	case state_check_address: // Process Address and send (N)Ack
		if (USISRL & 0x01){       // If read... This is done so that the addresses can be compared directly
			SLV_Addr++;		// Save R/W bit
		}			
		USICTL0 |= USIOE;        // SDA = output
		if (USISRL == SLV_Addr){  // Address match?
			USISRL = 0x00;         // Send Ack
			I2C_State = state_rx_command;         // Go to next state: RX data
		}else{
			USISRL = 0xFF;         // Send NAck
			I2C_State = state_prep_for_start;         // Go to next state: prep for next Start
		}
		USICNT |= 0x01;          //  Bit counter = 1, send (N)Ack bit
		break;

	case state_rx_command: // Rstepeceive data byte 1
		USICTL0 &= ~USIOE;       // SDA = input
		USICNT |=  0x08;         // Bit counter = 8, RX data
		I2C_State = state_check_command;          // Go to next state: Test data and (N)Ack
		break;

	case state_check_command:// Check Data & TX (N)Ack and understand command
		USICTL0 |= USIOE;        // SDA = output
		if (1){  // If data valid...
			I2C_State = smbus_parse(USISRL);  // Prep for Start condition; was state 6 before
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
		case 0:
			//send identifier back to master
			break;
		case 1:
			state = state_rx_data;
			i2c_data_number = 2; //number of bytes to be recieved
			new_i2c_data = 0;
			i2c_session_complete = 0;
			//get servo position from master
			break;
		case 2:
			//send back block of code with PWM status
			break;
		default:
			state = state_prep_for_start; //unknown data, prep for start 
	}
	return state;
}