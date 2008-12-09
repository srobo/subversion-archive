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
#include "pwr_qual.h"


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
	
	printf("command: %d,val: %d",command,val);

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
	printf("1\n");
	fd = open( "/dev/i2c-1", O_RDWR );
	printf("2\n");
	if( fd == -1 )
	{
		fprintf( stderr, "Failed to open /dev/`i2c-1: %m\n" );
		return 1;
	}

	printf("3\n");

	if( ioctl( fd, I2C_SLAVE, ADDRESS ) < 0 )
	{
		fprintf( stderr, "Failed to set slave address: %m\n" );
		return 2;
	}

	printf("4\n");

	if( ioctl( fd, I2C_PEC, 1) < 0)
	{
		fprintf( stderr, "Failed to enable PEC\n");
		return 3;
	}


	printf("5\n");
//------------------------------
	if (argc<2){ // check at least 1 arg
		printf("incorrect args\n");
		//printf("Usage: %s {w,l,v,i,r,s}\n");
		printf("w - identify(currently unsupported locally)\n"
		       "l - set led values, needs 2 args\n"
		       "v - read voltage\n"
		       "i - read current\n"
		       "r - get rail values\n"
		       "s - set rail values - be REALLY CAREFULL! ( needs 2 args)\n"
		       "d - get dip switch values\n"
		       "u - check for usb cabble plugged in\n"
		       "o - send char out ( needst args)\n");
		return -1;
	}

	
	switch( *argv[1]){
	case 'w':
		//printf("Read ID as %x\n", readbyte(fd, 0));
		printf("sorry you cant because it sucks\n");
		return -1;
		break;
	case 'l':
		printf("Leds\n");
		if (argc!=3)
		{
			printf("l takes another argument for the value\n");
			return -1;
		}
		printf("cmd:%d\n",LED);
		printf("%d\n",setpins(fd,LED,atoi(argv[2])));
		return 0;
/* 	case 'v': */
/* 		printf("%d\n", readword(fd, GETV)); */
/* 		return 0; */
/* 	case 'i': */
/* 		printf("%d\n", readword(fd, GETI)); */
/* 		return 0; */
/* 	case 's': */
/* 		if (argc!=3) */
/* 		{ */
/* 			printf("l takes another argument for the value\n"); */
/* 			return -1; */
/* 		} */
/* 		printf("%d\n", setpins(fd, SETRAILS,atoi(argv[2]))); */
/* 		return 0; */
/* 	case 'r': */
/* 		printf("%d\n", readbyte(fd, GETRAILS)); */
/* 		return 0; */

/* 	case 'x': */
/* 		printf("%d\n", readword(fd, GETRAILS)); */
/* 		return 0; */

/* 	case 'o': */
/* 		if (argc!=3) */
/* 		{ */
/* 			printf("l takes another argument for the value\n"); */
/* 			return -1; */
/* 		} */
/* 		printf("%d\n", setpins(fd,SENDSER,*argv[2])); */
/* 		return 0; */
	case 'd':
		printf("%d\n", readbyte(fd, DIPSWITCH));
		return 0;

/* 	case 'u': */
/* 		printf("%d\n", readbyte(fd, ISUSB)); */
/* 		return 0; */
/* 	case 'q': */
/* 		printf("break everything?\n"); */
/* 		printf("%d\n", readbyte(fd, 93)); */
/* 		break; */

	case 'e':
		printf("Ah Ah Ah Ah, stayin' alive!\n");
		printf("%d\n",setpins(fd,BEEGEES,1));
		//sends alive packet!
		break;
		
	default:
		printf("not a recognised command go think again\n");
	}

	return -1;


}
