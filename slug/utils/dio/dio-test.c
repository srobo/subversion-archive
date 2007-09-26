#include <stdio.h>
#include <stdint.h>
#include "i2c-dev.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define ADDRESS 0x20
#define POS0 3
#define POS1 135

typedef enum
{
	FALSE = 0, TRUE
} bool;

bool err_enable = TRUE;

int main( int argc, char** argv )
{
	uint8_t d = POS0;
	uint8_t val = 0;
	uint8_t setting;
	int fd;

	fd = open( "/dev/i2c-0", O_RDWR );

	if( argc != 2 )
	{
		printf("Usage: dio-test VALUE\n");
		return 1;
	}

	setting = atol( argv[1] );

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

/*     if( ioctl( fd, I2C_PEC, 1) < 0) */
/*     { */
/*         fprintf( stderr, "Failed to enable PEC\n"); */
/*         return 3; */
/*     } */

	if( i2c_smbus_write_byte( fd, setting ) < 0 )
		fprintf( stderr, "Failed to set io\n" );
	
	return 0;
}
