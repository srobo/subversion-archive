/* 05/04/07: A Johnson

This firmware enables a MSP430F2012 to control up to 6 independant
servos and recieve or transmit data via i2c.  Servo are controled via
a PWM signal with a pulse width between 1-2ms, but experimentally it
is found that maximum arm rotation (~100degrees) requires pulses
between 0.8-2.3ms. This code assumes that the clock is running at
12Mhz CC1 interrupts at the end of every pulse width (0.8-2.3ms) and
is clocked from timerA.
CC0 is the period of each pulse and interrupts at 20ms.

TODO-update all comments so they reflect what is happening
TODO-change all times so they are not magic values
TODO-Move all of i2c code out of interrupts, change them so they are polled
TODO-Get setServoPWM to return success or failure */

#include "hardware.h"
#include <stdint.h>
#include "i2c.h"
#include <signal.h>
#include "sweep.h"
#include "servo.h"

/* The current servo */
uint8_t current_servo;

/* The i2c handler */
void polled_i2c(void);

/* Set the output to the given value */
inline void set_p1out(uint8_t p1);

/**
Function will initialise the MSP430 for the PWM board. This involves
initialising the main oscillator, settings ports for IO, putting the USI
in I2C mode, etc
**/
void initialise_PwmBoard(void){
	
	/**Initialising MSP430**/
	WDTCTL = WDTCTL_INIT;	//Initializing watchdog timer
	DCOCTL = CALDCO_INIT;	//Initializing clock source
	BCSCTL1 = CALBC1_INIT;

	P1OUT = P1OUT_INIT;		//Init output data of port1&2
	P2OUT  = P2OUT_INIT;

	P1REN |= P1REN_INIT;	//Init something ???

	P1SEL  = P1SEL_INIT;	//Select port or module -function on port1
	P2SEL  = P2SEL_INIT;	//Select port or module -function on port2d

	P1DIR  = P1DIR_INIT;	//Init port direction register of port1
	P2DIR  = P2DIR_INIT;	//Init port direction register of port2

	P1IES  = P1IES_INIT;	//init port interrupts
	P2IES  = P2IES_INIT;
	P1IE   = P1IE_INIT;
	P2IE   = P2IE_INIT;
	P2IFG = 0x00; //clear all flags that might have been set
	
	servo_init();
	
	//TACCR0 interrupts after 20ms
	TACCR0 = PERIOD;
	//TACCR1 interrupts at end of pulse for servo 0
	TACCR1 = getServoPWM(0);

	TACCTL0 = CCIE; //turn on interrupts for ccp module
	TACCTL1 = CCIE; //turn on interrupts for ccp module
	
	//setting timer A to count up to TACCR0, and also turn on interrupts
	TACTL = TASSEL_SMCLK|MC_UPTO_CCR0|ID_DIV2|TAIE;

	current_servo = 0;
	initialise_i2c();
	enable_i2c();
	eint(); //enable interrupts
}

int main(void) {

	initialise_PwmBoard();

	while (1){
		/* Process i2c if either a START, Counter=0 flag is set */
		if( (USICTL1 & USIIFG) || (USICTL1 & USISTTIFG) )
			polled_i2c();

		sweepServo();
	}
}

/**
ISR for IO interrupt
If a negative edge (rail = 0) then set P1 to be all low
**/
interrupt (PORT2_VECTOR) isr_port2(void){
	P1OUT = 0x00;
	P2IFG = 0x00;
}

/**
ISR for TACCR0. Is called at the end of the pulse period ~20ms, 
it resets TIMERA, resets current_servo, and put servo0 pin high
**/
interrupt (TIMERA0_VECTOR) isr_TACR0(void){
	current_servo = 0;
	set_p1out(0xFE);
}

inline void set_p1out(uint8_t p1){
	if(P2IN & RAIL_MONITOR_PIN){ //if rail is high then change pin output
		P1OUT = p1;
	}
}
/**
ISR for TACCR1, TACCR2 (not available on F2012) and overflow. 
It is called at the end of every pulse ~0.8-2.2ms.
Will set the current servo pin low and set the next servo pin high. 
**/
interrupt (TIMERA1_VECTOR) isr_TAIV(void){ //both for period interrupt and
	switch( TAIV ){
	case  2: // CCR1
		current_servo++;
		//checks that there is a next servo!
		if(current_servo<SERVO_NUMBER){
			//get servo# pulse width, increment cc1 by its width
			TACCR1 += getServoPWM(current_servo);
			set_p1out(~(0x1<<current_servo)); //set servo# output high
		}else{
			set_p1out(0xFF);
			TACCR1 = getServoPWM(0); //reset CC1 to pulse Servo0
		}
		break;
	case  4: break;       // CCR2 is not present on the F2013
	case 10: break;       // overflow, will occur when
	}
}

void polled_i2c(void){
	uint8_t number_of_data;
	uint8_t * data;
	isr_usi ();
	if(available_i2c_data())
	{
		number_of_data = available_i2c_data();
		data = get_i2cData();
		setServoPWM(data[0], (MIN_PULSE + 25*(uint16_t)data[1])); //takes values from 0-135
	}	
}

interrupt (NOVECTOR) IntServiceRoutine(void)
{

}



