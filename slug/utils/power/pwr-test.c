#include <stdio.h>
#include <stdint.h>
#include "i2c-dev.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* PCF8574A address: 01000000 = 0x40 */


//#define ADDRESS 0x0F
#define ADDRESS 0x20


#define POS0 3
#define POS1 135

typedef enum
{
	FALSE = 0, TRUE
} bool;

bool err_enable = TRUE;

void setpins( int fd, uint8_t val )
{
	uint16_t p;

	

	if( i2c_smbus_write_byte_data( fd, 1, val ) < 0 && err_enable )
		fprintf( stderr, "i2c failed: %m\n" );
}


int main( int argc, char** argv )
{
	uint8_t d = POS0;
	uint8_t val = 0;
	uint8_t servo,i;
	int fd;

	struct timespec pause = {0,5000000};
	fd = open( "/dev/i2c-0", O_RDWR );

	if( fd == -1 )
	{
		fprintf( stderr, "Failed to open /dev/`i2c-0: %m\n" );
		return 1;
	}

	if( ioctl( fd, I2C_SLAVE, ADDRESS ) < 0 )
	{
		fprintf( stderr, "Failed to set slave address: %m\n" );
		return 2;
	}


	printf("hamster\n");

	val = 0xAA;
	while(1) 
	  {
	    //val=~val;
	    //for(i=0;i<255;i++)
	      // { 
		setpins(fd,0xAA);
	     sleep(1);
		printf("sp%d\n",i) ;
		// }
	     setpins(fd,0x55);
	     sleep(1);
	      printf("eon\n") ;
	     

//nanosleep(&pause,NULL);
//sleep(1);
	  }

	
	
	return 0;
}
