#include <stdio.h>
#include <stdint.h>
#include "i2c-dev.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "smbus_pec.h"

#define ADDRESS 0x14

#define I2C_DEVICE "/dev/i2c-0"

/* The commands */
enum
{
	/* Get the identity from the board */
	JOINTIO_IDENTIFY,
	/* Set the outputs of the board */
	JOINTIO_OUTPUT,
	/* Read the board inputs in analogue form */
	JOINTIO_INPUT,
	/* Read the current digital output setting from the board. */
	JOINTIO_OUTPUT_READ,
	/* Read the inputs in digital form */
	JOINTIO_INPUT_DIG
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

/* Read the state of the digital outputs.
   Returns FALSE on failure. */
bool jointio_read_outputs( int fd, uint8_t *val );

/* Set the digital outputs without failure. */
void jointio_set_outputs_retry( int fd, uint8_t val );

/* Reads the inputs.
   Returns FALSE on failure.
   Fills in *v with the inputs - v is an 8 entry array. */
bool jointio_get_inputs( int fd, uint16_t *v );

/* Reads the input in digital form */
bool jointio_get_digin( int fd, uint8_t *v );

/* Dump data to the terminal */
void dump_data( uint8_t *data, uint32_t len );

/* Perform the tests */
void jointio_test( int fd );

/* Enable use of the PEC */
void i2c_pec_enable( int fd );

/* Disable use of the PEC */
void i2c_pec_disable( int fd );

int main( int argc, char** argv )
{
	int fd;

	fd = jointio_i2c_conf();

	if( argc < 2 )
	{
		fprintf(stderr, "Usage: %s {output,input,digin}\n", argv[0]);
		exit(1);
	}
	
	if( strcmp( argv[1], "output" ) == 0 )
	{
		uint8_t val;
		if( argc < 3 ) {
			fprintf( stderr, "Usage: %s output VAL\n", argv[0] );
			exit(2);
		}
		
		val = (uint8_t)strtoul( argv[2], NULL, 10 );
		jointio_set_outputs_retry( fd, val );
	}
	else if( strcmp( argv[1], "input" ) == 0 )
	{
		uint16_t r[8];
		uint16_t count;

		if( argc > 2 ) {
			fprintf( stderr, "Usage: %s input\n", argv[0] );
			exit(2);
		}

		count = 0;
		while( !jointio_get_inputs(fd, r ) && count < 1000)
			count++;

		if( count > 1000 )
			printf("Error: Failed to read inputs - tried 1000 times\n");
		else
		{
			uint8_t i;

			for(i=0; i<8; i++)
				printf("Input %i = %hu\n", i, r[i]);
		}
	}
	else if( strcmp( argv[1], "digin" ) == 0 )
	{
		uint8_t in, i;

		if( argc > 2 ) {
			fprintf( stderr, "Usage: %s digin\n", argv[0] );
			exit(2);
		}

		while(! jointio_get_digin( fd, &in ) );

		for(i=0; i<8; i++)
			printf("Input %i = %i\n", 
			       i, (1<<i & in)?1:0);
	}		

	return 0;
}

int jointio_i2c_conf( void )
{
	int fd;

	fd = open( I2C_DEVICE, O_RDWR );

	if( fd == -1 )
	{
		fprintf( stderr, "Failed to open " I2C_DEVICE ": %m\n" );
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
	i2c_pec_enable(fd);

	/* There are only 4 outputs */
	val &= 0x0f;

	if( i2c_smbus_write_byte_data( fd, JOINTIO_OUTPUT, val ) < 0 )
	{
		printf("Failed: %m\n");
		return FALSE;
	}
	else
		return TRUE;
}

bool jointio_get_inputs( int fd, uint16_t *v )
{
	uint8_t buf[17];
	uint8_t i;
	uint8_t c = 0;

	/* Disable PEC for this command */
	i2c_pec_disable( fd );

	/* Write the command byte */
	if( i2c_smbus_write_byte( fd, JOINTIO_INPUT ) < 0 )
		return FALSE;

	c = crc8(ADDRESS<<1);
	c = crc8(c ^ JOINTIO_INPUT);
	c = crc8(c ^ ((ADDRESS<<1)|1));

	if( read( fd, buf, 17 ) < 17 )
		return FALSE;

	for( i=0; i<8; i++ )
	{
		v[i] = ( (uint16_t)buf[ i*2 ] ) << 8;
		v[i] |= buf[i*2 + 1];

		c = crc8( c ^ buf[i*2] );
		c = crc8( c ^ buf[i*2+1] );
	}

	/* Enable the PEC again */
	i2c_pec_enable( fd );

	if( c != buf[16] )
		return FALSE;
	else
		return TRUE;
}

void jointio_test( int fd )
{
	uint8_t i;

	printf("jointio: test mode\n");
	printf("Make sure that input 0 is connected to output 0.\n");

	for( i=0; i<10; i++ )
	{
		jointio_set_outputs_retry( fd, 0 );


		jointio_set_outputs_retry( fd, 1 );
	}		

	printf("Tests passed\n");		
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

bool jointio_read_outputs( int fd, uint8_t *val )
{
	int32_t r;

	i2c_pec_enable(fd);
	r = i2c_smbus_read_byte_data( fd, JOINTIO_OUTPUT_READ );

	if( r < 0 )
		return FALSE;

	*val = (uint8_t)r;
	printf("Read %hhx\n", *val);

	return TRUE;
}

void jointio_set_outputs_retry( int fd, uint8_t val )
{
	uint8_t d = 0;

	val &= 0x0f;
	
	do
	{
		while( !jointio_set_outputs(fd, val) )
		{
			printf("Wrote: %hhx\n", val);
		}

		while( !jointio_read_outputs(fd, &d) )
		{
			printf("Read: %hhx\n", d);
		}
	}
	while( d != val );
}

bool jointio_get_digin( int fd, uint8_t *v )
{
	int32_t r;
	i2c_pec_enable(fd);

	r = i2c_smbus_read_byte_data(fd, JOINTIO_INPUT_DIG);
	if( r < 0 )
		return FALSE;

	*v = (uint8_t)r;
	return TRUE;
}
