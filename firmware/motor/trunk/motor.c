#include "common.h"
#include "motor.h"

typedef struct 
{
	motor_state_t state;
	speed_t speed;
} motor_t;

/* The motors */
motor_t motors[2];

/* Set the speed/state of a motor */
void motor_set( uint8_t channel, speed_t speed,  motor_state_t state )
{
	if( motors[channel].speed != speed )
	{
		pwm_set( channel, (pwm_ratio_t)speed );
		motors[channel].speed = speed;
	}

	/* Only set the state if it's changed */
	if( motors[channel].state != state )
	{
		uint8_t v = 0;
		uint8_t p;

		switch( state )
		{
		case M_OFF:
			v = 1; break;
		case M_FORWARD:
			v = 2; break;
		case M_BACKWARD:
			v = 3; break;
		default:
			//case PWM_BRAKE:
			v = 0; break;
		}

		/* Calculate the shift necessary */
		if( channel == 0 )
			p = 3;
		else
			p = 5;

		P3OUT &= ~( 3 << p );
		P3OUT |= v << p;

		motors[channel].state = state;
	}

}

/* Get the speed of a motor */
speed_t motor_get_speed( uint8_t channel )
{
	return motors[channel].speed;
}

/* Get the state of a motor */
motor_state_t motor_get_state( uint8_t channel )
{
	return motors[channel].state;
}

/* Initialise the motors */
void motor_init( void )
{
	uint8_t i;

	for( i=0; i<2; i++ )
	{
		motors[i].state = M_BRAKE;
		motors[i].speed = 1;
		motor_set( i, 0, M_OFF );
	}
}
