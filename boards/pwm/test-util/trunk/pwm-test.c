#include <stdio.h>
#include <stdint.h>
#include "i2c-dev.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define ADDRESS 0x1e
#define POS0 3
#define POS1 135

typedef enum
{
	FALSE = 0, TRUE
} bool;

bool err_enable = TRUE;

/* Enable the use of the PEC*/
void i2c_pec_enable( int fd );

/* Disable use of the PEC */
void i2c_pec_disable( int fd );

void setservo( int fd, uint8_t n, uint8_t val )
{
	uint16_t p;

	p = ((uint16_t)val<<8) | n;

	if( i2c_smbus_write_word_data( fd, 1, p ) < 0 && err_enable )
		fprintf( stderr, "i2c failed: %m\n" );
}

void spam( int fd )
{
	uint8_t addr = 0;
	err_enable = FALSE;

	for( addr=0; addr<128; addr++ )
	{
		if( addr == ADDRESS )
			addr++;

		if( ioctl( fd, I2C_SLAVE, addr ) < 0 )
		{
			fprintf( stderr, "Failed to set slave address: %m\n" );
			break;
		}

		setservo( fd, 1, 0 );
	}
}

int main( int argc, char** argv )
{
	uint8_t d = POS0;
	uint8_t val = 0;
	uint8_t servo,i;
	int fd;

	fd = open( "/dev/i2c-0", O_RDWR );

	if( argc == 2 )
	{
		if( strcmp( argv[1], "spam" ) == 0 )
			spam(fd);
		return 0;
	}

	if( argc != 3 )
	{
		printf("Usage: i2c-test SERVO VALUE\n");
		return 1;
	}

	servo = atol( argv[1] );
	val = atol( argv[2] );	

	printf( "servo: %u value: %u\n", servo, val );

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
	i2c_pec_enable(fd);
	setservo(fd,servo,val);
	
	return 0;
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

