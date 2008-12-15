#include <stdio.h>
#include <stdint.h>
#include "i2c-dev.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define ADDRESS 0x2e
#define POS0 3
#define POS1 135
#define READ_COMMAND 0x02
#define RETRY_ATTEMPTS 255
#define TRY_INDEF 1

typedef enum
{
	FALSE = 0, TRUE
} bool;

typedef struct
{
	uint8_t number;
	uint8_t position;
} servo_status;

bool err_enable = TRUE;
static max_servo_no = 5;

servo_status current = {0,0};

/* Enable the use of the PEC*/
void i2c_pec_enable( int fd );

/* Disable use of the PEC */
void i2c_pec_disable( int fd );

/* Read back the last servo changed
   and its current position. */
servo_status servo_read( int fd);

void setservo( int fd, uint8_t n, uint8_t val )
{
	uint16_t p;

	p = ((uint16_t)val<<8) | n;

	if( i2c_smbus_write_word_data( fd, 1, p ) < 0 && err_enable )
      		fprintf( stderr, "i2c failed: %m\n" );
}
servo_status servo_read(int fd)
{
	int32_t r;
	servo_status stat = {255, 0};
	r = i2c_smbus_read_word_data(fd, READ_COMMAND);
	
	/* Incomming bits:
		Bits 7:0  -> Servo Number
		Bits 15:8 -> Servo position
	*/
	uint16_t d = (uint16_t) r;
	stat.number = (uint8_t) d & 0x0f;
	stat.position = (d >> 8);
	return stat;
}
void spam( int fd )
{
	uint8_t addr = 0;
	err_enable = FALSE;

	for( addr=0; addr<128; addr++ )
	{
		if( addr == ADDRESS )
			addr++;

		if( ioctl( fd, I2C_SLAVE, addr ) < 0 )
		{
			fprintf( stderr, "Failed to set slave address: %m\n" );
			break;
		}

		setservo( fd, 1, 0 );
	}
}

int main( int argc, char** argv )
{
	uint8_t d = POS0;
	uint8_t val = 0;
	uint8_t servo,i;
	int fd;

	fd = open( "/dev/i2c-0", O_RDWR );

	if( argc == 2 )
	{
		if( strcmp( argv[1], "spam" ) == 0 )
			spam(fd);
		return 0;
	}

	if( argc != 3 )
	{
		printf("Usage: i2c-test SERVO VALUE\n");
		return 1;
	}

	servo = atol( argv[1] );
	val = atol( argv[2] );	

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
	i2c_pec_enable(fd);
	uint8_t attempt = 0;
	servo_status fback = {0, 0};
	
	if(servo >= max_servo_no)
	{
		fprintf( stderr, "Invalid Servo Number. Valid Servos:  0..%u\n", max_servo_no);
		return 3;
	}
	do
	{
		setservo(fd, servo, val);
		fback = servo_read(fd);
		if(fback.position == val && fback.number == servo)
			break;
		attempt++;
	}
	while(attempt < RETRY_ATTEMPTS || TRY_INDEF);
	
	if(attempt < RETRY_ATTEMPTS)
	{
		fprintf(stderr, "Succeeded with %u retries\n", attempt);
		return 0;
	}
	else
	{
		fprintf(stderr, "Failed with %u retries\n", RETRY_ATTEMPTS);
		return 4;
	}
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

