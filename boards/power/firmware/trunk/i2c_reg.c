#include "i2c_reg.h"
//#include "time.h"
//#include "schedule.h"
#include "types.h"
//#include "timer-b.h"
//#include "power.h"
//#include "job.h"
//#include "gum-watchdog.h"
#include <string.h>
#include <signal.h>

/* Register functions:
 * i2ct: The type of the register.
 * i2cr: Read from the register.
 * i2cw: Write to the register.
 * i2cs: The size of the register. */

/* Read only register */

/* Read only register */
#define I2C_REG_RO( NAME ) \
	uint8_t i2cr_ ## NAME ( uint8_t *data );	\
	uint16_t i2cs_ ## NAME ( void )

/* Write only register */
#define I2C_REG_WO( NAME ) \
	uint16_t i2cs_ ## NAME ( void ); \
	void i2cw_ ## NAME ( uint8_t* data, uint8_t len )

/* Read/write register */
#define I2C_REG( NAME ) \
	I2C_REG_RO( NAME ); \
	void i2cw_ ## NAME ( uint8_t* data, uint8_t len )

#define I2C_REG_ENTRY_RO( NAME )	\
	{ \
		.size = i2cs_ ## NAME, \
		.read = i2cr_ ## NAME,	\
		.write = NULL,	\
	}

#define I2C_REG_ENTRY_WO( NAME )	\
	{ \
		.size = i2cs_ ## NAME, \
		.read = NULL,	\
		.write = i2cw_ ## NAME,	\
	}

#define I2C_REG_ENTRY( NAME )	\
	{ \
		.size = i2cs_ ## NAME, \
		.read = i2cr_ ## NAME,	\
		.write = i2cw_ ## NAME, \
	}

/* Register functions */
I2C_REG_RO( identitify );		/* 0: Identity */
I2C_REG( time );		/* 1: Time read/write*/
I2C_REG( job );			/* 2: Job table read/write */
I2C_REG_RO( job_data_size );	/* 3: Number of readings available */
I2C_REG( job_data );		/* 4: Job data */
I2C_REG_WO( gum_power );	/* 5: Turn the gumstix off */
I2C_REG( gum_wake );		/* 6: Time to turn the gumstix on */
I2C_REG( job_exec_mask );	/* 7: Mask job execution */
I2C_REG( job_read_cursor );	/* 8: Position of the job data read cursor */
I2C_REG_RO( job_data_size_time ); /* 9: Number of readings, and time of last reading */
I2C_REG( job_table_entry );	  /* 10: The job table offset that's being written to. */
I2C_REG( power_control );	  /* 11: Control a power line */
I2C_REG( io_control );		  /* 12: Control an IO line */
/* When adding new commands, make sure you change I2C_NUM_COMMANDS */

const reg_access_t dev_regs[] = 
{
	I2C_REG_ENTRY_RO( identify ), /* Identity */
	I2C_REG_ENTRY( time ),	      /* Time */
	I2C_REG_ENTRY( job ),	      /* Job */
	I2C_REG_ENTRY_RO( job_data_size ),
	I2C_REG_ENTRY( job_data ),  /* Job Data */
	I2C_REG_ENTRY_WO( gum_power ),	   /* Gumstix Power */
	I2C_REG_ENTRY( gum_wake ),	   /* Gumstix Wake-up Time */
	I2C_REG_ENTRY( job_exec_mask ),   /* Mask job execution */
	I2C_REG_ENTRY( job_read_cursor ),   /* Position of the job data read cursor */
	I2C_REG_ENTRY_RO( job_data_size_time ),
	I2C_REG_ENTRY( job_table_entry ),
	I2C_REG_ENTRY( power_control ),
	I2C_REG_ENTRY( io_control )
};


uint16_t i2cs_identify( void )
{
	return 2;
}

uint8_t i2cr_identify( uint8_t *data )
{
	data[0] = 0x50;
	data[1] = 0x43;
	return 2;
}

uint16_t i2cs_time( void )
{
	return sizeof( time_t );
}

