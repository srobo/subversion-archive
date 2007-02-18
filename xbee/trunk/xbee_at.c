/* xbee AT related functions */
#define _GNU_SOURCE 		/* For TEMP_FAILURE_RETRY */

#include <unistd.h>
#include <sys/select.h>
#include <stdint.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <glib.h>
#include "xbee_at.h"

int timeval_subtract (struct timeval *result, 
		      struct timeval x, 
		      struct timeval y);

gboolean real_sleep( const struct timeval *t );

/* Waits for an "OK" - discarding other input */
static gboolean xbee_wait_ok( xbee_t* xb );

/* Waits for an "OK" - error on "ERROR" */
static gboolean xbee_check_ok( xbee_t* xb );

/* Finds expired time */
static gboolean time_expired( struct timeval* start,
			  struct timeval* result );

/* WARNING: If data arrives at the xbee whilst the device is waiting to 
   enter AT mode, this function will _probably_ return FALSE.  */
gboolean xbee_at_mode( xbee_t* xb )
{
	const struct timeval guard_time = { .tv_sec = 1, .tv_usec = 100000 },
		gt_low = { .tv_sec = 1, .tv_usec = 0 };
	assert( xb != NULL );

	/* Check that we're not already in AT mode */
	if( xbee_get_at_mode( xb ) )
		return TRUE;

	/* Wait one guard time */
	if( ! real_sleep( &guard_time ) ) 
	{
		fprintf( stderr, "Failed to sleep for guard time\n" );
		return FALSE;
	}

	/* Send the sequence */
	if( ! xbee_puts( xb, "+++" ) ) 
	{
		fprintf( stderr, "Error: Failed to write AT mode switching command: %m\n" );
		return FALSE;
	}

	/* Wait a shorter guard time */
	if( ! real_sleep ( &gt_low ) )
	{
		fprintf( stderr, "Error: Failed to sleep for guard time\n" );
		return FALSE;
	}

/* 	/\* Flush the input buffer *\/ */
/* 	if( tcflush( xb->fd, TCIFLUSH ) < 0 ) */
/* 	{ */
/* 		fprintf( stderr, "Error: Failed to flush input buffer: %m\n" ); */
/* 		return FALSE; */
/* 	} */

	/*** Read the 'OK' ***/
	if( !xbee_wait_ok( xb ) )
	{
		fprintf( stderr, "Error: Failed to receive OK after moving into AT mode\n" ); 
		return FALSE;
	}
	
	g_debug( "In AT mode." );
	
	/* We're now in AT mode - set the time */
	if( gettimeofday( &xb->at_time, NULL  ) == -1 )
	{
		fprintf( stderr, "Error: Failed to get time: %m\n" );
		return FALSE;
	}

	xb->at_mode = TRUE;
	
	return TRUE;
}

gboolean xbee_get_at_mode( xbee_t* xb )
{
	struct timeval now, res;
	/* AT mode expires after 10 seconds - 9.5 for safety */
	const struct timeval c = { .tv_sec = 9, .tv_usec = 500000 };

	assert( xb != NULL );

	if( ! xb->at_mode )
		return FALSE;

	if( gettimeofday( &now, NULL ) == -1 )
	{
		fprintf( stderr, "Failed to get current time: %m\n" );
		return FALSE;
	}

	/* Difference now and then */
	timeval_subtract( &res, now, xb->at_time );
	if( timeval_subtract( NULL, c, res ) )
	{
		return FALSE;
	}
	else
		return TRUE;
}

/* Routine taken from glibc documentation */
/* Subtract the `struct timeval' values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0.  */
int
timeval_subtract (result, x, y)
struct timeval *result, x, y;
{
	/* Perform the carry for the later subtraction by updating Y. */
	if (x.tv_usec < y.tv_usec) {
		int nsec = (y.tv_usec - x.tv_usec) / 1000000 + 1;
		y.tv_usec -= 1000000 * nsec;
		y.tv_sec += nsec;
	}
	if (x.tv_usec - y.tv_usec > 1000000) {
		int nsec = (x.tv_usec - y.tv_usec) / 1000000;
		y.tv_usec += 1000000 * nsec;
		y.tv_sec -= nsec;
	}

	/* Compute the time remaining to wait.
	   `tv_usec' is certainly positive. */
	if( result != NULL )
	{
		result->tv_sec = x.tv_sec - y.tv_sec;
		result->tv_usec = x.tv_usec - y.tv_usec;
	}

	/* Return 1 if result is negative. */
	return x.tv_sec < y.tv_sec;
}

/* real_sleep
 * Actually sleeps for the specified amount of time.
 *  - t: The time to sleep.
 */
gboolean real_sleep( const struct timeval *t )
{
	struct timeval start, now, rem = { .tv_sec = 0, .tv_usec = 0 };

	if( gettimeofday( &start, NULL ) == -1 ) {
		fprintf( stderr, "Error: failed to get time: %m\n" );
		return FALSE;
	}

	while( ! timeval_subtract( &rem, *t, rem ) 
	       && rem.tv_sec > 0 && rem.tv_usec > 0 )
	{
		select( 0, NULL, NULL, NULL, &rem );

		if( gettimeofday( &now, NULL ) == -1 )
		{
			fprintf( stderr, "Error: Failed to get time: %m\n" );
			return FALSE;
		}

		/* Calculate remaining time */
		if( timeval_subtract( &rem, now, start ) )
		{
			fprintf( stderr, "Something's gone wrong with time.\n" );
			return FALSE;
		}
	}

	return TRUE;
}

