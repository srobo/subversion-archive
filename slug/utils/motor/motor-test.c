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

void motor_set( int fd, uint8_t m, motor_state_t s, pwm_ratio_t val )
{
	uint16_t v = 0;
	if( val>MOTOR_MAX )
		val = MOTOR_MAX;

	v = val;
	v |= ((uint16_t)s) << 9;
	v |= m?0x800:0;

	i2c_smbus_write_word_data( fd, 0, v );
}

int main( int argc, char** argv )
{
	int fd;
	uint8_t channel;
	motor_state_t dir;
	pwm_ratio_t pwm;

	if( argc < 4 )
	{
		printf("Usage: %s CHANNEL DIR PWM_VALUE\n"
		       "Where:\n"
		       "\t CHANNEL is the motor number (0 or 1).\n"
		       "\t DIR is the motor direction:\n"
		       "\t\tf = foward\n"
		       "\t\tb = backward\n"
		       "\t\to = off\n"
		       "\t\ts = brake\n"
		       "\t PWM_VALUE is a member of [0,%u]\n"
		       , argv[0], MOTOR_MAX );
		return 1;
	}

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

	fd = open( "/dev/i2c-0", O_RDWR );

	if( fd == -1 )
	{
		fprintf( stderr, "Failed to open /dev/i2c-0: %m\n" );
		return 1;
	}

	if( ioctl( fd, I2C_SLAVE, ADDRESS ) < 0 )
	{
		fprintf( stderr, "Failed to set slave address: %m\n" );
		return 2;
	}

	motor_set( fd, channel, dir, pwm );
	
	return 0;
}
