#include "i2c-watchdog.h"
#include <stdint.h>
#include "i2c.h"

static uint8_t count = 0;

void i2c_watchdog_start( void )
{
	count = 1;
}

void i2c_watchdog_stop( void )
{
	count = 0;
}

void i2c_watchdog_check( void )
{
	if(count == 1)
		count++;
	else if( count == 2 )
	{
		/* Reset the I2C */
		i2c_init();
		i2c_enable();

		count = 0;
	}
}
