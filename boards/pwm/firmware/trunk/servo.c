#include "servo.h"
#include "common.h"
#include <signal.h>
#include "watchdog.h"
#include "railmon.h"

uint16_t servo_pulse[SERVO_NUMBER];

/* The current_servo */
uint8_t current_servo;

/* Set the output to the given value */
inline void set_p1out(uint8_t p1);

void servo_init( void )
{
	uint8_t i;

	for(i=0; i<SERVO_NUMBER; i++)
		servo_set_pwm(i, MIDDLE_PULSE);

	/* Outputs */
	P4DIR |= 0x3F;

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
}

uint16_t servo_get_pwm(uint8_t servo)
{
	return servo_pulse[servo];
}

void servo_set_pwm(uint8_t servo, uint16_t pulse_width)
{
	if(servo < SERVO_NUMBER)
	{
		if(pulse_width >= MAX_PULSE)
			pulse_width = MAX_PULSE;
		else if(pulse_width <= MIN_PULSE)
			pulse_width = MIN_PULSE;

		servo_pulse[servo] = pulse_width;
	}
}

/* ISR for TACCR0. 
 * Called at the end of the pulse period ~20ms,
 * it resets TIMERA, resets current_servo, and put servo0 pin high. */
interrupt (TIMERA0_VECTOR) isr_TACR0(void)
{
//	i2c_watchdog_check();
	watchdog_clear();

	current_servo = 0;
	set_p1out(0xFE);
}

inline void set_p1out(uint8_t p1)
{
	/* Only change the output if the rail is up */
	if( railmon_is_up() )
		P4OUT = p1;
}

/* ISR for TACCR1, TACCR2 (not available on F2012) and overflow. 
 * It is called at the end of every pulse ~0.8-2.2ms.
 * Will set the current servo pin low and set the next servo pin high. */
interrupt (TIMERA1_VECTOR) isr_TAIV(void)
{ 
	watchdog_clear();

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
