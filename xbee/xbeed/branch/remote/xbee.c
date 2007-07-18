#include "xbee.h"
#include <unistd.h>
#include <sys/select.h>
#include <stdint.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include "xbee_at.h"
/* #include "xbee_ipc.h" */
#include <linux/joystick.h>
#include <fcntl.h>


#define BUFLEN 256

/* Displays the contents of a frame */
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

/* Returns the next byte to transmit */
static uint8_t xbee_outgoing_next( xbee_t* xb );

/* Adds a frame to the transmit queue */
static gboolean xbee_out_queue_add( xbee_t* xb, uint8_t *data, uint8_t len );
static void xbee_out_queue_add_frame( xbee_t* xb, xb_frame_t* frame );

/* Removes the last frame from the transmit queue */
static void xbee_out_queue_del( xbee_t* xb );

static void xbee_print_stats( xbee_t* xb );

/*** "Internal" Client API Functions ***/
int xbee_transmit( xbee_t* xb, xb_addr_t* addr, void* buf, uint8_t len );

/* Copy 64 bytes into a buffer -> MSB to 0, LSB to end of buffer */
static void copy_64b_ml( uint64_t data, uint8_t* buf );

typedef struct 
{
	struct js_event info;
	gboolean transmitted;
} joy_state_t;

typedef struct
{
	int fd;

	uint8_t buffer[ sizeof( struct js_event ) ];
	int count;

	GArray *axes;
	/* sizes of the arrays */
	uint8_t a_size;
}  joy_t;

gboolean joy_open( joy_t *joy );
gboolean joy_proc( joy_t *joy, xbee_t *xb );

/* Process an initialisation event from the joystick */
void joy_proc_init( struct js_event* ev, GArray* array, uint8_t* size );

/* Process an event from the joystick */
void joy_proc_event( struct js_event* ev, GArray* array, uint8_t size );

/* Transmit pending joystick events */
void joy_gen_events( joy_t *joy, xbee_t *xb );
void joy_gen_events_array( xbee_t *xb, GArray *array, uint8_t size );

/* Transmit a joystick event */
void joy_transmit( xbee_t *xb, struct js_event* ev );

int16_t joy_diff( int16_t a, int16_t b );

void hack( xbee_t* xb );
void grab_address( xbee_t* xb );

