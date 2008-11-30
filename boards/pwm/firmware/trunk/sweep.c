#include "sweep.h"
#include <stdint.h>
#include "hardware.h"
#include "servo.h"

/**
Delay function, will run a while loop corresponding to 1ms when FOSC is 12Mhz
@param uint32 number of ms the delay must last
TODO -  possibly investigate a bug that increments the duration of the delay
inexplicably when a value greater than 30000 is given to i.
*/
void delay_MS(uint32_t d)
{
	volatile uint16_t i;

	while(d--)
	{
		i = 15000;
		while(i>0) 
			i--;

		i = 20000;
		while(i>0)
			i--;
	}
}

/* This function sweeps all servos between two hardcoded arbitrary positions */
#if(1)
void sweepServo(void)
{
	uint8_t channel;

	delay_MS(50);
	for( channel=0; channel < SERVO_NUMBER; channel++ )
		servo_set_pwm(channel, (uint16_t)(2.3*TICKS_PER_MS));

	delay_MS(50);
	for( channel=0; channel < SERVO_NUMBER; channel++ )
		servo_set_pwm(channel, (uint16_t)(0.7*TICKS_PER_MS));
}
#else
void sweepServo(void)
{
}
#endif
