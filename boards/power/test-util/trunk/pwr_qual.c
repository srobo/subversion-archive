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
//#include "smbus_pec.h"
#define POLY    (0x1070U << 3)

int init_i2c(void);
int32_t sr_block_read( int fd, uint8_t reg, uint8_t **buf );
int pecoff(int fd);
int pecon(int fd);




//#if USE_CHECKSUMS
uint8_t crc8( uint8_t tempdata )
{
	uint8_t i;
	uint16_t data;

	data = (uint16_t)tempdata<<8;
	for(i = 0; i < 8; i++) 
	{
		if (data & 0x8000)
			data = data ^ POLY;
		data <<= 1;
	}

	return (data >> 8) & 0xFF;
}
 

int main( int argc, char** argv )
{
	int fd =0;
	int retval;
	uint8_t value[30];

	uint8_t *buf;

  	fd = init_i2c();

	switch( *argv[1]){
	case 'w':
/* 		//printf("Read ID as %x\n", readbyte(fd, 0)); */
/* 		printf("sorry you cant because it sucks\n"); */
/* 		return -1; */
		sr_block_read(fd,atoi(argv[2]),&buf);
		//printf("r: %d",*buf);
		
		
		break;
	case 'l':
		if (argc!=3)
		{
			printf("l takes another argument for the value\n");
			return -1;
		}
		value[0]= atoi(argv[2]);
		retval = i2c_smbus_write_block_data(fd,LED,1,value);
		printf("returned %d",retval);
		return 0;
	}


}

/* int32_t sr_block_write( int fd, uint8_t reg, uint8_t **buf ) */


int32_t sr_block_read( int fd, uint8_t reg, uint8_t **buf )
{
	int len, r;
	uint8_t checksum, i;

	pecoff(fd); /* need to disable pec so can do 2 actions consecutively - see gumsense readme */

	
	/* We have to hack around the fact that the i2c adapter doesn't
	   support i2c block read. */
	/*  First, do a read to get the length byte. */
	/* This read byte operation also sets the command that we're doing
	   within the gumsense */

	/* Set the command and grab the length */
	len = i2c_smbus_read_byte_data( fd, reg );
	printf("len=%d\n",len);
	if( len < 0 ) {
		fprintf( stderr, "Failed to read register %hhu length\n", reg );
		fprintf( stderr, "length is %d", len );
		goto error0;
	}
	
	/* We have to read the byte count, command and checksum: */
	/* (see README) */
	*buf = (uint8_t*)malloc(len + 3);

	if( *buf == NULL )
	{
		fprintf( stderr, "Failed to allocate memory\n" );
		goto error0;
	}

	r = read(fd, *buf, len + 3 );
	
	if( r < 0 ) {
		fprintf( stderr, "Failed to read register %hhu\n", reg );
		goto error1;
	}

	if( r != len + 3 ) {
		fprintf( stderr, "Failed to read all of register %hhu\n", reg );
		goto error1;
	}

	/* Generate the checksum: */
	checksum = crc8( (ADDRESS<<1) | 1 );
	for( i=0; i<len+2; i++ )
		checksum = crc8( checksum ^ (*buf)[i] );

	if( (*buf)[r-1] != checksum ) {
		if( 1 )
			fprintf( stderr, "Incorrect checksum reading register %hhu\n", reg );
		printf( "Checksums: received = 0x%2.2hhx, calculated = 0x%2.2hhx\n",
			(*buf)[len+2],
			checksum );
		/* Checksum's incorrect */
		goto error1;
	}

	if( (*buf)[1] != reg ) {
		fprintf( stderr, "Incorrect register read %hhu\n", reg );

		/* Incorrect command read back */
		goto error1;
	}

	/* TODO: Remove this horrible hack */
	memmove( (*buf) + 1, (*buf) + 2, len );

	if( 1 )	{
		uint8_t i;
		printf( "Read %i bytes from register %hhu:\n", len, reg );
		for( i=0; i<len+2; i++ )
			printf( "%hhX: %hhX\n", i, (*buf)[i] );
	}

	pecon(fd);

	return len+1;

error1:
	free( *buf );
	*buf = NULL;

error0:
	pecon(fd);
	return -1;
}


int init_i2c(void){
	int fd;			
      
	fd = open( "/dev/i2c-1", O_RDWR );
	
	if( fd == -1 )
	{
		fprintf( stderr, "Failed to open /dev/`i2c-1: %m\n" );
		return 1;
	}

	

	if( ioctl( fd, I2C_SLAVE, ADDRESS ) < 0 )
	{
		fprintf( stderr, "Failed to set slave address: %m\n" );
		return 2;
	}

	

	pecon(fd);
/* 	if( ioctl( fd, I2C_PEC, 1) < 0) */
/* 	{ */
/* 		fprintf( stderr, "Failed to enable PEC\n"); */
/* 		return 3; */
/* 	} */

	return fd;
}


int pecon(int fd){

	if( ioctl( fd, I2C_PEC, 1) < 0)
	{
		fprintf( stderr, "Failed to enable PEC\n");
		return 3;
	}
	return;
}

int pecoff(int fd){
	if( ioctl( fd, I2C_PEC, 0) < 0) /* need to disable pec so can do 2 actions consecutively - see gumsense readme */
	{
		fprintf( stderr, "Failed to disnable PEC\n");
		return 3;
	}
}




