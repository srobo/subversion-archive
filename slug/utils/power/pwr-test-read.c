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

//#define ADDRESS 0x55
#define ADDRESS 0x3f

typedef enum
{
	FALSE = 0, TRUE
} bool;

bool err_enable = TRUE;

uint16_t readword(int fd, uint8_t cmd) {
    signed int tmp;
    tmp = i2c_smbus_read_word_data( fd, cmd );
    if(tmp == -1){
        printf("Error reading byte.\n");
        printf("Got 0x%02x.\n");
        return 0;
    }

    return tmp;
}



uint8_t readbyte(int fd, uint8_t cmd) {
    signed int tmp;
    tmp = i2c_smbus_read_byte_data( fd, cmd );
    if(tmp == -1){
        printf("Error reading byte.\n");
        printf("Got 0x%02x.\n");
        return 0;
    }

    return 0xFF & tmp;
}

char setpins( int fd, uint8_t command, uint8_t val )
{
	uint16_t p;

	if( i2c_smbus_write_byte_data( fd, command , val ) < 0 && err_enable )
	  {
		fprintf( stderr, "i2c failed: %m\n" );
		return 1;
	  }
	return 0;
}

int main( int argc, char** argv )
{
	int fd;

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



    if( ioctl( fd, I2C_PEC, 1) < 0)
    {
        fprintf( stderr, "Failed to enable PEC\n");
        return 3;
    }


    char val = 0xa5;
	while(1) 
	  {
	    
	    //printf("Read dips as %x\n", readbyte(fd, 5));
	    //printf("Read ID as %x\n", readbyte(fd, 0));
	    //printf("Read rails as %x\n", readbyte(fd, 7));
	    //printf("Read v as %x\n", readword(fd, 3));	
	    //printf("Read I as %x\n", readword(fd, 4));	
	    printf("setpins to %x returned %x\n",val,setpins(fd,1,val));
	    val = ~val;
	
        sleep(1);
	  }
	return 0;
}