static gboolean xbee_wait_ok( xbee_t* xb )
{
	uint8_t t, buf[2] = {0,0};
	fd_set s;
	int r;
	struct timeval start, trem, exp = {0,0};
	const struct timeval ok_wait = { .tv_sec = 10, .tv_usec = 0 };

	assert( xb != NULL );

	/* Get the time at which we started waiting */
	if( gettimeofday( &start, NULL ) )
	{
		fprintf( stderr, "Error: Failed to get time: %m\n" );
		return FALSE;
	}

	trem = ok_wait;

	while( !timeval_subtract( &trem, trem, exp ) &&
	       !(buf[0] == 'O' && buf[1] == 'K') )
	{
		FD_ZERO(&s);
		FD_SET( xb->fd, &s );

		switch( TEMP_FAILURE_RETRY(select( FD_SETSIZE, &s, NULL, NULL, &trem )) )
		{
		case 1:
			/* Data ready */
			r = read( xb->fd, &t, 1 );

			switch (r)
			{
			case 1:
				buf[0] = buf[1];
				buf[1] = (uint8_t)t;
				break;

			case 0:
				break;

			case -1:
				fprintf( stderr, "read returned %i: %m\n", r );
				return FALSE;
			}

			break;
		case -1:
			/* Error */
			fprintf( stderr, "Error: Select failed: %m\n" );
			return FALSE;
		}

		if( !time_expired( &start, &exp ) )
			return FALSE;
	}

	if( buf[0] == 'O' && buf[1] == 'K' )
	{
		return TRUE;	
	}

	fprintf( stderr, "Error: Timeout waiting for OK\n" );
	return FALSE;
}

static gboolean time_expired( struct timeval* start,
			  struct timeval* result )
{
	struct timeval now;

	if( gettimeofday( &now, NULL ) )
	{
		fprintf( stderr, "Error: Failed to get time: %m\n" );
		return FALSE;
	}

	if( timeval_subtract( result, now, *start ) )
	{
		fprintf( stderr, "Something's gone wrong with time.\n" );
		return FALSE;
	}

	return TRUE;
}

static gboolean xbee_check_ok( xbee_t* xb )
{
	uint8_t d, buf[6] = {'\0','\0','\0','\0','\0','\0'};
	ssize_t i, r;
	fd_set s;
	struct timeval start, trem, exp = {0,0};
	const struct timeval ok_wait = { .tv_sec = 2, .tv_usec = 0 };

	assert( xb != NULL );

	/* Get the time at which we started waiting */
	if( gettimeofday( &start, NULL ) )
	{
		fprintf( stderr, "Error: Failed to get time: %m\n" );
		return FALSE;
	}

	trem = ok_wait;
	i = 0;

	while( //!timeval_subtract( &trem, trem, exp ) &&
	       !( i > 1 && buf[i-2] == 'O' && buf[i-1] == 'K') &&
	       !( i > 4 && strcmp( (const char*)buf, "ERROR" ) == 0 ) )
	{
		FD_ZERO(&s);
		FD_SET( xb->fd, &s );

		if( TEMP_FAILURE_RETRY(select( FD_SETSIZE, &s, NULL, NULL, &trem )) != 1 )
		{
			fprintf( stderr, "Error: Select failed: %m\n" );
			return FALSE;
		}

		/* Shift the data */
		if( i == 5 )
		{
			memmove( buf, &buf[1], 4 );
			i --;
		}

		r = read( xb->fd, &d, 1 );

		switch( r )
		{
		case 1:
/* 			printf( "Received: %c\n", d ); */
			buf[i] = d;
			i += r;
			break;

		case -1:
			fprintf( stderr, "Error: %m.\n" );
			return FALSE;
			break;

		default:
			fprintf( stderr, "Unexpected read return value\n" );
			return FALSE;
		}


		if( !time_expired( &start, &exp ) )
			return FALSE;
	}

	if( i > 1 && strncmp( (const char*)&buf[i-2], "OK", 2 ) == 0 )
		return TRUE;

	fprintf( stderr, "Error: timeout waiting for OK\n" );
	return FALSE;
}

gboolean xbee_set_api_mode( xbee_t* xb )
{
	assert( xb != NULL );

	if( xb->api_mode )
		return TRUE;

	if( !xbee_at_mode( xb ) )
		return FALSE;

	if( !xbee_puts( xb, "ATAP2\r" ) )
		return FALSE;

	/* Check that we're now in the right mode */
	if( !xbee_check_ok( xb ) )
		return FALSE;

	/* Exit AT command mode */
	if( !xbee_puts( xb, "ATCN\r" ) )
		return FALSE;

	if( !xbee_check_ok( xb ) )
		return FALSE;

	xb->api_mode = TRUE;
	g_debug("xbee in API mode\n");

	return TRUE;
}

gboolean xbee_puts( xbee_t* xb, char* buf )
{
	ssize_t n;
	assert( xb != NULL && xb->fd >= 0 && buf != NULL );

/* 	printf( "Writing: '%s'\n", buf ); */

	while( *buf )
	{
		n = write( xb->fd, buf, 1 );
		if( n < 0 )
		{
			fprintf( stderr, "Error writing to serial device: %m\n" );
			return FALSE;
		}	    
		else
			buf += n;
	}

	return TRUE;
}
