/*
This is the PWM firmware for 6 PWM outputs!
Its assumed atm that the clock is running at abpout 6.52Mhz
CC1 interrupts every 163 ticks of the timerA
CC0 is the upper bound of timerA and is at 65200
*/
#include "hardware.h"
#include "stdint.h"
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



char new_period;
unsigned int servo_pulse[SERVO_NUMBER];
unsigned char current_servo;

void initialiseServoArray(unsigned int pulse_width);
unsigned int getServoPWM(unsigned char servo);
void setServoPWM(unsigned char servo, unsigned int pulse_width);


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
    
	P1OUT  = P1OUT_INIT;                //Init output data of port1
    P2OUT  = P2OUT_INIT;                //Init output data of port2

    P1SEL  = P1SEL_INIT;                //Select port or module -function on port1
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
	
	TACCTL0 = CCIE; //turn on interrupts for ccp module
	TACCTL1 = CCIE; //turn on interrupts for ccp module
	TACTL = TASSEL_SMCLK|MC_UPTO_CCR0|ID_DIV2|TAIE; //settign timer A to count up to Taccr0, and also turn on interrupts
	
	current_servo = 0;
	eint(); 
	
    while (1) {   
		delay_MS(1000);
		setServoPWM(0, 2.2*TICKS_PER_MS);
		delay_MS(1000);
		setServoPWM(0, 0.8*TICKS_PER_MS );
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

