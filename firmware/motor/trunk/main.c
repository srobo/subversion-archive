#include "common.h"
#include "motor.h"
#include "pwm.h"
#include "i2c.h"
#include "init.h"

static int i = 0;

int main( void )
{
	pwm_ratio_t v = 0;

	/* Disable the watchdog timer */
	WDTCTL = WDTHOLD | WDTPW;

	init();

	motor_set( 0, 0, M_FORWARD );
	motor_set( 1, 0, M_FORWARD );

	pwm_set( 0, 100 );
	pwm_set( 1, 100 );
	
	while(1)
	{
		uint32_t i;
		v += 10;

		if( v > PWM_MAX )
			v = 0;

		for(i=0; i<10000;i++);

		pwm_set( 0, v );
		pwm_set( 1, v );

	}
}
