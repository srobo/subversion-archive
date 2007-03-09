#include "xbee.h"
#include <unistd.h>
#include <sys/select.h>
#include <stdint.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include "xbee_at.h"
#include "xbee_ipc.h"

#define BUFLEN 256

void debug_show_frame( uint8_t* buf, uint16_t len );

/* Process incoming data */
static gboolean xbee_proc_incoming( xbee_t* xb );

/* Reads in available bytes from the input.
 * When a full frame is achieved, it returns 0.
 * When a full frame has not been acheived, it returns 1.
 * When an error occurs, it returns -1 */
static int xbee_read_frame( xbee_t* xb  );

/* Calculate the checksum of a block of data */
static uint8_t xbee_checksum( uint8_t* buf, uint16_t len );

static uint8_t xbee_sum_block( uint8_t* buf, uint16_t len, uint8_t cur );

/* Process outgoing data */
static gboolean xbee_proc_outgoing( xbee_t* xb );

/* Whether data's ready to transmit */
static gboolean xbee_outgoing_queued( xbee_t* xb );

/*** Transmission frame queue functions ***/
/* Adds a frame to the queue */

/* Remove a frame from the list */

void xbee_init( xbee_t* xb, int fd )
{
	assert( xb != NULL && fd >= 0 );

	xb->fd = fd;
	xb->api_mode = FALSE;
	xb->at_time.tv_sec = 0;
	xb->at_time.tv_usec = 0;

	xb->out_frames = g_queue_new();

	xb->in_len = 0;
	xb->escape = FALSE;

	xb->bytes_discarded = 0;
	xb->frames_discarded = 0;
	xb->bytes_rx = xb->bytes_tx = 0;
	xb->frames_rx = xb->frames_tx = 0;

	xb->o_chk = xb->tx_pos = 0;
	xb->checked = FALSE;
	xb->tx_escaped = FALSE;
}

gboolean xbee_main( xbee_t* xb )
{
	assert( xb != NULL );
	fd_set f_r, f_w;
	GIOChannel *gio;
	GMainLoop* ml;

	if( !xbee_set_api_mode( xb ) )
	{
		fprintf( stderr, "Failed to enter API mode - quitting.\n" );
		return FALSE;
	}

	while( 1 )
	{
		int sel;

		FD_ZERO( &f_r );
		FD_ZERO( &f_w );

		/* We always want to monitor for incoming data */
		FD_SET( xb->fd, &f_r );

		/* We only have  to do this if we've got something that needs 
		   writing */
		if( xbee_outgoing_queued(xb) )
			FD_SET( xb->fd, &f_w );

		/* Wait on serial port events */
		/* we'll also wait on IPC here later */
		sel = TEMP_FAILURE_RETRY(select( FD_SETSIZE, &f_r, &f_w, NULL, NULL ));
		
		/* Act on select return */
		if( sel == -1 )
		{
			fprintf( stderr, "Select call failed, ending xbee mainloop: %m\n" );
			return FALSE;
		}
		/* sel shouldn't ever be zero, but just in case */
		else if( sel != 0 ) 
		{
			/* Act on events */
			if( FD_ISSET( xb->fd, &f_r ) != 0 )
				xbee_proc_incoming( xb );
			
			if( FD_ISSET( xb->fd, &f_w ) != 0 )
				xbee_proc_outgoing( xb );
		}
	}
}

static gboolean xbee_proc_incoming( xbee_t* xb )
{
	assert( xb != NULL );

	while( xbee_read_frame( xb ) == 0 )
	{
		uint16_t flen;
		flen = (xb->inbuf[1] << 8) | xb->inbuf[2];

		/* Frame received */
		/* TODO: Process frame! */
		debug_show_frame( xb->inbuf, flen + 4 );

		/* Discard frame after processing? */
		xb->in_len = 0;
	}

	return TRUE;
}

