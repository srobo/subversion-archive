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
  state_prepfor_rx_address,
  state_check_rx_address,
  state_prepfor_rx_command,
  state_check_rx_command,
  state_prepfor_rx_data,
  state_check_rx_data,
  state_prepfor_rx_start,
  state_send_tx_data,
  state_checkack_tx_data
}state_t;

uint8_t i2c_data[32];		// i2c data, array can contain a maximum of 32 values to be sent or read as per SMBUS specification
uint8_t SLV_Addr = ADDRESS;		// Address is 0x48, LSB  for R/W << CORECT 0x70??
state_t I2C_State = state_idle;		//i2c transmition states
uint8_t i2c_session_complete = 0;		//set when transmission is complete, guess could be replaced by reading one of the states
uint8_t new_i2c_data = 0;		// number of data already recieved
uint8_t i2c_data_number = 0;	//number of data bytes to be recieved

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
	USICTL0 = USIPE6|USIPE7;// Port & USI mode setup
//  USICTL1 = USII2C|USIIE|USISTTIE;     // Enable I2C mode & USI interrupts
	USICTL1 = USII2C;		// Enable I2C mode
	USICKCTL = USICKPL;		// Setup clock polarity
	USICNT |= USIIFGCC;		// Disable automatic clear control
}

void enable_i2c(void){
	USICTL0 &= ~USISWRST;                // Enable USI
	USICTL1 &= ~USIIFG;                  // Clear pending flag
}

/**
 The SMBus parts ,
 From: http://www.kernel.org/pub/linux/kernel/v2.6/linux-2.6.1.tar.bz2/linux-2.6.1/drivers/i2c/
**/
#define POLY    (0x1070U << 3)
uint8_t PEC;
static uint8_t crc8(uint16_t data){
	uint16_t i;
	for(i = 0; i < 8; i++) {
		if (data & 0x8000)
			data = data ^ POLY;
		data = data << 1;
	}
	return (uint8_t)(data >> 8);
}

/**
@param crc: The crc calculated up to now
@param count: The # of entries in the array message
@param *message: The array containing the message to checksum
**/
uint8_t i2c_smbus_pec(uint8_t crc, uint16_t count, uint8_t *message){
	uint16_t i;
	for(i = 0; i < count; i++)				//checksum untill all bytes have been processed
		crc = crc8((crc ^ message[i]) << 8);
	return crc;
}