uint8_t i2cr_time( uint8_t *data )
{
	time_t t = time_get_safe();
	uint8_t i;

	for( i=0; i<4; i++ )
	{
		data[i] = t & 0xff;
		t >>= 8;
	}

	return 4;
}

void i2cw_time( uint8_t* data, uint8_t len )
{
	time_t t = 0;
	uint8_t i;

	/* Time is sent LSB first */
	for( i=4; i>0; i-- ) 
	{
		t <<= 8;
		t |= data[i-1];
	}

	time_set( t );
}

uint16_t i2cs_job_data( void )
{
	/* We only allow a maximum of MAX_BLOCK_SIZE bytes to be read at a time */
	if( read_len - read_cursor < (MAX_BLOCK_SIZE/2) )
		return (read_len - read_cursor) * 2;
	else
		return MAX_BLOCK_SIZE;
}

uint8_t i2cr_job_data( uint8_t* data )
{
	uint8_t i;
	uint16_t n;
	uint8_t num_sent = 0;

	/* Translate into buffer offset */
	n = (read_cursor + job_data_tail) % NUM_READINGS;

	for( i=0; i < (MAX_BLOCK_SIZE/2); i++ )
	{
		data[i*2] = job_data[n] & 0xFF;
		data[i*2 + 1] = (job_data[n] >>8) & 0xFF;

		num_sent++;
		n = (n+1) % NUM_READINGS;

		/* Stop if there's no more data */
		if( n == job_data_tail )
			break;

		/* Stop if we're reached the position we told them about */
		if( n == read_end_offset )
			break;
	}
	
	if( i == (MAX_BLOCK_SIZE/2) )
		/* Automatically advance to the next block */
		read_cursor += (MAX_BLOCK_SIZE/2);
		

	return num_sent * 2;
}

void i2cw_job_data( uint8_t* data, uint8_t len )
{
	uint16_t pass = 0;
	/* When this register is written to, all data gets cleared */

	pass = data[0] | ( ((uint16_t)data[1]) << 8 );

	if( pass == GUM_DATA_CLEAR_PASSWORD )
	{
		/* Advance the tail */
		job_data_tail = read_end_offset;
		job_data_len -= read_len;
	}
}

uint16_t i2cs_gum_power( void )
{
	return 2;
}

void i2cw_gum_power( uint8_t* data, uint8_t len )
{
	uint16_t password;

	/* LSB first */
	password = data[0] | (((uint16_t)data[1]) << 8);

	if( password == GUM_POWER_PASSWORD ) {
		dint();
		gum_watchdog_power_down();
		power_gum_off();
		eint();
	}
}

uint16_t i2cs_gum_wake( void )
{
	return sizeof(time_t);
}

void i2cw_gum_wake( uint8_t* data, uint8_t len )
{
	time_t t = 0;
	uint8_t i;

	for( i=4; i>0; i-- )
	{
		t <<= 8;
		t |= data[i-1];
	}

	schedule_gumstix_wakeup_set( t );
}

uint8_t i2cr_gum_wake( uint8_t *data )
{
	time_t t;
	uint8_t i;
	t = schedule_gumstix_wakeup_get();

	for( i=0; i<4; i++ ) {
		data[i] = t & 0xff;
		t >>= 8;
	}

	return 4;
}

uint16_t i2cs_job_data_size( void )
{
	return 2;
}

uint8_t i2cr_job_data_size( uint8_t *data )
{
	/* We save the current length here */
	read_len = job_data_len;
	/* We save the offset of the end here */
	read_end_offset = (job_data_tail + read_len) % NUM_READINGS;

	data[0] = job_data_len & 0xFF;
	data[1] = (job_data_len >> 8) & 0xFF;

	return 2;
}

uint16_t i2cs_job_exec_mask( void )
{
	return 1;
}

uint8_t i2cr_job_exec_mask( uint8_t *data )
{
	if( schedule_jobs_enabled() )
		data[0] = 1;
	else
		data[0] = 0;

	return 1;
}

void i2cw_job_exec_mask( uint8_t *data, uint8_t len )
{
	if( data[0] )
		schedule_jobs_enable();
	else
		schedule_jobs_disable();
}

