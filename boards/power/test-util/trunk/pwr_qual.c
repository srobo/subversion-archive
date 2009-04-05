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
#include <assert.h>
#include "pwr_qual.h"
//#include "smbus_pec.h"
#define POLY    (0x1070U << 3)

#define BUFLEN 30

int init_i2c(void);

/* Read from register.
 * Arguments:
 *  -  fd: The file descriptor
 *  - reg: The register number to read.
 *  - buf: Pointer to buffer to read data into.  Must be BUFLEN long.
 * Returns: The length of data read. */
int32_t sr_read( int fd, uint8_t reg, uint8_t *buf );

int32_t sr_write( int fd, uint8_t command, uint8_t len, uint8_t *buf );

int pecoff(int fd);

int pecon(int fd);

uint8_t crc8( uint8_t tempdata );

/* Read the XBee reset pin state. 
 * Args:
 *  -  fd: The I2C file descriptor.
 * Returns: 0 if the XBee is held in reset, 1 otherwise.
 *          Negative values on error. */
int8_t power_board_xbee_reset_get( int fd );

/* Set the XBee reset pin state.
 * Args:
 *  -  fd: The I2C file descriptor.
 *  - val: Reset pin state.  non-zero to assert reset pin.
 * Returns 0 when successful. */
int8_t power_board_xbee_reset_set( int fd, uint8_t val );

static uint8_t value[BUFLEN];