void xbee_init( xbee_t* xb, int fd )
{
	assert( xb != NULL && fd >= 0 );

	xb->fd = fd;
	xb->api_mode = FALSE;
	xb->at_time.tv_sec = 0;
	xb->at_time.tv_usec = 0;
	xb->at_mode = FALSE;

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
	joy_t joy;

	if( !xbee_set_api_mode( xb ) )
	{
		fprintf( stderr, "Failed to enter API mode - quitting.\n" );
		return FALSE;
	}

	
	if( !joy_open(&joy) )
		return FALSE;

	while( 1 )
	{
		int sel;

		FD_ZERO( &f_r );
		FD_ZERO( &f_w );

		/* We always want to monitor for incoming data */
		FD_SET( xb->fd, &f_r );
		FD_SET( joy.fd, &f_r );
	
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
			
			if( FD_ISSET( joy.fd, &f_r ) != 0 )
				if ( joy_proc( &joy , xb ) != 0 )
					return FALSE;
			
			if( FD_ISSET( xb->fd, &f_w ) != 0 )
				xbee_proc_outgoing( xb );
		}

		if( g_queue_get_length( xb->out_frames ) < 5 )
			joy_gen_events( &joy, xb );

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

static uint8_t xbee_outgoing_next( xbee_t* xb )
{
	const uint8_t FRAME_START = 0x7E;
	xb_frame_t* frame;

	assert( xb != NULL );

	frame = (xb_frame_t*)g_queue_peek_tail( xb->out_frames );

	if( xb->tx_pos == 0 )
		return FRAME_START;
	else if( xb->tx_pos == 1 )
		return (frame->len >> 8) & 0xFF;
	else if( xb->tx_pos == 2 )
		return frame->len & 0xFF;
	else if( xb->tx_pos < frame->len + 3 )
	{
		/* next byte to be transmitted in the frame buffer */
		uint16_t dpos = xb->tx_pos - 3; 

		return frame->data[ dpos ];
	}
	else
	{
		if( !xb->checked )
		{
			/* Calculate checksum */
			xb->o_chk = xbee_sum_block( frame->data, frame->len, 0 );
			xb->o_chk = 0xFF - xb->o_chk;
		}

		return xb->o_chk;
	}
}

/* This function needs a bit of cleanup */
static gboolean xbee_proc_outgoing( xbee_t* xb )
{
	uint8_t d;
	ssize_t w;
	xb_frame_t* frame;

	assert( xb != NULL );
	/* If there's an item in the list, transmit part of it */

	while( g_queue_get_length( xb->out_frames ) )
	{
		frame = (xb_frame_t*)g_queue_peek_tail( xb->out_frames );
		d = xbee_outgoing_next( xb );

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

/*  		printf( "Wrote: %2.2X\n", (unsigned int)d );  */

		if( xb->tx_pos > 0 && d == 0x7D )
			xb->tx_escaped = TRUE;
		else
			xb->tx_pos += w;

		xb->bytes_tx += w;

		/* check for end of frame */
		if( frame->len + 4 == xb->tx_pos )
		{
			xbee_out_queue_del( xb );
			xb->frames_tx ++;
			xb->tx_pos = 0;
			xb->o_chk = 0;
			xb->checked = FALSE;
/* 			printf( "Frame transmitted\n" ); */
			xbee_print_stats( xb );
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


/*  		printf( "Read: %2.2X\n", (unsigned int)d ); */
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
					printf( "Checksum invalid\n" );
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
	return;

	printf("IN: ");
	for( i=0 ; i < len; i++ )
	{
		printf( "%2.2X ", (unsigned int)buf[i] );

		if( (i+1)%16 == 0 )
			printf( "\n" );
	}
	printf("\n");
}

static gboolean xbee_out_queue_add( xbee_t* xb, uint8_t *data, uint8_t len )
{
	xb_frame_t *frame;
	assert( xb != NULL && data != NULL );
	
	frame = g_malloc( sizeof( xb_frame_t ) );

	/* Copy data into frame */
	frame->data = g_memdup( data, len );
	frame->len = len;

	g_queue_push_head( xb->out_frames, frame );

	return TRUE;
}

static void xbee_out_queue_add_frame( xbee_t* xb, xb_frame_t* frame )
{
	assert( xb != NULL && frame != NULL );

	g_queue_push_head( xb->out_frames, frame );
}


static void xbee_out_queue_del( xbee_t* xb )
{
	xb_frame_t *frame;
	assert( xb != NULL );

	frame = (xb_frame_t*)g_queue_peek_tail( xb->out_frames );

	/* Free the data */
	g_free( frame->data );
	frame->data = NULL;

	/* Free the element */
	g_free( frame );

	g_queue_pop_tail( xb->out_frames );
}

int xbee_transmit( xbee_t* xb, xb_addr_t* addr, void* buf, uint8_t len )
{
	xb_frame_t *frame;
	uint8_t* pos;
	assert( xb != NULL && addr != NULL && buf != NULL );

	/* 64-bit address frame structure:
	 * 0: API Identifier: 0x00
	 * 1: Frame ID
	 * 2-9: Target address - MSB first
	 * 10: Option flags
	 * 11...10+len: Data */

	/* 16-bit address frame structure:
	 * 0: API Identifier: 0x01
	 * 1: Frame ID
	 * 2-3: Target address - MSB first
	 * 4: Option flags
	 * 5...4+len: Data */

	frame = g_malloc( sizeof(xb_frame_t) );

	/* Calculate frame length */
	if( addr->type == XB_ADDR_16 )
		frame->len = len + 5;
	else
		frame->len = len + 11;

	frame->data = g_malloc( frame->len );

	if( addr->type == XB_ADDR_64 )
	{
		frame->data[0] = 0x00; /* API Identifier */
		/* Copy address */
		g_memmove( &frame->data[2], addr->addr, 8 );
/* 		copy_64b_ml( XB_ADDR_GET_64(addr), &frame->data[2] ); */
		pos = &frame->data[10]; 
	}
	else
	{
		frame->data[0] = 0x01; /* API Identifier */
		/* Copy address */
		g_memmove( &frame->data[2], addr->addr, 2 );
		pos = &frame->data[4];
	}
	/* pos now pointing at option byte */
	*pos = 0;		/* TODO: Option byte */
	pos ++;

	/* Frame ID: */
	frame->data[1] = 0;	/* TODO: Frame ID */

	/* Copy data */
	g_memmove( pos, buf, len );

	xbee_out_queue_add_frame( xb, frame );

	return 0;
}

static void copy_64b_ml( uint64_t data, uint8_t* buf )
{
	uint8_t i;
	assert( buf != NULL );

	for( i=8; i > 0; i-- )
	{
		buf[i-1] = data & 0xFF;
		data >>= 8;
	}
}

void hack( xbee_t* xb )
{
	uint8_t data[] = {0,1,2,3,4,5};
	xb_addr_t addr =
		{
			.type = XB_ADDR_64,
			.addr = {0x00, 0x13, 0xA2, 0x00, 0x40, 0x09, 0x00, 0xA9}
		};
	assert( xb != NULL );

	if( g_queue_get_length( xb->out_frames ) == 0 )
		xbee_transmit( xb, &addr, data, sizeof( data ) );

}

void grab_address( xbee_t* xb )
{
	uint8_t sh[] = { 0x08, 2, 'S', 'H' };
	uint8_t sl[] = { 0x08, 3, 'S', 'L' };

	xbee_out_queue_add( xb, sh, 4 );
	xbee_out_queue_add( xb, sl, 4 );
}

static void xbee_print_stats( xbee_t* xb )
{
	assert( xb != NULL );

	printf( "\rFrames: %6lu IN, %6lu OUT. Bytes: %9lu IN, %9lu OUT Queued: %5u",
		(long unsigned int)xb->frames_rx, 
		(long unsigned int)xb->frames_tx, 
		(long unsigned int)xb->bytes_rx, 
		(long unsigned int)xb->bytes_tx, 
		g_queue_get_length( xb->out_frames )
		);
}


void xbee_free( xbee_t* xb )
{
	assert( xb != NULL );

	while( g_queue_get_length( xb->out_frames ) > 0 )
	{
		xb_frame_t *f = (xb_frame_t*)g_queue_peek_tail(xb->out_frames);
		g_free( f->data );
		g_free( f );
		g_queue_pop_tail( xb->out_frames );
	}

	g_queue_free( xb->out_frames );
	xb->out_frames = NULL;
}


gboolean joy_open( joy_t *joy )
{
	assert( joy != NULL );

	joy->fd = open ("/dev/js0", O_RDONLY);	        

	if ( joy->fd < 0 )
	{
		printf("\nError with joystick\n");
		printf("%m\n");
		return FALSE;
	}

	/* Initialise the structure */
	joy->count = 0;
	joy->axes = g_array_new(FALSE, TRUE, sizeof(joy_state_t));
	joy->a_size = 0;

	return TRUE;
}


gboolean joy_proc( joy_t *joy, xbee_t *xb )
{
	assert( joy != NULL && xb != NULL );
	int number_read;

	number_read = read (joy->fd, 
			    joy->buffer + joy->count, 
			    sizeof(struct js_event) - joy->count); //read 8-count

	if ( number_read < 0 ) 
	{
		fprintf(stderr,"Bad read from stick\n");
		return(-1);
	}

	joy->count += number_read;
                
	/* Have we got a full event struct? */
	if ( joy->count == sizeof( struct js_event ) )
	{
		struct js_event *ev = (struct js_event*)joy->buffer;

		joy->count = 0;

		/* Determine if the packet's a dummy */
		if( ev->type & JS_EVENT_INIT )
		{
			if( ev->type & JS_EVENT_AXIS )
			{
				joy_proc_init( ev, joy->axes, &joy->a_size );
/* 				printf( "\rJoystick init event: Axis %hhu\n", ev->number ); */
			}
			else if( ev->type & JS_EVENT_BUTTON )
			{
/* 				printf( "\rJoystick init event: Button %hhu\n", ev->number ); */
			}
		}
		else
		{
			if( ev->type & JS_EVENT_BUTTON )
			{
				/* Transmit button events */
				joy_transmit( xb, ev );
			}
			else if( ev->type & JS_EVENT_AXIS )
			{
				/* Buffer axis events */
				joy_proc_event( ev, joy->axes, joy->a_size );
			}
		}
	}

	return 0;
}

void joy_proc_init( struct js_event* ev, GArray* array, uint8_t* size )
{
	joy_state_t *s;

	if( ev->number + 1 > *size )
	{
		*size = ev->number + 1;
		g_array_set_size( array, *size );
	}

	s = &g_array_index( array, joy_state_t, ev->number );

	s->info = *ev;
	s->info.type &= ~JS_EVENT_INIT;
	s->transmitted = FALSE;
}

void joy_proc_event( struct js_event* ev, GArray* array, uint8_t size )
{
	assert( ev != NULL && array != NULL );
	assert( ev->number < size );
	joy_state_t *s;

	s = &g_array_index( array, joy_state_t, ev->number );

	if( joy_diff( ev->value, s->info.value )  > 90 )
	{
		s->transmitted = FALSE;
		s->info = *ev;
	}
}

void joy_gen_events( joy_t *joy, xbee_t *xb )
{
	assert( joy != NULL && xb != NULL );

	joy_gen_events_array( xb, joy->axes, joy->a_size );
}

void joy_gen_events_array( xbee_t *xb, GArray *array, uint8_t size )
{
	uint8_t i;
	
	for( i = 0; i < size; i ++ )
	{
		joy_state_t *s = &g_array_index( array, joy_state_t, i );

		if( !s->transmitted )
		{
			joy_transmit( xb, &s->info );
			s->transmitted = TRUE;
		}
	}
}

void joy_transmit( xbee_t *xb, struct js_event* ev )
{
	xb_addr_t broadcast = {
		.type=XB_ADDR_64,
		.addr={0,0,0,0,0,0,0xff,0xff}}; /* Broadcast */
//				.addr={0,0x13,0xA2,0,0x40,0x09,0,0xA8}};

	xbee_transmit( xb , &broadcast, ev , sizeof(struct js_event));
}

int16_t joy_diff( int16_t a, int16_t b )
{
	if( a > b )
		return a - b;
	return b - a;
}