/* This function needs a bit of cleanup */
static gboolean xbee_proc_outgoing( xbee_t* xb )
{
	uint8_t d;
	ssize_t w;
	xb_frame_t* frame;
	const uint8_t FRAME_START = 0x7E;
	assert( xb != NULL );
	/* If there's an item in the list, transmit part of it */

	while( g_queue_get_length( xb->out_frames ) )
	{
		frame = (xb_frame_t*)g_queue_peek_tail( xb->out_frames );
		if( xb->tx_pos == 0 )
			d = FRAME_START;
		else if( xb->tx_pos == 1 )
			d = (frame->len >> 8) & 0xFF;
		else if( xb->tx_pos == 2 )
			d = frame->len & 0xFF;
		else if( xb->tx_pos < frame->len + 3 )
		{
			/* next byte to be transmitted in the frame buffer */
			uint16_t dpos = xb->tx_pos - 3; 

			d = frame->data[ dpos ];
		}
		else
		{
			if( !xb->checked )
			{
				/* Calculate checksum */
				xb->o_chk = xbee_sum_block( frame->data, frame->len, 0 );
				xb->o_chk = 0xFF - xb->o_chk;
			}

			d = xb->o_chk;
		}

		/* Requires escaping? */
		if( xb->tx_pos != 0 && ( d == 0x7E || d == 0x7D || d == 0x11 || d == 0x13 ) )
		{
			if( !xb->tx_escaped )
				d = 0x7D;
			else
			{
				d ^= 0x20;
				xb->tx_escaped = FALSE;
			}
		}

		/* write data */
		w = TEMP_FAILURE_RETRY(write( xb->fd, &d, 1 ));
		if( w == -1 && errno == EAGAIN )
			break;
		if( w == -1 )
		{
			fprintf( stderr, "Error writing to file: %m\n" );
			return FALSE;
		}
		if( w == 0 ) continue;

/* 		printf( "Wrote: %2.2X\n", (unsigned int)d ); */

		if( xb->tx_pos > 0 && d == 0x7D )
			xb->tx_escaped = TRUE;
		else
			xb->tx_pos += w;

		xb->bytes_tx += w;

		/* check for end of frame */
		if( frame->len + 4 == xb->tx_pos )
		{
			g_queue_pop_tail( xb->out_frames );
			xb->frames_tx ++;
			xb->tx_pos = 0;
			xb->o_chk = 0;
			xb->checked = FALSE;
			printf( "Frame transmitted\n" );
		}
	}

	return TRUE;
}

static gboolean xbee_outgoing_queued( xbee_t* xb )
{
	assert( xb != NULL );

	if( g_queue_get_length(xb->out_frames) )
		return TRUE;
	else
		return FALSE;
}

/* Reads in available bytes from the input.
 * When a full frame is achieved, it returns 0.
 * When a full frame has not been acheived, it returns 1.
 * When an error occurs, it returns -1 */
static int xbee_read_frame( xbee_t* xb )
{
	int r;
	uint8_t d;
	gboolean whole_frame = FALSE;
	assert( xb != NULL && xb->in_len < XB_INBUF_LEN );

	while( !whole_frame )
	{
		r = TEMP_FAILURE_RETRY( read( xb->fd,  &d, 1 ) );

		if( r == -1 )
		{
			if ( errno == EAGAIN )
				break;

			fprintf( stderr, "Error: Failed to read input: %m\n" );
			return -1;
		}

		if( r == 0 ) continue;

		xb->bytes_rx ++;

		/* If we come across the beginning of a frame */
		if( d == 0x7E )
		{
			/* Discard current data */
			xb->bytes_discarded += xb->in_len;
			xb->in_len = 1;
			xb->inbuf[0] = d;

			/* Cancel escaping */
			xb->escape = FALSE;
		}
		else
		{
			/* Unescape data if necessary */
			if( xb->escape ) 
			{
				d ^= 0x20;
				xb->escape = FALSE;
			}
			else if( d == 0x7D )
				xb->escape = TRUE;

			/* Make sure we don't overflow the buffer */
			if( xb->in_len == XB_INBUF_LEN )
			{
				fprintf( stderr, "Warning: Incoming frame too long - discarding\n" );
				xb->bytes_discarded += xb->in_len;
				xb->in_len = 0;
			}

			if( !xb->escape )
			{
				xb->inbuf[ xb->in_len ] = d;
				xb->in_len ++;
			}

		}

		if( xb->in_len >= 3 )
		{
			uint16_t flen;

			flen = (xb->inbuf[1]) << 8 | xb->inbuf[2];

			if( xb->in_len >= (flen + 4) )
			{
				uint8_t chk;

				/* Check the checksum */
				chk = xbee_checksum( &xb->inbuf[3], flen );

				if( chk == xb->inbuf[ flen + 3 ] )				    
					whole_frame = TRUE;
				else
				{
					/* Checksum invalid */
					memmove( xb->inbuf, &xb->inbuf[flen + 4], xb->in_len - (flen + 4 ) );
					xb->frames_discarded ++;
				}
			}
		}
	}

	if( !whole_frame )
	{
		return 1;	/* Not a whole frame yet */
	}
	xb->frames_rx++;
	return 0;	/* Whole frame */
}

static uint8_t xbee_checksum( uint8_t* buf, uint16_t len )
{
	uint8_t c = 0;
	assert( buf != NULL );

	for( ; len > 0; len -- )
		c += buf[len - 1];

	return 0xFF - c;
}

static uint8_t xbee_sum_block( uint8_t* buf, uint16_t len, uint8_t cur )
{
	assert( buf != NULL );

	for( ; len > 0; len-- )
		cur += buf[len - 1];

	return cur;
}

void debug_show_frame( uint8_t* buf, uint16_t len )
{
	uint16_t i;

	printf("IN: ");
	for( i=0 ; i < len; i++ )
	{
		printf( "%2.2X ", (unsigned int)buf[i] );

		if( (i+1)%16 == 0 )
			printf( "\n" );
	}
	printf("\n");
}
