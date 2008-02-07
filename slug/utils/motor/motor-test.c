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
	COMMAND_GET1
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
	M_BRAKE
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
void motor_set( int fd, uint8_t m, motor_state_t s, pwm_ratio_t val );

int main( int argc, char** argv )
{
	int fd;
	uint8_t channel;
	motor_state_t dir;
	pwm_ratio_t pwm;
	bool test = FALSE;

	if( argc == 2 && strcmp( argv[1], "test" ) == 0 )
		/* Run tests */
		test = TRUE;
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

		motor_set( fd, channel, dir, pwm );
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

void motor_set( int fd, uint8_t m, motor_state_t s, pwm_ratio_t val )
{
	uint16_t v = 0;
	if( val>MOTOR_MAX )
		val = MOTOR_MAX;

	v = val;
	v |= ((uint16_t)s) << 9;
	v |= m?0x800:0;

	if( i2c_smbus_write_word_data( fd, COMMAND_SET, v ) < 0 )
		fprintf( stderr, "Failed to set motor: %m\n" );
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

