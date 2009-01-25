#include <stdio.h>
#include <stdint.h>
#include "i2c-dev.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#define ADDRESS 0x12

/* The I2C Commands */
enum
{
	/* Get the board identity */
	COMMAND_IDENTIFY,

	/* Configure a motor */
	COMMAND_SET,

	/* Get motor 0 settings */
	COMMAND_GET0,

	/* Get motor 1 settings */
	COMMAND_GET1,
	
	/* Get Firmware version */
	COMMAND_FWVER,

	/*Firware Reception & Chunk transmission*/
	COMMAND_FWCHUNK,

	/* Firmware CRC Transmission */
	COMMAND_FWCONFIRM,

	/* Feedback read*/
	COMMAND_FEEDBACK,

	/* current 0 */
	COMMAND_CURRENT0,
	/*current 1 */
	COMMAND_CURRENT1	
};

typedef enum
{
	FALSE = 0, TRUE
} bool;

typedef enum
{
	M_OFF = 0,
	M_FORWARD,
	M_BACKWARD,
	M_BRAKE,

	M_INVALID
} motor_state_t;

#define MOTOR_MAX 328

typedef uint16_t pwm_ratio_t;

/* Configure the i2c device */
int motor_i2c_conf( void );

/* Enable use of the PEC */
void i2c_pec_enable( int fd );

/* Disable use of the PEC */
void i2c_pec_disable( int fd );

/* Send the identify command */
void motor_identify( int fd );

/* Send a command to configure a motor. */
bool motor_set( int fd, uint8_t m, motor_state_t s, pwm_ratio_t val );

/* Read back a motor's settings.
   Returns FALSE on error. */
bool motor_read( int fd, uint8_t m, motor_state_t *s, pwm_ratio_t *val );

/* Configure a motor, but read back and try again. */
void motor_set_retry( int fd, uint8_t m, motor_state_t s, pwm_ratio_t val );

/* Read feedback pins.
   Lower 4 bits of return value are the feedback input bits. 
   Returns negative value on fail. */
int16_t motor_fback_read( int fd );

/* Wrapper around motor_fback_read that retries forever until there's no failure */
uint8_t motor_fback_read_retry( int fd );

/* Read current values 8 */
void motor_current_read( int fd);

int main( int argc, char** argv )
{
	int fd;
	uint8_t channel;
	motor_state_t dir;
	pwm_ratio_t pwm;
	bool test = FALSE;
	bool fback = FALSE;
	bool current = FALSE;

	if( argc == 2 && strcmp( argv[1], "test" ) == 0 )
		/* Run tests */
		test = TRUE;

	else if( argc == 2 && strcmp( argv[1], "fback") == 0)
		/* run feedback*/
		fback = TRUE;

	else if( argc == 2 && strcmp( argv[1], "current") == 0)
		/* run current*/
		current = TRUE;

	else if( argc < 4 )
	{
		printf("Usage: %s CHANNEL DIR PWM_VALUE\n"
		       "Or: %s test\n"
		       "Where:\n"
		       "\t CHANNEL is the motor number (0 or 1).\n"
		       "\t DIR is the motor direction:\n"
		       "\t\tf = foward\n"
		       "\t\tb = backward\n"
		       "\t\to = off\n"
		       "\t\ts = brake\n"
		       "\t PWM_VALUE is a member of [0,%u]\n"
		       , argv[0], argv[0], MOTOR_MAX );
		return 1;
	}

	fd = motor_i2c_conf();

	if( test )
	{

	}
	else if( fback)
	{
		uint8_t v = motor_fback_read_retry(fd);
		uint8_t i;
		printf( "Feedback pins:\n" );
		for( i=0; i<4; i++ ) 
			printf( "\tPin %hhu = %hhu\n", i, ((1<<i)&v)?1:0 );
	}
	else if( current)
	{
		printf("Reading Motor currents:\n");
		motor_current_read(fd);	
	}
	else
	{
		channel = strtoul( argv[1], NULL, 10 );
		switch( *argv[2] )
		{
		case 'f':
			dir = M_FORWARD;
			break;
		case 'b':
			dir = M_BACKWARD;
			break;
		case 'o':
			dir = M_OFF;
			break;
		case 's':
			dir = M_BRAKE;
			break;
		default:
			fprintf(stderr, "Invalid direction code - quitting\n");
			return 2;
		}
		pwm = strtoul( argv[3], NULL, 10 );

		motor_set_retry( fd, channel, dir, pwm );
	}
	
	return 0;
}