int main( int argc, char** argv )
{
	int fd =0;
	int retval;
	uint8_t temp;

	uint8_t *buf = value;

  	fd = init_i2c();

	if (argc<2){ // check at least 1 arg
		printf("incorrect args\n");
		printf("Usage:\n");
		printf("i - identify(currently unsupported locally)\n"
		       "l - set led values, needs 2 args\n"
		       "d - get dip switch values\n"
		       "b - check for button status\n"
		       "s - get/set slug power add val arg to set\n" 
		       "p - get/set servo power\n"
		       "m - get/set motor power\n"
		       "c - get battery charge status\n"
		       "v - read voltage\n"
		       "a - read current\n"
		       "e - Stay alive packet\n"
		       "f - Fake pressing the button\n"
		       "r - set/get RTS on xbee\n"
		       "g - get CTS from Xbee\n"
		       "x - Xbee reset control\n");
		       
		return -1;
	}


	switch( *argv[1]){
	case 'i':
		retval = sr_read(fd, IDENTIFY , value);
		printf("Read ID as %x %x\n", value[1],value[0]);
		break;
	case 'l':
		if (argc == 2 )
		{ 
			retval = sr_read(fd, LED , value);
			printf("Leds = %d \n",value[0]);
		     
		}
		else if (argc == 3){
			value[0]= atoi(argv[2]);
			sr_write(fd,LED,1,value);
		
		}
		else{
			printf("Usage:\n "
			       "show LED status: pwr_qual l"
			       "set led: pwr_qual l <N>\n");
			return -1;
		}			
		
		break;


	case 'd':
		retval = sr_read(fd, DIPSWITCH , value);
		printf("Read dips as %d \n",value[0]);
		break;

	case 'b':
		retval = sr_read(fd, BUTTON , value);
		printf("Button history is %d\n", value[0]);
		break;

       	case 's':
		if (argc == 2 )
		{
			retval = sr_read(fd, SLUG_POWER , value);
			printf("Slug power = %d \n",value[0]);
		     
		}
		else if (argc == 3){
			if (atoi(argv[2])>0){
				value[0] = 1;
			}
			else{
				value[0] =0 ;
			}

			sr_write(fd,SLUG_POWER,1,value);
		}
		else{
			printf("Usage:\n "
			       "show rail status: pwr_qual s"
			       "set rail 0/1: pwr_qual s {0,1}\n");
			return -1;
		}			
		
		break;

       	case 'p':
		if (argc == 2 )
		{
			retval = sr_read(fd, SERVO_POWER , value);
			printf("retva: %d\n",retval);
			printf("Servo power = %d \n",value[0]);
		     
		}
		else if (argc == 3){
			if (atoi(argv[2])>0){
				value[0] = 1;
			}
			else{
				value[0] =0 ;
			}

			sr_write(fd,SERVO_POWER,1,value);
		}
		else{
			printf("Usage:\n "
			       "show rail status: pwr_qual p"
			       "set rail 0/1: pwr_qual p {0,1}\n");
			return -1;
		}
		break;

		
       	case 'm':
		if (argc == 2 )
		{
			retval = sr_read(fd, MOTOR_POWER , value);
			printf("Motor power = %d \n",value[0]);
		     
		}
		else if (argc == 3){
			if (atoi(argv[2])>0){
				value[0] = 1;
			}
			else{
				value[0] =0 ;
			}

			sr_write(fd,MOTOR_POWER,1,value);
		}
		else{
			printf("Usage:\n "
			       "show rail status: pwr_qual m"
			       "set rail 0/1: pwr_qual m {0,1}\n");
			return -1;
		}
		break;
		    
   	case 'c':
		retval = sr_read(fd, BATTERY , value);
		printf("Battery status: %d\n",value[0]);
		break;	



	case 'v':
		retval = sr_read(fd, VOLT , value);
		printf("%d\n", ((value[3]<<8) | value[0]));
		break;
	case 'a':
		retval = sr_read(fd, AMP , value);
		printf("%d\n", ((value[3]<<8) | value[0]));
		break;


	case 'e':
		if (argc == 2 )
		{
			retval = sr_read(fd, BEEGEES , value);
			printf("slug alive? 0 = timer,1= safe: %d \n",value[0]);
		     
		}
		else if (argc == 3){
			sr_write(fd,BEEGEES,1,value);
			retval = sr_read(fd, BEEGEES , value);
			if (value[0])
				printf("Ah Ah Ah Ah, stayin' alive!\n");
			else
				printf("Failed to disable timer");
		}
		else{
			printf("Usage:\n "
			       "get alive stat: pwr_qual e\n"
			       "     0= timer active,1=timer neutered\n"
			       " pwr_qual e 1     # any arg sets timer off\n");
			return -1;
		}
		break;




		printf("Ah Ah Ah Ah, stayin' alive!\n");
		retval = sr_read(fd, BEEGEES , value);
		//sends alive packet!
		break;

	case 't':
		if (argc == 2 )
		{ 
			retval = sr_read(fd, TEST , value);
			printf("test = %d %d\n",value[1],value[0]);
		     
		}
		else if (argc == 4){
			value[0]= atoi(argv[2]);
			value[1]= atoi(argv[3]);
			sr_write(fd,TEST,2,value);
		
		}
		else{
			printf("Usage:\n "
			       " pwr_qual t"
			       "set test : pwr_qual t <N>\n");
			return -1;
		}			
		
		break;	

	case 'f':
		sr_write(fd,BUTTON_FAKE,1,value);
		break;


	case 'r':
		if (argc == 2 )
		{
			retval = sr_read(fd, RTS , value);
			printf("RTS: %d \n",value[0]);
		     
		}
		else if (argc == 3){
			if (atoi(argv[2])>0){
				value[0] = 1;
			}
			else{
				value[0] =0 ;
			}

			temp = value[0];
			sr_write(fd,RTS,1,value);
			retval = sr_read(fd, RTS , value);
			if (value[2] == temp)
				printf("written ok");
			else
			{
				printf("Failed write");
				return -1;
			}
		}
		else{
			printf("Usage:\n "
			       "get RTS stat: pwr_qual r\n"
			       " pwr_qual r {0,1}  \n");
			return -1;
		}
		break;


	case 'g':
		if (argc == 2 )
		{
			retval = sr_read(fd, CTS , value);
			printf(" CTS %d \n",value[0]);
		     
		}
		else{
			printf("Usage:\n "
			       "CTS stat: pwr_qual g\n"
			       " pwr_qual g {0,1} \n");
			return -1;
		}
		break;


	case 'x':
		if (argc == 2 ) {
			/* Read the XBee reset status */			
			if( power_board_xbee_reset_get( fd ) == 0 )
				printf( "XBee held in reset.\n" );
			else
				printf( "XBee reset pin not asserted.\n" );
		} else if (argc == 3)
			/* Set the XBee state. */
			power_board_xbee_reset_set( fd, atoi(argv[2]) );
		else {
			printf("Usage:\n "
			       "\tGet XBee reset status: pwr_qual x\n"
			       "\tSet XBee reset pin: pwr_qual x N"
			       "\t\tWhere N is zero to assert the reset pin." );
			return -1;
		}
		break;

	default:
		printf("Sorry not recognised command try no args for usage\n");

		
	}
	return 0;

}


