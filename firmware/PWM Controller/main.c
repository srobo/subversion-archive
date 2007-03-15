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
uint16_t servo_pulse[SERVO_NUMBER];
unsigned char current_servo;


/**
Delay function, will run a while loop corresponding to 1ms when FOSC is 12Mhz
@param uint32 number of ms the delay must last
TODO possibly investigate a bug that increments the duration of the delay  inexplicably when a value greater than 30000 is given to i.
*/
void delay_MS(uint32_t d) {
	uint16_t i;
	while(d){
		d--;
		i = 15000;
		while(i>0){	i--;}
		i = 20000;
		while(i>0){	i--;}
	}   
} 

/**
Function will initialise the array containing the pulsewidth of the servos to the value given in the parameter
@param uint16 default value to initialise the servo with
**/
void initialiseServoArray(uint16_t pulse_width){
	unsigned char i;
	for(i=0; i<SERVO_NUMBER; i++){
		setServoPWM(i, pulse_width);
	}
}
/**
This function will return the current pulsewidth of the servo passed as an parameter.
@param uint8 servo number 
@return uint16 pulsewidth of the servo given in the parameter
**/
uint16_t getServoPWM(unsigned char servo){
	return servo_pulse[servo];
}

/**
Function will set the pulsewdith of the servo given in the parameter.
@param uint8 servo number
@param uint16 pulsewidth of the servo , when FOSC 12Mhz then range is between 2260 (0.8ms) to 6215(2.2ms)
**/
void setServoPWM(unsigned char servo, uint16_t pulse_width){
	servo_pulse[servo] = pulse_width;
} 
 
/**
Function will initialise the MSP430 for the PWM board. This involves initialising ports for IO and I2C.
**/ 
void initialise_PwmBoard(void){
	WDTCTL = WDTCTL_INIT;               //Initializing watchdog timer
	DCOCTL = CALDCO_12MHZ_;				//Initializing FOSC
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
		
	initialiseServoArray(MIDDLE_PULSE); //1.5ms pulse
	
	TACCR0 = PERIOD; //period for pulses
	TACCR1 = getServoPWM(0); //pulsewidth for servo0
	
	TACCTL0 = CCIE; //turn on interrupts for ccp module
	TACCTL1 = CCIE; //turn on interrupts for ccp module
	TACTL = TASSEL_SMCLK|MC_UPTO_CCR0|ID_DIV2|TAIE; //settign timer A to count up to Taccr0, and also turn on interrupts
	
	current_servo = 0;
	initialise_i2c();
	enable_i2c();
	eint(); 
	
}

 /**
 Main function
 **/
int main(void) {
	int static unused_initialized_variable_to_make_gdb_happy = 1;
	initialise_PwmBoard();
    while (1) {   
	//	delay_MS(1000);
	//	setServoPWM(0, 2.2*TICKS_PER_MS);
	//	delay_MS(1000);
	//	setServoPWM(0, 0.8*TICKS_PER_MS );
	}
}

interrupt (TIMERA0_VECTOR) isr_TACR0(void){ //isr for period interrupt, resets TIMERA also resets current_servo, and put servo0 pin high
	current_servo = 0;
	P1OUT = (0xFE);
}

interrupt (TIMERA1_VECTOR) isr_TAIV(void){ //both for period interrupt and 
	switch( TAIV ){
		case  2: // CCR1
			current_servo++;
			if(current_servo<SERVO_NUMBER){ //deal with next servo and check there is a next servo!
				TACCR1 += getServoPWM(current_servo); //get servo# pulse width, increment cc1 by its width
				P1OUT = (0xFE<<current_servo); //set servo# output high
			}else{
				P1OUT = (0xFF);
				TACCR1 = getServoPWM(0); //reset CC1 to pulse Servo0
			}
			break;                          
		case  4: break;       // CCR2 is not present on the F2013
		case 10: break;       // overflow, will occur when 
            
 }
}

interrupt (USI_VECTOR) isr_USI(void){ //isr for period interrupt, resets TIMERA also resets current_servo, and put servo0 pin high
	char number_of_data;
	char * data;
	isr_usi ();
	if(available_i2c_data()){
		number_of_data = available_i2c_data();
		data = get_i2cData();
		setServoPWM(data[number_of_data-2], (MIN_PULSE + 25*data[number_of_data-1])); //takes values from 0-135
	}
}
 