int motor_i2c_conf( void )
{
	int fd;

	fd = open( "/dev/i2c-0", O_RDWR );

	if( fd == -1 )
	{
		fprintf( stderr, "Failed to open /dev/i2c-0: %m\n" );
		exit(1);
	}

	if( ioctl( fd, I2C_SLAVE, ADDRESS ) < 0 )
	{
		fprintf( stderr, "Failed to set slave address: %m\n" );
		exit(2);
	}

	i2c_pec_enable( fd );

	return fd;
}

void i2c_pec_enable( int fd )
{
	if( ioctl( fd, I2C_PEC, 1) < 0) 
	{ 
		fprintf( stderr, "Failed to enable PEC\n"); 
		exit(3);
	} 
}

void i2c_pec_disable( int fd )
{
	if( ioctl( fd, I2C_PEC, 0) < 0) 
	{ 
		fprintf( stderr, "Failed to disable PEC\n"); 
		exit(3);
	} 
}

bool motor_set( int fd, uint8_t m, motor_state_t s, pwm_ratio_t val )
{
	uint16_t v = 0;
	if( val>MOTOR_MAX )
		val = MOTOR_MAX;

	v = val;
	v |= ((uint16_t)s) << 9;
	v |= m?0x800:0;

	if( i2c_smbus_write_word_data( fd, COMMAND_SET, v ) < 0 )
		return FALSE;

	return TRUE;
}

void motor_identify( int fd )
{
	int id;

	id = i2c_smbus_read_word_data( fd, COMMAND_IDENTIFY );

	if( id < 0 )
		fprintf( stderr, "Failed to read motor identity: %m\n" );
	else
		printf( "Identified as %X\n", id );
}

bool motor_read( int fd, uint8_t m, motor_state_t *s, pwm_ratio_t *val )
{
	int32_t r;
	uint16_t d;
	uint8_t command;

	if( m == 0 )
		command = COMMAND_GET0;
	else
		command = COMMAND_GET1;

	r = i2c_smbus_read_word_data( fd, command );
	
	if( r < 0 )
		return FALSE;

	d = (uint16_t)r;

	/* Data returned. Bits:
	    8-0: Speed
	   9-10: Direction */

	*val = d & 0x01ff;
	*s = (d >> 9) & 3;

	return TRUE;
}

void motor_set_retry( int fd, uint8_t m, motor_state_t s, pwm_ratio_t val )
{
	motor_state_t r_s = M_INVALID;
	pwm_ratio_t r_v;
	uint32_t a_s=0, a_r=0, attempts = 0;

	do
	{
		while( !motor_set( fd, m, s, val ) )
			a_s++;

		while( !motor_read( fd, m, &r_s, &r_v )	)
			a_r++;

		attempts++;
	}
	while( r_s != s && r_v != val );

	printf("%u attempts. s=%u, r=%u\n", attempts, a_s, a_r);
}

int16_t motor_fback_read( int fd )
{
	int32_t r;

	r = i2c_smbus_read_byte_data(fd, COMMAND_FEEDBACK);

	if ( r < 0 )
		return -1;

	return (int16_t)r;	
}

uint8_t motor_fback_read_retry( int fd )
{
	int16_t v;

	do
		v = motor_fback_read(fd);
	while( v < 0 );

	return (uint8_t)v;
}


void  motor_current_read( int fd)
{
	int32_t r;
	r = i2c_smbus_read_word_data(fd, COMMAND_CURRENT0);
	uint16_t d = (uint16_t) r;
	fprintf(stderr, "Motor 0 Current: \t%u\n",d );

	r = i2c_smbus_read_word_data(fd, COMMAND_CURRENT1);
	d = (uint16_t) r;
	fprintf(stderr, "Motor 1 Current: \t%u\n",d );
	return;	
}