/* len = i2c_smbus_read_byte_data( gum->fd, reg ); */





/* 	//------------------------------ */
/* 	if (argc<2){ // check at least 1 arg */
/* 		printf("incorrect args\n"); */
/* 		//printf("Usage: %s {w,l,v,i,r,s}\n"); */
/* 		printf("w - identify(currently unsupported locally)\n" */
/* 		       "l - set led values, needs 2 args\n" */
/* 		       "v - read voltage\n" */
/* 		       "i - read current\n" */
/* 		       "r - get rail values\n" */
/* 		       "s - set rail values - be REALLY CAREFULL! ( needs 2 args)\n" */
/* 		       "d - get dip switch values\n" */
/* 		       "u - check for usb cabble plugged in\n" */
/* 		       "o - send char out ( needst args)\n"); */
/* 		return -1; */
/* 	} */

	
/* 	switch( *argv[1]){ */
/* 	case 'w': */
/* 		//printf("Read ID as %x\n", readbyte(fd, 0)); */
/* 		printf("sorry you cant because it sucks\n"); */
/* 		return -1; */
/* 		break; */
/* 	case 'l': */
/* 		printf("Leds\n"); */
/* 		if (argc!=3) */
/* 		{ */
/* 			printf("l takes another argument for the value\n"); */
/* 			return -1; */
/* 		} */
/* 		printf("cmd:%d\n",LED); */
/* 		printf("%d\n",setpins(fd,LED,atoi(argv[2]))); */
/* 		return 0; */
/* /\* 	case 'v': *\/ */
/* /\* 		printf("%d\n", readword(fd, GETV)); *\/ */
/* /\* 		return 0; *\/ */
/* /\* 	case 'i': *\/ */
/* /\* 		printf("%d\n", readword(fd, GETI)); *\/ */
/* /\* 		return 0; *\/ */
/* /\* 	case 's': *\/ */
/* /\* 		if (argc!=3) *\/ */
/* /\* 		{ *\/ */
/* /\* 			printf("l takes another argument for the value\n"); *\/ */
/* /\* 			return -1; *\/ */
/* /\* 		} *\/ */
/* /\* 		printf("%d\n", setpins(fd, SETRAILS,atoi(argv[2]))); *\/ */
/* /\* 		return 0; *\/ */
/* /\* 	case 'r': *\/ */
/* /\* 		printf("%d\n", readbyte(fd, GETRAILS)); *\/ */
/* /\* 		return 0; *\/ */

/* /\* 	case 'x': *\/ */
/* /\* 		printf("%d\n", readword(fd, GETRAILS)); *\/ */
/* /\* 		return 0; *\/ */

/* /\* 	case 'o': *\/ */
/* /\* 		if (argc!=3) *\/ */
/* /\* 		{ *\/ */
/* /\* 			printf("l takes another argument for the value\n"); *\/ */
/* /\* 			return -1; *\/ */
/* /\* 		} *\/ */
/* /\* 		printf("%d\n", setpins(fd,SENDSER,*argv[2])); *\/ */
/* /\* 		return 0; *\/ */
/* 	case 'd': */
/* 		printf("%d\n", readbyte(fd, DIPSWITCH)); */
/* 		return 0; */

/* /\* 	case 'u': *\/ */
/* /\* 		printf("%d\n", readbyte(fd, ISUSB)); *\/ */
/* /\* 		return 0; *\/ */
/* /\* 	case 'q': *\/ */
/* /\* 		printf("break everything?\n"); *\/ */
/* /\* 		printf("%d\n", readbyte(fd, 93)); *\/ */
/* /\* 		break; *\/ */

/* 	case 'e': */
/* 		printf("Ah Ah Ah Ah, stayin' alive!\n"); */
/* 		printf("%d\n",setpins(fd,BEEGEES,1)); */
/* 		//sends alive packet! */
/* 		break; */
		
/* 	default: */
/* 		printf("not a recognised command go think again\n"); */
/* 	} */

/* 	return -1; */


/* } */



/* uint16_t readword(int fd, uint8_t cmd) { */
/* 	signed int tmp; */
/* 	tmp = i2c_smbus_read_word_data( fd, cmd ); */
/* 	if(tmp == -1){ */
/* 		printf("Error reading byte.\n"); */
/* 		printf("Got 0x%02x.\n"); */
/* 		return 0; */
/* 	} */

/* 	return tmp; */
/* } */



/* uint8_t readbyte(int fd, uint8_t cmd) { */
/* 	signed int tmp; */
/* 	tmp = i2c_smbus_read_byte_data( fd, cmd ); */
/* 	if(tmp == -1){ */
/* 		printf("Error reading byte.\n"); */
/* 		printf("Got 0x%02x.\n"); */
/* 		return 0; */
/* 	} */

/* 	return 0xFF & tmp; */
/* } */

/* char setpins( int fd, uint8_t command, uint8_t val ) */
/* { */
/* 	uint16_t p; */
	
/* 	printf("command: %d,val: %d",command,val); */

/* 	if( i2c_smbus_write_byte_data( fd, command , val ) < 0 && err_enable ) */
/* 	{ */
/* 		fprintf( stderr, "i2c failed: %m\n" ); */
/* 		return 1; */
/* 	} */
/* 	return 0; */
/* } */
