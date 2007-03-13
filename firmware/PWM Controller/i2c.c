/****
I2c Section
**/

#include "hardware.h"
#include "i2c.h"

#define ADDRESS 0x1E

char MST_Data = 0;                     // Variable for received data
char SLV_Addr = ADDRESS;                  // Address is 0x48<<1 for R/W
int I2C_State = 0;   
char new_i2c_data = 0;                  // State variable

char available_i2c_data(void){
	return new_i2c_data;
}

char get_i2cData(void){
	new_i2c_data = 0; 
	return MST_Data;
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
//#pragma tell function to be inline
void isr_usi (void){
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

      case 10:// Check Data & TX (N)Ack
              USICTL0 |= USIOE;        // SDA = output
              if (1)  // If data valid...
              {
				MST_Data = USISRL;
				new_i2c_data = 1;
                USISRL = 0x00;         // Send Ack
              }
              else
              {
                USISRL = 0xFF;         // Send NAck
              }
              USICNT |= 0x01;          // Bit counter = 1, send (N)Ack bit
              I2C_State = 6;           // Go to next state: prep for next Start
              break;
    }

  USICTL1 &= ~USIIFG;                  // Clear pending flags
}