int32_t sr_write( int fd, uint8_t command, uint8_t len, uint8_t *buf ){
	int retval=0;
	retval = i2c_smbus_write_block_data(fd,command,len,buf);
	if (retval != 0)
	{
		fprintf( stderr, "Block Write failed:  %m\n" );
	}
	return retval;
}


int32_t sr_read( int fd, uint8_t reg, uint8_t *buf )
{
	/* The Origins of this code are stolen from Robert Spanton's Gumsense firmware forge.ecs.soton.ac.uk */
	int len, r;
	uint8_t checksum, i;

	pecoff(fd); /* need to disable pec so can do 2 actions consecutively - see gumsense readme */

	
	/* We have to hack around the fact that the i2c adapter doesn't
	   support i2c block read. */
	/*  First, do a read to get the length byte. */
	/* This read byte operation also sets the command that we're doing
	   within the powerboard */

	/* Set the command and grab the length */
	len = i2c_smbus_read_byte_data( fd, reg );

	if( len < 0 ) {
		fprintf( stderr, "Failed to read register %hhu length\n", reg );
		fprintf( stderr, "length is %d", len );
		goto error0;
	}

	/* Prevent buffer overflows */
	assert( (len+3) < BUFLEN );

	r = read(fd, buf, len + 3 );
	
	if( r < 0 ) {
		fprintf( stderr, "Failed to read register %hhu\n", reg );
		goto error0;
	}

	if( r != len + 3 ) {
		fprintf( stderr, "Failed to read all of register %hhu\n", reg );
		goto error0;
	}

	/* Generate the checksum: */
	checksum = crc8( (ADDRESS<<1) | 1 );
	for( i=0; i<len+2; i++ )
		checksum = crc8( checksum ^ (buf)[i] );

	if( (buf)[r-1] != checksum ) {
		if( 1 )
			fprintf( stderr, "Incorrect checksum reading register %hhu\n", reg );
		printf( "Checksums: received = 0x%2.2hhx, calculated = 0x%2.2hhx\n",
			(buf)[len+2],
			checksum );
		/* Checksum's incorrect */
		goto error0;
	}

	if( (buf)[1] != reg ) {
		fprintf( stderr, "Incorrect register read %hhu\n", reg );

		/* Incorrect command read back */
		goto error0;
	}

	if( 0 )	{		/* read buffer debug - set to 1 to gat buffer printout */
		uint8_t i;
		printf( "Read %i bytes from register %hhu:\n", len, reg );
		for( i=0; i<len+2; i++ )
			printf( "%hhX: %hhX\n", i, (buf)[i] );
	}

	pecon(fd);

	/* Copy the data to the beginning of the buffer. */
	memmove( buf, buf + 2, len );
	return len;

error0:
	pecon(fd);
	return -1;
}


int init_i2c(void){
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

       
	pecon(fd);

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

int8_t power_board_xbee_reset_get( int fd )
{
	int r = sr_read( fd, XBEE, value );
	assert( r == 1 );

	return value[0];
}

int8_t power_board_xbee_reset_set( int fd, uint8_t val )
{
	int8_t r;

	/* Clean up val to be 1 if non-zero */
	if( val )
		val = 1;
	do
	{
		value[0] = val;
		while( sr_write( fd, XBEE, 1, value) < 0 );

		while( (r=power_board_xbee_reset_get(fd)) < 0 );
	}
	while( r != val );

	return 0;	
}