uint16_t i2cs_job_read_cursor( void )
{
	return 2;
}

uint8_t i2cr_job_read_cursor( uint8_t *data )
{
	uint16_t cursor;

	cursor = read_cursor / (MAX_BLOCK_SIZE/2);

	/* Read cursor {does,must}n't change in interrupts */
	data[0] = cursor & 0xFF;
	data[1] = (cursor >> 8) & 0xFF;

	return 2;
}

/* One writes the desired block number to this i2c register.
   The global variable read_cursor, however, contains the number of 
   the first reading of the requested block */
void i2cw_job_read_cursor( uint8_t *data, uint8_t len )
{
	uint16_t cursor;

	cursor = data[0] | (((uint16_t)data[1]) << 8);
	read_cursor = cursor * (MAX_BLOCK_SIZE/2);
}

uint16_t i2cs_job_data_size_time( void )
{
	return 6;
}

uint8_t i2cr_job_data_size_time( uint8_t *data )
{
	time_t latest;
	uint8_t i;

	/* 0-1: number of readings
	   2-5: last sample time */

	read_len = job_data_len;
	read_end_offset = (job_data_tail + read_len) % NUM_READINGS;
	latest = job_last_exec_time;

	data[0] = read_len & 0xFF;
	data[1] = (read_len >> 8) & 0xFF;

	for( i=0; i<4; i++ ) 
	{
		data[2+i] = latest & 0xff;
		latest >>= 8;
	}

	return 6;
}

uint16_t i2cs_job_table_entry( void )
{
	return 1;
}

uint8_t i2cr_job_table_entry( uint8_t* data )
{
	if( data[0] < SCHEDULER_NUM_JOBS )
		data[0] = i2c_job_entry;
	return 1;
}

void i2cw_job_table_entry( uint8_t* data, uint8_t len )
{
	i2c_job_entry = data[0];
}

uint16_t i2cs_power_control( void )
{
	return 1;
}

uint8_t i2cr_power_control( uint8_t* data )
{
	uint8_t i;
	/* Format:
	   Bits 0-3: Power rail states.

	   Bit 0: PCON4
	       1: PCON5
	       2: PCON6
	       3: PCON7 */

	data[0] = 0;
	for( i=7; i>3; i-- )
	{
		data[0] <<= 1;
		if( power_switched_get(i) )
			data[0] |= 0x10;
	}

	return 1;
}

void i2cw_power_control( uint8_t* data, uint8_t len )
{
	uint8_t pnum;

	/* Format:
	   Bits 3-1: Power rail number (4-7, 0-3 unused).
	   Bit 0: Power rail state (1=ON). */

	pnum = (data[0] >> 1) & 0x07;

	/* Rails 0-3 don't exist */
	/* They're numbered this way to be consistent with the net names
	   on the schematic */
	if( pnum < 4 )
		return;

	if( data[0] & 1 )
		power_switched_on( pnum );
	else
		power_switched_off( pnum );
}

uint16_t i2cs_io_control( void )
{
	return 1;
}

uint8_t i2cr_io_control( uint8_t* data )
{
	uint8_t i, mask;

	/* Format:
	   0: The state of the pins. (bitmask)
	   1: The direction of the pins (bitmask) (In=0, Out=1) */

	data[0] = 0;
	for( i=0, mask=1; i<8; i++, mask<<=1 )
	{
		if( P2DIR & mask )
			data[0] |= P2OUT & mask;
		else
			data[0] |= P2IN & mask;
	}
	data[1] = P2DIR;

	return 2;
}

void i2cw_io_control( uint8_t* data, uint8_t len )
{
	uint8_t ion;
	/* Format:
	   Bits 4-2: IO number (0-7)
	   Bit 1: Direction (1=Output, 0=Input)
	   Bit 0: Output value (ignored if input) */

	ion = data[0] >> 2;

	if( data[0] & 2 )
	{
		P2OUT = (P2OUT & ~(1<<ion)) | ((data[0] & 1) << ion);
		P2DIR |= 1 << ion;
	}
	else
		P2DIR &= ~(1 << ion);
}
