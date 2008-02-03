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
TODO-Get servo_set_pwm to return success or failure */

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

/* Initialise everything. */
void init(void);

int main(void)
{
	init();

	while (1)
	{
		/* Process i2c if either a START, Counter=0 flag is set */
		if( (USICTL1 & USIIFG) || (USICTL1 & USISTTIFG) )
			polled_i2c();

		sweepServo();
	}
}

void init(void)
{
	/* Disable the watchdog timer */
	WDTCTL = WDTPW | WDTHOLD;

	/* Configure DCOCTL to be 12 MHz */
	DCOCTL = CALDCO_12MHZ;
	BCSCTL1 = CALBC1_12MHZ;

	P1OUT = 0xC0;
	P2OUT = 0x00;

	/* Pull-ups */
	P1REN |= 0;

	/* P1.6 and P1.7 are I2C */
	P1SEL = 0xC0;
	/* All gpio on port 2 */
	P2SEL = 0;

	P1DIR  = 0x3f;
	P2DIR  = 0x7f;

	P1IES  = 0;
	P2IES  = RAIL_MONITOR_PIN;
	P1IE   = 0;
	P2IE   = RAIL_MONITOR_PIN;
	P2IFG = 0x00; //clear all flags that might have been set
	
	servo_init();
	
	//TACCR0 interrupts after 20ms
	TACCR0 = PERIOD;
	//TACCR1 interrupts at end of pulse for servo 0
	TACCR1 = servo_get_pwm(0);

	TACCTL0 = CCIE; //turn on interrupts for ccp module
	TACCTL1 = CCIE; //turn on interrupts for ccp module
	
	//setting timer A to count up to TACCR0, and also turn on interrupts
	TACTL = TASSEL_SMCLK	/* SMCLK clock source */
		| MC_UPTO_CCR0	/* Count up to the value in CCR0 */
		| ID_DIV4	/* Divide the frequency by 4 */
		| TAIE;		/*  */

	current_servo = 0;
	initialise_i2c();
	enable_i2c();
	eint(); //enable interrupts
}

/* ISR for IO interrupt */
interrupt (PORT2_VECTOR) isr_port2(void)
{
	/* The rail has dropped - set the outputs to be low. */
	P1OUT = 0x00;
	P2IFG = 0x00;
}

/* ISR for TACCR0. 
 * Called at the end of the pulse period ~20ms,
 * it resets TIMERA, resets current_servo, and put servo0 pin high. */
interrupt (TIMERA0_VECTOR) isr_TACR0(void)
{
	current_servo = 0;
	set_p1out(0xFE);
}

inline void set_p1out(uint8_t p1)
{
	/* Only change the output if the rail is up */
	if(P2IN & RAIL_MONITOR_PIN)
		P1OUT = p1;
}

/* ISR for TACCR1, TACCR2 (not available on F2012) and overflow. 
 * It is called at the end of every pulse ~0.8-2.2ms.
 * Will set the current servo pin low and set the next servo pin high. */
interrupt (TIMERA1_VECTOR) isr_TAIV(void)
{ 
	switch( TAIV )
	{
	case  2: // CCR1
		current_servo++;
		//checks that there is a next servo!
		if(current_servo<SERVO_NUMBER)
		{
			//get servo# pulse width, increment cc1 by its width
			TACCR1 += servo_get_pwm(current_servo);
			set_p1out(~(0x1<<current_servo)); //set servo# output high
		} 
		else
		{
			set_p1out(0xFF);
			TACCR1 = servo_get_pwm(0); //reset CC1 to pulse Servo0
		}
		break;

	case  4: break;       // CCR2 is not present on the F2013
	case 10: break;       // overflow, will occur when
	}
}

void polled_i2c(void)
{
	uint8_t number_of_data;
	uint8_t * data;
	isr_usi ();
	if(available_i2c_data())
	{
		number_of_data = available_i2c_data();
		data = get_i2cData();
		servo_set_pwm( data[0],
			       (MIN_PULSE + ((MAX_PULSE-MIN_PULSE)/255)*(uint16_t)data[1]));
	}	
}

interrupt (NOVECTOR) IntServiceRoutine(void)
{

}



