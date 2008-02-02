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

/* Set the digital outputs.
   Returns FALSE on failure. */
bool jointio_set_outputs( int fd, uint8_t val );

/* Structure for holding inputs */
typedef struct
{
	uint8_t digital;
	uint16_t an[16];
} input_t;

/* Reads the inputs.
   Returns FALSE on failure.
   Fills in *v with the inputs. */
bool jointio_get_inputs( int fd, input_t *v );

/* Dump data to the terminal */
void dump_data( uint8_t *data, uint32_t len );

/* Perform the tests */
void jointio_test( int fd );

int main( int argc, char** argv )
{
	int fd;

	fd = jointio_i2c_conf();

	/* Are we in test mode? */
	if( argc == 2 && strcmp(argv[1],"test") == 0 )
		jointio_test(fd);
	else
	{
		printf( "Test result: %x\n", (uint32_t)i2c_smbus_read_byte_data(fd, JOINTIO_TEST) );

		/* printf( "Read identity as %8.8x\n", jointio_identify(fd) );  */
	}

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

	if( ioctl( fd, I2C_PEC, 1) < 0) 
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

bool jointio_set_outputs( int fd, uint8_t val )
{
	if( i2c_smbus_write_byte_data( fd, JOINTIO_OUTPUT, val ) < 0 )
		return FALSE;
	else
		return TRUE;
}

bool jointio_get_inputs( int fd, input_t *v )
{
	uint8_t buf[17];

	/* Write the command byte */
	if( i2c_smbus_write_quick( fd, JOINTIO_INPUT ) < 0 )
		return FALSE;

	if( read( fd, buf, 17 ) )
	{
		
	}	

	return FALSE;
}

void jointio_test( int fd )
{
	uint8_t i;

	printf("jointio: test mode\n");
	printf("Make sure that input 0 is connected to output 0.\n");

	for( i=0; i<10; i++ )
	{
		if( ! jointio_set_outputs( fd, 0 ) ) 
		{
			printf("Error: Failed to set output: %m\n");
			exit(2);
		}

		
			
		

		if( ! jointio_set_outputs( fd, 1 ) )
		{
			printf("Error: Failed to set output: %m\n");
			exit(2);

		}
	}		

	printf("Tests passed\n");		
}
