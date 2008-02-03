#include "servo.h"

uint16_t servo_pulse[SERVO_NUMBER];

void servo_init( void )
{
	
}

void initialiseServoArray(uint16_t pulse_width)
{
	uint8_t i;

	for(i=0; i<SERVO_NUMBER; i++)
		setServoPWM(i, MIDDLE_PULSE);
}

uint16_t getServoPWM(uint8_t servo)
{
	return servo_pulse[servo];
}

void setServoPWM(uint8_t servo, uint16_t pulse_width)
{
	if(servo < SERVO_NUMBER)
	{
		if(pulse_width >= MAX_PULSE){
			pulse_width = MAX_PULSE;
		}else if(pulse_width <= MIN_PULSE){
			pulse_width = MIN_PULSE;
		}
		servo_pulse[servo] = pulse_width;
	}
}

