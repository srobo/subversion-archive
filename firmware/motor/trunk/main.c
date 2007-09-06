#include "common.h"
#include "motor.h"
#include "pwm.h"
#include "i2c.h"
#include "init.h"

static int i = 0;

void UNEXPECTED()
{
}

int main( void )
{
	i = 0;

	/* Disable the watchdog timer */
	WDTCTL = WDTHOLD | WDTPW;

	init();

	motor_set( 0, 0, M_OFF );
	motor_set( 1, 0, M_OFF );

	while(1);
}
