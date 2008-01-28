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
	int r;
	uint8_t setting;
	int fd;
	uint8_t buf[16]; 
	uint8_t data= 0x01;

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

	if( ioctl( fd, I2C_PEC, 1) < 0) 
	{ 
		fprintf( stderr, "Failed to enable PEC\n"); 
		return 3; 
	} 
	
	r = i2c_smbus_write_byte_data(fd, 1, 1);
	if( r < 0 )
		fprintf(stderr, "Failed to write\n");
	else
		printf( "Read %x from dio\n", r );
	

/*START of code to test reading from the input pins
  r = i2c_smbus_write_byte(fd, 2);
  if( r < 0 )
  fprintf(stderr, "Failed to read dio pins\n");
  else
  printf( "Read %x from dio\n", r );
	
  printf("%d\n",read(fd, buf, 16));	
	
  int y;
  uint16_t value;
  for(y=0;y<8;y++)
  {		
  value = ((uint16_t)buf[2*y] << 8) |  (buf[(2*y)+1]);
  printf("\n buf[%d] = %d", y, (int)value);
  printf("\n %d %d", (int)buf[2*y], (int)buf[(2*y)+1]);
  }
  END of code to test reading from the inputs pins*/ 
	
/* 	if( i2c_smbus_write_byte( fd, setting ) < 0 ) */
/* 		fprintf( stderr, "Failed to set io\n" ); */
	
	return 0;
}
