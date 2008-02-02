#include <stdio.h>
#include <stdint.h>
#include "i2c-dev.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define ADDRESS 0x14

/* The commands */
enum
{
	/* Get the identity from the board */
	JOINTIO_IDENTIFY,
	/* Set the outputs of the board */
	JOINTIO_OUTPUT,
	/* Read the board inputs */
	JOINTIO_INPUT,

	JOINTIO_TEST = 3
};

typedef enum
{
	FALSE = 0, TRUE
} bool;

/* Open and configure I2C device.
   Returns the file descriptor. */
int jointio_i2c_conf( void );

/* Get the JointIO Identity. */
uint32_t jointio_identify( int fd );

/* Dump data to the terminal */
void dump_data( uint8_t *data, uint32_t len );

int main( int argc, char** argv )
{
	int fd;

	fd = jointio_i2c_conf();

	printf( "Test result: %x\n", (uint32_t)i2c_smbus_read_byte_data(fd, JOINTIO_TEST) );

/* 	printf( "Read identity as %8.8x\n", jointio_identify(fd) ); */

	return 0;
}

int jointio_i2c_conf( void )
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

	if( ioctl( fd, I2C_PEC, 0) < 0) 
	{ 
		fprintf( stderr, "Failed to enable PEC\n"); 
		exit(3);
	} 

	return fd;
}

uint32_t jointio_identify( int fd )
{
	uint8_t tmp[4];
	uint32_t ident = 0;
	int r;

	/* Send the IDENTIFY command. */
	if( i2c_smbus_write_quick( fd, JOINTIO_IDENTIFY ) < 0 )
	{
		fprintf( stderr, "Failed to read identity: %m\n" );
		exit(4);
	}
	
	r = read( fd, tmp, 4 );
	if( r < 4 )
	{
		fprintf( stderr, "Identity could not be read: %m\n" );
		if( r  > 0 )
		{
			fprintf( stderr, "%i bytes were read: ", r );
			dump_data( tmp, r );
			printf("\n");
		}
		exit(4);
	}

	dump_data( tmp, 4 );

	for( r=0; r<4; r++ )
		ident |= ((uint32_t)tmp[r]) << (8*r);

	return ident;
}

void dump_data( uint8_t *data, uint32_t len )
{
	uint32_t i;

	for(i=0;i<len;i++)
	{
		if( i != 0 )
			printf( " " );

		printf( "%2.2hhx", data[i] );
	}
}
