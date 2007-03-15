/****
I2c Section, implements the i2c hardware code and the SMBUS protols
SMBUS protols supported:
Write word protocol
Read word protocol
Block read protocol
**/

#include "hardware.h"
#include "i2c.h"


char i2c_data[32];		// i2c data, array can contain a maximum of 32 values to be sent or read as per SMBUS specification
char SLV_Addr = ADDRESS;		// Address is 0x48, LSB  for R/W
int I2C_State = 0;		//i2c transmition states
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
  USICTL0 = USIPE6|USIPE7|USISWRST;    // Port & USI mode setup
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
    I2C_State = 2;                     // Enter 1st state on start
  }

  switch(I2C_State)
    {
      case 0: // Idle, should not get here
              break;

      case 2: // RX Address
              USICNT = (USICNT & 0xE0) + 0x08; // (Keep previous setting, make sure counter is 0, then add 8)Bit counter = 8, RX address
              USICTL1 &= ~USISTTIFG;   // Clear start flag
              I2C_State = 4;           // Go to next state: check address
              break;

      case 4: // Process Address and send (N)Ack
              if (USISRL & 0x01)       // If read...
                SLV_Addr++;            // Save R/W bit
				USICTL0 |= USIOE;        // SDA = output
              if (USISRL == SLV_Addr)  // Address match?
              {
                USISRL = 0x00;         // Send Ack
                I2C_State = 8;         // Go to next state: RX data
              }
              else
              {
                USISRL = 0xFF;         // Send NAck
                I2C_State = 6;         // Go to next state: prep for next Start
              }
              USICNT |= 0x01;          // Bit counter = 1, send (N)Ack bit
              break;

      case 6: // Prep for Start condition
              USICTL0 &= ~USIOE;       // SDA = input
              SLV_Addr = ADDRESS;         // Reset slave address
              I2C_State = 0;           // Reset state machine
              break;

      case 8: // Receive data byte
              USICTL0 &= ~USIOE;       // SDA = input
              USICNT |=  0x08;         // Bit counter = 8, RX data
              I2C_State = 10;          // Go to next state: Test data and (N)Ack
              break;

    case 10:// Check Data & TX (N)Ack and understand command
        USICTL0 |= USIOE;        // SDA = output
        if (1){  // If data valid...
			I2C_State = smbus_parse(USISRL);  // Prep for Start condition; was state 6 before
            USISRL = 0x00;         // Send Ack
        }else{
            USISRL = 0xFF;         // Send NAck
        }
        USICNT |= 0x01;          // Bit counter = 1, send (N)Ack bit
        break;
    
	case 11: //Write word , prepping to recieve data
		if(new_i2c_data++ < i2c_data_number){ // Receive data byte
			USICTL0 &= ~USIOE;       // SDA = input
			USICNT |=  0x08;         // Bit counter = 8, RX data
			I2C_State = 12;          // Go to next state: Test data and (N)Ack
		}else{ // Prep for Start condition
			i2c_session_complete =1;
			USICTL0 &= ~USIOE;       // SDA = input
            SLV_Addr = ADDRESS;         // Reset slave address
            I2C_State = 0;           // Reset state machine
		}
        break;
		
	case 12: //store data
		USICTL0 |= USIOE;        // SDA = output
		i2c_data[new_i2c_data-1] = USISRL; //store data in aray
        USISRL = 0x00;         // Send Ack
		I2C_State = 11; 	//go back and set up for next bit if there is one
        USICNT |= 0x01;          // Bit counter = 1, send Ack bit
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
			state = 11;
			i2c_data_number = 2; //number of bytes to be recieved
			new_i2c_data = 0;
			i2c_session_complete = 0;
			//get servo position from master
			break;
		case 2:
			//send back block of code with PWM status
			break;
		default:
			state = 6; //unknown data, prep for start 
	}
	return state;
}