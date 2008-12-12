#include "i2c_reg.h"
#include "types.h"
#include <string.h>
#include <signal.h>
#include "led.h"
#include "switch.h"
#include "power.h"
#include "battery.h"
#include "timed.h"
#include "device.h"



/* Register functions:
 * i2ct: The type of the register.
 * i2cr: Read from the register.
 * i2cw: Write to the register.
 * i2cs: The size of the register. */

/* Read only register */

/* Read only register */
#define I2C_REG_RO( NAME )				\
	uint8_t i2cr_ ## NAME ( uint8_t *data );	\
	uint16_t i2cs_ ## NAME ( void )

/* Write only register */
#define I2C_REG_WO( NAME )					\
	uint16_t i2cs_ ## NAME ( void );			\
	void i2cw_ ## NAME ( uint8_t* data, uint8_t len )

/* Read/write register */
#define I2C_REG( NAME )						\
	I2C_REG_RO( NAME );					\
	void i2cw_ ## NAME ( uint8_t* data, uint8_t len )

#define I2C_REG_ENTRY_RO( NAME )		\
	{					\
		.size = i2cs_ ## NAME,		\
			.read = i2cr_ ## NAME,	\
			.write = NULL,		\
			}

#define I2C_REG_ENTRY_WO( NAME )		\
	{					\
		.size = i2cs_ ## NAME,		\
			.read = NULL,		\
			.write = i2cw_ ## NAME,	\
			}

#define I2C_REG_ENTRY( NAME )			\
	{					\
		.size = i2cs_ ## NAME,		\
			.read = i2cr_ ## NAME,	\
			.write = i2cw_ ## NAME,	\
			}

/* Register functions */
I2C_REG_RO( identify );		
I2C_REG( led );		
I2C_REG_RO( dipswitch );	
I2C_REG_RO( button );	
I2C_REG( slug_power );	
I2C_REG( servo_power );
I2C_REG( motor_power );		
I2C_REG_RO( battery );	
I2C_REG_RO( volt );	
I2C_REG_RO( amp );	
I2C_REG_WO( beegees );	
I2C_REG( test );
/* When adding new commands, make sure you change I2C_NUM_COMMANDS */

const reg_access_t dev_regs[] = 
{
	I2C_REG_ENTRY_RO( identify ), 
	I2C_REG_ENTRY( led ),	      
	I2C_REG_ENTRY_RO( dipswitch ), 
	I2C_REG_ENTRY_RO( button ),
	I2C_REG_ENTRY( slug_power ),  
	I2C_REG_ENTRY( servo_power ),	
	I2C_REG_ENTRY( motor_power ),	   
	I2C_REG_ENTRY_RO( battery ),   
	I2C_REG_ENTRY_RO( volt ),
	I2C_REG_ENTRY_RO( amp ),
	I2C_REG_ENTRY_WO( beegees ), 
	I2C_REG_ENTRY( test ),   
};


uint16_t i2cs_identify( void )
{
	return 2;
}

uint8_t i2cr_identify( uint8_t *data )
{
	data[0] = 0x50;
	data[1] = 0x43;
	return 2;
}


/* Led Handler */

uint16_t i2cs_led( void )
{
	return 1;
}

uint8_t i2cr_led( uint8_t *data )
{
	data[0]=led_get();
	return 1;
}

void i2cw_led( uint8_t* data, uint8_t len )
{
	led_set(data[0]);
}

/* Dip switch Handler */


uint16_t i2cs_dipswitch( void )
{
	return 1;
}

uint8_t i2cr_dipswitch( uint8_t* data )
{
	data[0] =switch_get();
	return 1;
}


/* Go TEAM! start button handler */

uint16_t i2cs_button( void )
{
	return 1;
}


uint8_t i2cr_button( uint8_t* data )
{
	data[0]=23;
	return 1;
}


/* slug power handler */

uint16_t i2cs_slug_power( void )
{
	return 1;
}

void i2cw_slug_power( uint8_t* data, uint8_t len )
{
	pwr_set_slug(data[0]);
}

uint8_t i2cr_slug_power( uint8_t *data )
{
	data[0]=pwr_get_slug();
	return 1;
}

/* servo power handler */


uint16_t i2cs_servo_power( void )
{
	return 1;
}

uint8_t i2cr_servo_power( uint8_t *data )
{
	data[0]= pwr_get_servo();
	return 1;
}

void i2cw_servo_power( uint8_t* data, uint8_t len )
{
	pwr_set_servo(data[0]);
}

/* motor handler */


uint16_t i2cs_motor_power( void )
{
	return 1;
}

uint8_t i2cr_motor_power( uint8_t *data )
{
	data[0]= pwr_get_motor();
	return 1;
}

void i2cw_motor_power( uint8_t* data, uint8_t len )
{
	pwr_set_motor(data[0]);
}


/* Battery */


uint16_t i2cs_battery( void )
{
	return 1;
}

uint8_t i2cr_battery( uint8_t *data )
{
	data[0]= battery_get();
	return 1;
}


/* voltage */


uint16_t i2cs_volt( void )
{
	return 1;
}

uint8_t i2cr_volt( uint8_t *data )
{
	data[0]= 'v';
	return 1;
}


/* current */


uint16_t i2cs_amp( void )
{
	return 1;
}

uint8_t i2cr_amp( uint8_t *data )
{
	data[0]= 'i';
	return 1;
}





/* Beegees - Staying Alive :-) */

uint16_t i2cs_beegees( void )
{
	return 1;
}


void i2cw_beegees( uint8_t *data, uint8_t len )
{
	stayingalive();
}


/* test Handler */

uint16_t i2cs_test( void )
{
	return 2;
}

uint8_t i2cr_test( uint8_t *data )
{
	togc;
	data[0]= TAR&0xff;
	data[1]= ((TAR&0xff00)>>8);
	return 2;
}

void i2cw_test( uint8_t* data, uint8_t len )
{
	data[0]=42;
	data[1]=42;
	togd;
}