//******************************************************************************
// I2C State Machine
//******************************************************************************
inline void isr_usi (void){
  if (USICTL1 & USISTTIFG)             // Start entry?
  {
    I2C_State = state_prepfor_rx_address;
  }

  switch(I2C_State){
    case state_idle: // Idle, will only be here after resetting state machine
        break;

	/****************************************************************************
	Recieve address and check if it
	******************************************************************************/
	case state_prepfor_rx_address:
		USICNT = (USICNT & 0xE0) + 0x08;	//  (Keep previous setting, make sure counter is 0, then prep for 8bit address
		USICTL1 &= ~USISTTIFG;				// Clear start flag
		I2C_State = state_check_rx_address;	// Go to next state: check address
		break;

	case state_check_rx_address:
		if (USISRL & 0x01){					// Master is expecting to read data
			SLV_Addr++;						// Save R/W bit
		}	
		if (USISRL == SLV_Addr){			// Address match
			if(USISRL & 0x01){				//Master is expecting to read data
				I2C_State = state_send_tx_data;
			}else{							//Master is sending data
				I2C_State = state_prepfor_rx_command;
			}
			PEC = i2c_smbus_pec(PEC, 1, &USISRL); /**Is this the correct way of making a value a pointer?**/
			USICTL0 |= USIOE;				// SDA = output
			USISRL = 0x00;					// Send Ack
			USICNT |= 0x01;					//  Bit counter = 1, send Ack bit
		}else{								//Not correct address, reset to idle
			SLV_Addr = ADDRESS;				// Reset slave address
			I2C_State = state_idle;			// Reset state machine
		}
		break;
		
	/****************************************************************************
	Recieve smbus command and see if a corresponding command exist
	******************************************************************************/
	case state_prepfor_rx_command:
		USICTL0 &= ~USIOE;					// SDA = input
		USICNT |= 0x08;						// Bit counter = 8, RX data
		I2C_State = state_check_rx_command;	// Go to next state: Test data
		break;

	case state_check_rx_command:
		USICTL0 |= USIOE;					// SDA = output
		if (1){								/** If data valid... ALWAYS VALID**/
			I2C_State = smbus_parse(USISRL);//parse command
			PEC = i2c_smbus_pec(PEC, 1, &USISRL); /**Is this the correct way of making a value a pointer?**/
			USISRL = 0x00;					// Send ACK
		}else{
			USISRL = 0xFF;					// Send NACK
		}
		USICNT |= 0x01;						// Bit counter = 1, send (N)ACK
		break;
		
	/****************************************************************************
	Recieve data and store it in the buffer array
	******************************************************************************/
	case state_prepfor_rx_data:
			USICTL0 &= ~USIOE;				// SDA = input
			USICNT |=  0x08;				// Bit counter = 8, RX
			I2C_State = state_check_rx_data;
		break;
		
	case state_check_rx_data:				//check & store data
		USICTL0 |= USIOE;					// SDA = output
		i2c_data[(uint8_t)new_i2c_data++] = USISRL;	//store data in next index
		if(new_i2c_data == i2c_data_number){//last byte is PEC
			if(PEC == i2c_data[(uint8_t)i2c_data_number]){ //compare PEC with the PEC sent by the master
				USISRL = 0x00;				// Send Ack
				i2c_session_complete =1;
			}else{
				USISRL = 0xFF;				// Send NACK
			}
			I2C_State = state_prepfor_rx_start;
		}else{
			PEC = i2c_smbus_pec(PEC, 1, &i2c_data[(uint8_t)new_i2c_data]); //process the last message into the checksum
			I2C_State = state_prepfor_rx_data;//go back and set up for next byte
		}
		USICNT |= 0x01;						// Bit counter = 1, send Ack bit
		break;
		
	/****************************************************************************
	Send data and check for (N)ACK
	******************************************************************************/
	case state_send_tx_data:
		USICTL0 |= USIOE;        // SDA = output
		USISRL = i2c_data[new_i2c_data];       // Send data
		USICNT |=  0x08;         // Bit counter = 8, TX data
		I2C_State = state_prepforack_tx_data;// Go to next state: receive (N)Ack
		break;
		
	case state_prepforack_tx_data:
		USICTL0 &= ~USIOE;       // SDA = input
		USICNT |= 0x01;          // Bit counter = 1, receive (N)Ack
		I2C_State = state_checkack_tx_data;          // Go to next state: check (N)Ack
		break;
		
	case state_checkack_tx_data:// Process Data Ack/NAck
		if (USISRL & 0x01){       // If Nack received...
			I2C_State = state_prepfor_rx_start;
		}else{                    // Ack received
			new_i2c_data++;         // Increment Slave data
			I2C_State = state_send_tx_data;
		}
		break;
	/****************************************************************************
	Reset the statemachine
	******************************************************************************/
	case state_prepfor_rx_start:
		USICTL0 &= ~USIOE;					// SDA = input
		SLV_Addr = ADDRESS;					// Reset slave address
		I2C_State = state_idle;				// Reset state machine
		PEC = 0;							//Reset PEC
		break;
	}
	
	/****************************************************************************
	Exit state machine
	******************************************************************************/
	USICTL1 &= ~USIIFG;						// Clear pending flags
}

/**
Parse command and setup i2c statemachine appropriatly
**/
char smbus_parse(char command){
	char state =6;
	switch(command){
		case 0:
			//send identifier back to master
			i2c_data[0]=(IDENTIFIER & 0xFF);
			i2c_data[1]=(IDENTIFIER>>8) & 0xFF);
			i2c_data[2]= PEC
			break;
		case 1: //get servo position from master
			state = state_prepfor_rx_data;
			i2c_data_number = 3; //number of bytes to be recieved
			new_i2c_data = 0;
			i2c_session_complete = 0;
			break;
		case 2:
			//send back block of code with PWM status
			break;
		default:
			state = state_prepfor_rx_start; //unknown data, prep for start 
	}
	return state;
}
