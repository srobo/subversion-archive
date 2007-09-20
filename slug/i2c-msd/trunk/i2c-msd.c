/* This program will map an image file to a device pretending to be
   a mass storage device over I2C. */
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include "i2c-dev.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "i2c-funcs.h"

/* Number of seconds to poll insanely for */
#define ACTIVE_TIMEOUT 2

/* The name of the file to map as a mass storage device */
char* image_fname;
uint8_t i2c_addr = 0x3F;

/* Display program usage */
void print_usage(const char* name)
{
	fprintf(stderr, "Usage: %s IMAGE_FILE\n"
		"\tIMAGE_FILE will be mapped as a mass storage device\n",
		name );
}

int parse_args( int argc, char** argv )
{
	if( argc < 2 )
	{
		print_usage( argv[0] );
		return -1;
	}

	image_fname = argv[1];

	return 0;
}

int i2c_open( int *fd )
{
	*fd = open( "/dev/i2c-0", O_RDWR );

	if( *fd == -1 )
	{
		fprintf( stderr, "Failed to open /dev/i2c-0: %m\n" );
		return -2;
	}

	if( ioctl( *fd, I2C_SLAVE, i2c_addr ) < 0 )
	{
		fprintf( stderr, "Failed to set slave address: %m\n" );
		return -3;
	}

	return 0;
}

int image_open( int *b_fd, uint8_t **b_buf, off_t *map_size )
{
	struct stat b_stat;
	assert( b_fd != NULL && b_buf != NULL && map_size != NULL );

	*b_fd = open( image_fname, O_RDWR );
	if( *b_fd == -1 )
	{
		fprintf( stderr, "Failed to open image file: %m\n" );
		return -1;
	}

	/* Get the file size */
	if( fstat( *b_fd, &b_stat ) == -1 )
	{
		fprintf( stderr, "Failed to get file size: m\n" );
		return -1;
	}

	*map_size = b_stat.st_size;

	if( (*map_size % 512) != 0 )
	{
		fprintf( stderr, "Error: Image must be a multiple of 512 bytes\n" );
		return -2;
	}
	
	if( *map_size == 0 )
	{
		fprintf( stderr, "Error: Image must be non-zero size\n" );
		return -3;
	}

	/* Memory map */
	*b_buf = mmap( NULL, *map_size, 
		      PROT_READ | PROT_WRITE, 
		      MAP_SHARED,
		      *b_fd,
		      0 );

	if( *b_buf == (uint8_t*)-1 )
	{
		fprintf( stderr, "Failed to memory map file: %m\n" );
		return -3;
	}

	return 0;
}

int main( int argc, char** argv )
{
	int p;		
	/* The i2c-dev fd */
	int fd;
	/* The image fd */
	int b_fd;
	off_t map_size = 0;
	uint8_t *b_buf = NULL;
	char *churn = "/|\\-";
	uint8_t cpos = 0;
	uint32_t w_count = 0, r_count = 0;
	struct timeval last_active, now;

	enum 
	{
		MODE_WAIT,	/* Wait for activity */
		MODE_ACTIVE	/* Fast poll mode */
	} mode;

	/* The poll pause */
	struct timespec pause_wait = {
		.tv_sec = 0,
		.tv_nsec = 100000000
	};

	struct timespec pause_active = {
		.tv_sec = 0,
		.tv_nsec = 100000
	};

	/* Parse args */
	p = parse_args(argc, argv);
	if( p != 0 )
		return p;

	p = i2c_open( &fd );
	if( p < 0 )
		return p;

	if( image_open( &b_fd, &b_buf, &map_size ) < 0)
		return -3;

	assert( map_size > 0 );

	printf( "%lu block device\n", map_size/512 );
	printf("\n");

	mode = MODE_WAIT;
	while(1)
	{
		uint8_t subs;
		uint32_t sector;
		msd_state_t s = msd_poll_full( fd, &subs );

/* 		if( s == MSD_ERROR ) */
/* 			return -5; */

		printf( "%c r:%u w:%u\r", churn[cpos], r_count, w_count );
		fflush(stdout);
		cpos = (cpos==3)?0:cpos+1;
		
		if( s == MSD_READ || s == MSD_WRITE ) {
			sector = msd_get_sector(fd);

			if( s == MSD_READ ) {
				msd_send( fd, b_buf + (sector * 512) + (subs * 32) );
				w_count++;
			}
			else if ( s == MSD_WRITE ) {
				msd_recv( fd, b_buf + (sector * 512) + (subs * 32) );
				r_count++;
			}

			mode = MODE_ACTIVE;
			if( gettimeofday(&last_active, NULL) < 0 ) {
				fprintf(stderr, "Failed to get time! %m\n");
				return -1;
			}
		}
		else if( mode == MODE_ACTIVE )
		{
			if( gettimeofday(&now,NULL) < 0 ) {
				fprintf(stderr, "Failed to get time! %m\n");
				return -2;
			}
			
			if( now.tv_sec - last_active.tv_sec > ACTIVE_TIMEOUT )
				mode = MODE_WAIT;
		}		

		if( mode == MODE_WAIT )
			nanosleep(&pause_wait, NULL);
		else
			nanosleep(&pause_active, NULL);
	}

	if( munmap( (void*)b_buf, map_size ) == -1 )
		fprintf( stderr, "Failed to unmap image file: %m\n" );

	close(b_fd);

	return 0;
}
