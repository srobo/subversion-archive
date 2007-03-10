/*
This is the PWM firmware for 6 PWM outputs!
Its assumed atm that the clock is running at abpout 6.52Mhz
CC1 interrupts every 163 ticks of the timerA
CC0 is the upper bound of timerA and is at 65200
*/
#include "hardware.h"

/**
Delay function.
*/
void delay(unsigned int d) {
   int i;
   for (i = 0; i<d; i++) {
      nop();
      nop();
   }
}

/**

*/
#define SERVO_NUMBER 6

volatile char new_period;
volatile unsigned int servo_pulse[SERVO_NUMBER];
unsigned char current_servo;

void initialiseServoArray(unsigned int pulse_width);
int getServoPWM(unsigned char servo);
void setServoPWM(unsigned char servo, unsigned int pulse_width);


void initialiseServoArray(unsigned int pulse_width){
	unsigned char i;
	for(i=0; i<SERVO_NUMBER; i++){
		setServoPWM(i, pulse_width);
	}
}

int getServoPWM(unsigned char servo){
	return servo_pulse[servo];
}

void setServoPWM(unsigned char servo, unsigned int pulse_width){
	servo_pulse[servo] = pulse_width;
} 

int main(void) {
	int static unused_initialized_variable_to_make_gdb_happy = 1;
	volatile int dosomething;
   WDTCTL = WDTCTL_INIT;               //Init watchdog timer

    P1OUT  = P1OUT_INIT;                //Init output data of port1
    P2OUT  = P2OUT_INIT;                //Init output data of port2

    P1SEL  = P1SEL_INIT;                //Select port or module -function on port1
    P2SEL  = P2SEL_INIT;                //Select port or module -function on port2

    P1DIR  = P1DIR_INIT;                //Init port direction register of port1
    P2DIR  = P2DIR_INIT;                //Init port direction register of port2

    P1IES  = P1IES_INIT;                //init port interrupts
    P2IES  = P2IES_INIT;
    P1IE   = P1IE_INIT;
    P2IE   = P2IE_INIT;
	
	TACTL = TASSEL_SMCLK|MC_UPTO_CCR0|ID_DIV8|TAIE; //settign timer A to count up to Taccr0, and also turn on interrupts
	
	initialiseServoArray(4890); //163*30 ~ 1.5ms pulse
	
	TACCR0 = 65200; //period for pulses
	TACCR1 = getServoPWM(0); //pulsewidth for servo0
	
	TACCTL0 = CCIE; //turn on interrupts for ccp module
	TACCTL1 = CCIE; //turn on interrupts for ccp module
	
	new_period = 1; //when the system starts its a new 
	current_servo = 0;
	dosomething = 0;
	eint();
	
    while (1) {   
		dosomething++;
	}
}

interrupt (TIMERA0_VECTOR) isr_TACR0(void){ //isr for period interrupt
	current_servo = 0;
	P1OUT = (0x01);
}

interrupt (TIMERA1_VECTOR) isr_TAIV(void){ //both for period interrupt and 
	switch( TAIV ){
		case  2: // CCR1
			if(++current_servo<SERVO_NUMBER){ //deal with next servo and check there is a next servo!
				TACCR1 += getServoPWM(current_servo); //get servo# pulse width, increment cc1 by its width
				P1OUT = (0x01<<current_servo); //set servo# output high
			}else{
				TACCR1 = getServoPWM(0); //reset CC1 to pulse Servo0
			}
			break;                          
		case  4: break;       // CCR2 is not present on the F2013
		case 10: break;       // overflow, will occur when 
            
 }
}

