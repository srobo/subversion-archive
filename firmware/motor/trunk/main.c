#include "common.h"
#include "motor.h"

static int i = 0;

int main( void )
{
	/* Disable the watchdog timer */
	WDTCTL = WDTHOLD | WDTPW;

	init();

	while(1)
	{
		motor_set( 0, 0, M_OFF );
		motor_set( 0, 0, M_FORWARD );
		motor_set( 0, 0, M_BACKWARD );
		motor_set( 0, 0, M_BRAKE );

		motor_set( 1, 0, M_OFF );
		motor_set( 1, 0, M_FORWARD );
		motor_set( 1, 0, M_BACKWARD );
		motor_set( 1, 0, M_BRAKE );
	}

	while(1);
}
