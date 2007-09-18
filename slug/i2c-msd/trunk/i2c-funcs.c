#include "i2c-funcs.h"
#include "i2c-dev.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

static uint16_t msd_get_sector_low( int fd );
static uint16_t msd_get_sector_high( int fd );
static uint16_t msd_get_sector_gen( int fd, uint8_t cmd );

msd_state_t msd_poll( int fd )
{
	return msd_poll_full( fd, NULL );
}

msd_state_t msd_poll_full( int fd, uint8_t *subsector )
{
	int32_t i;

	i = i2c_smbus_read_byte_data( fd, CMD_POLL );

	if( i < 0 ) {
		fprintf( stderr, "Failed to read poll data: %m\n" );
		return MSD_ERROR;
	}

	printf( "Read: %hX\n", (uint8_t)i );

	if( subsector != NULL )
		*subsector = (uint8_t)i & 0x1f;

	if( i & 0x80 )
		return MSD_WRITE;
	
	if( i & 0x40 )
		return MSD_READ;

	return MSD_IDLE;
}

void msd_send( int fd, uint8_t *data )
{
	int w;
	assert( data != NULL );

	/* Send the command */
	if( i2c_smbus_write_byte( fd, CMD_TX_DATA ) < 0 )
	{
		fprintf( stderr, "Failed to send subsector TX command: %m\n" );
		exit(-1);
	}

	/* At the moment, we're fudging the CRC... */
	/* TODO! */
	w = write( fd, data, 33 );

	if( w == -1 )
	{
		fprintf( stderr, "Failed to send subsector: %m\n");
		exit(-1);
	}

	if( w != 32 )
	{
		fprintf( stderr, "Failed to send subsector - only wrote %i bytes\n", w );
		exit(-2);
	}

}

void msd_recv( int fd, uint8_t *data )
{
	int r;
	assert( data != NULL );

	/* Send the command */
	if( i2c_smbus_write_byte( fd, CMD_RX_DATA ) < 0 )
	{
		fprintf( stderr, "Failed to send subsector RX command: %m\n" );
		exit(-1);
	}

	r = read( fd, data, 32 );

	if( r == -1 )
	{
		fprintf( stderr, "Failed to read subsector: %m\n");
		exit(-1);
	}

	if( r != 32 )
	{
		fprintf( stderr, "Failed to read subsector - only read %i bytes\n", r );
		exit(-1);
	}
}

/* TODO: These functions need to return on error - rather than bomb */
static uint16_t msd_get_sector_low( int fd )
{
	return msd_get_sector_gen( fd, CMD_SECTOR_LOW );
}

static uint16_t msd_get_sector_high( int fd )
{
	return msd_get_sector_gen( fd, CMD_SECTOR_HIGH );
}

static uint16_t msd_get_sector_gen( int fd, uint8_t cmd )
{
	int32_t i;

	i = i2c_smbus_read_word_data( fd, cmd );

	if( i < 0 )
	{
		fprintf( stderr, "Failed to read sector number: %m\n" );
		exit(-1);
	}

	printf( "Read: %x\n", i );

	return (uint16_t)i;
}

uint32_t msd_get_sector( int fd )
{
	return msd_get_sector_low(fd) | (((uint32_t)msd_get_sector_high(fd)) << 16);
}
