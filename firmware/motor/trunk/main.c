#include "common.h"
#include "motor.h"
#include "pwm.h"
#include "i2c.h"
#include "init.h"

static int i = 0;

UNEXPECTED()
{
	while(1);
}


int main( void )
{
	pwm_ratio_t v = 0;
	motor_state_t dir = M_FORWARD;
	i = 1;

	/* Disable the watchdog timer */
	WDTCTL = WDTHOLD | WDTPW;

	init();

	motor_set( 0, 0, M_FORWARD );
	motor_set( 1, 0, M_OFF );

	pwm_set( 0, PWM_MAX );
	pwm_set( 1, PWM_MAX );

	while(1)
	{
		uint32_t i;
		v += 10;

		if( v > PWM_MAX )
		{
			v = 0;
			dir = dir==M_FORWARD? M_BACKWARD : M_FORWARD;
		}

		for(i=0; i<10000;i++);
		
		motor_set( 1, v, dir );
		motor_set( 0, v, dir );
	}
}
