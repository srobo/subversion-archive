/*
This is the PWM firmware for 6 PWM outputs!
Its assumed atm that the clock is running at abpout 6.52Mhz
CC1 interrupts every 163 ticks of the timerA
CC0 is the upper bound of timerA and is at 65200
*/
#define __MSP430_2003__

#include "hardware.h"
#include <stdint.h>
#include "i2c.h"

char new_period;
unsigned int servo_pulse[SERVO_NUMBER];
unsigned char current_servo;
volatile char test_value = 2.2;


/**
Delay function.
*/
void delay_MS(uint32_t d) {
	unsigned int i;
	while(d){
		d--;
		i = 15000;
		while(i>0){	i--;}
		i = 20000;
		while(i>0){	i--;}
	}   
} 

void initialiseServoArray(unsigned int pulse_width){
	unsigned char i;
	for(i=0; i<SERVO_NUMBER; i++){
		setServoPWM(i, pulse_width);
	}
}

unsigned int getServoPWM(unsigned char servo){
	return servo_pulse[servo];
}

void setServoPWM(unsigned char servo, unsigned int pulse_width){
	servo_pulse[servo] = pulse_width;
} 
 
int main(void) {
	int static unused_initialized_variable_to_make_gdb_happy = 1;
	WDTCTL = WDTCTL_INIT;               //Init watchdog timer
	
	DCOCTL = CALDCO_12MHZ_;
	BCSCTL1 = CALBC1_12MHZ_; 
	
	
    
	P1OUT = 0xC0;                        // P1.6 & P1.7 Pullups
    P2OUT  = P2OUT_INIT;                //Init output data of port2
	
	P1REN |= 0xC0;                       // P1.6 & P1.7 Pullups
	
    P1SEL  = 0xC0;                //Select port or module -function on port1
    P2SEL  = P2SEL_INIT;                //Select port or module -function on port2d 

    P1DIR  = P1DIR_INIT;                //Init port direction register of port1
    P2DIR  = P2DIR_INIT;                //Init port direction register of port2

    P1IES  = P1IES_INIT;                //init port interrupts
    P2IES  = P2IES_INIT;
    P1IE   = P1IE_INIT;
    P2IE   = P2IE_INIT;
	
	initialiseServoArray(1.5*TICKS_PER_MS); //1.5ms pulse
	
	TACCR0 = PERIOD; //period for pulses
	TACCR1 = getServoPWM(0); //pulsewidth for servo0
	
	//TACCTL0 = CCIE; //turn on interrupts for ccp module
	//TACCTL1 = CCIE; //turn on interrupts for ccp module
	//TACTL = TASSEL_SMCLK|MC_UPTO_CCR0|ID_DIV2|TAIE; //settign timer A to count up to Taccr0, and also turn on interrupts
	
	current_servo = 0;
	initialise_i2c();
	enable_i2c();
	eint(); 
	
    while (1) {   
	//	delay_MS(1000);
	//	setServoPWM(0, test_value*TICKS_PER_MS);
	//	delay_MS(1000);
	//	setServoPWM(0, 0.8*TICKS_PER_MS );
	P1OUT = (test_value);
	}
}

interrupt (TIMERA0_VECTOR) isr_TACR0(void){ //isr for period interrupt, resets TIMERA also resets current_servo, and put servo0 pin high
	current_servo = 0;
//	P1OUT = (0xFE);
}

interrupt (TIMERA1_VECTOR) isr_TAIV(void){ //both for period interrupt and 
	switch( TAIV ){
		case  2: // CCR1
			current_servo++;
			if(current_servo<SERVO_NUMBER){ //deal with next servo and check there is a next servo!
				TACCR1 += getServoPWM(current_servo); //get servo# pulse width, increment cc1 by its width
		//		P1OUT = (0xFE<<current_servo); //set servo# output high
			}else{
		//		P1OUT = (0xFF);
				TACCR1 = getServoPWM(0); //reset CC1 to pulse Servo0
			}
			break;                          
		case  4: break;       // CCR2 is not present on the F2013
		case 10: break;       // overflow, will occur when 
            
 }
}
char tmp;
interrupt (USI_VECTOR) isr_USI(void){ //isr for period interrupt, resets TIMERA also resets current_servo, and put servo0 pin high
	isr_usi ();
	if(available_i2c_data()){
		tmp = get_i2cData();
		if(tmp == 5){
			test_value = 1;
		}else if(tmp == 55){
			test_value = 0;
		}
	}
}


/****
I2c Section
**/

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

