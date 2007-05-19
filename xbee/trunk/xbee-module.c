/* XbeeModule  */
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/select.h>
#include <stdint.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#include "xbee-module.h"
#include "xbee_at.h"

/*** Incoming Data Functions ***/

/* Process incoming data */
gboolean xbee_module_proc_incoming( XbeeModule* xb );

/* Reads in available bytes from the input.
 * When a full frame is achieved, it returns 0.
 * When a full frame has not been acheived, it returns 1.
 * When an error occurs, it returns -1 */
int xbee_module_read_frame( XbeeModule* xb  );

static uint8_t xbee_module_checksum( uint8_t* buf, uint16_t len );

/* Displays the contents of a frame */
static void debug_show_frame( uint8_t* buf, uint16_t len );

/*** Outgoing Queue Functions ***/

/* Process outgoing data */
static gboolean xbee_module_proc_outgoing( XbeeModule* xb );

static uint8_t xbee_module_outgoing_escape_byte( XbeeModule* xb, uint8_t d );

/* Whether data's ready to transmit */
static gboolean xbee_module_outgoing_queued( XbeeModule* xb );

/* Returns the next byte to transmit */
static uint8_t xbee_module_outgoing_next( XbeeModule* xb );

/* Allocates a new frame and copies it in */
static gboolean xbee_module_out_queue_add( XbeeModule* xb, uint8_t *data, uint8_t len );

/* Adds the frame directly to the queue (memory allocation must have
 * already been done) */
static void xbee_module_out_queue_add_frame( XbeeModule* xb, xb_frame_t* frame );

/* Removes the last frame from the transmit queue */
static void xbee_module_out_queue_del( XbeeModule* xb );

/*** Misc ***/

/* Calculate the checksum of a block of data */
static uint8_t xbee_module_sum_block( uint8_t* buf, uint16_t len, uint8_t cur );

/* Displays connection statistics */
static void xbee_module_print_stats( XbeeModule* xb );

/* xbee source functions  */
static gboolean xbee_module_source_prepare( GSource *source, gint *timeout_ );

static gboolean xbee_module_source_check( GSource *source );

static gboolean xbee_module_source_dispatch( GSource *source,
					     GSourceFunc callback, 
					     gpointer user_data );

void xbee_module_source_finalize( GSource *source );

static GSourceFuncs xbee_sourcefuncs = 
{
	.prepare = xbee_module_source_prepare,
	.check = xbee_module_source_check,
	.dispatch = xbee_module_source_dispatch,
	.finalize = xbee_module_source_finalize,

	.closure_callback = NULL,
	.closure_marshal = NULL
};

/* Source callback */
gboolean xbee_source_callback( XbeeModule *xb );

/*** "Internal" Client API Functions ***/
int xbee_transmit( XbeeModule* xb, xb_addr_t* addr, void* buf, uint8_t len );

void hack( XbeeModule* xb );

/* Configure the serial port */
gboolean xbee_serial_init( XbeeModule* xb );

/* Initialise the module. */
gboolean xbee_init( XbeeModule* xb );

void xbee_instance_init( GTypeInstance *xb, gpointer g_class );

/* Free information related to a module. */
void xbee_free( XbeeModule* xb );

gboolean xbee_init( XbeeModule* xb )
{
	assert( xb != NULL );

	if( !xbee_serial_init( xb ) )
		return FALSE;

	if( !xbee_module_set_api_mode( xb ) )
	{
		fprintf( stderr, "Failed to enter API mode - quitting.\n" );
		return FALSE;
	}

	return TRUE;
}

static uint8_t xbee_module_outgoing_next( XbeeModule* xb )
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
			xb->o_chk = xbee_module_sum_block( frame->data, frame->len, 0 );
			xb->o_chk = 0xFF - xb->o_chk;
		}

		return xb->o_chk;
	}
}

/* This function needs a bit of cleanup */
static gboolean xbee_module_proc_outgoing( XbeeModule* xb )
{
	uint8_t d;
	ssize_t w;
	xb_frame_t* frame;

	assert( xb != NULL );
	/* If there's an item in the list, transmit part of it */

	while( g_queue_get_length( xb->out_frames ) )
	{
		frame = (xb_frame_t*)g_queue_peek_tail( xb->out_frames );
		d = xbee_module_outgoing_next( xb );

		/* Get the byte to read */
		d = xbee_module_outgoing_escape_byte( xb, d );

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
			xbee_module_out_queue_del( xb );
			xb->frames_tx ++;
			xb->tx_pos = xb->o_chk = 0;
			xb->checked = FALSE;
/* 			printf( "Frame transmitted\n" ); */
			xbee_module_print_stats( xb );
		}
	}

	return TRUE;
}

static gboolean xbee_module_outgoing_queued( XbeeModule* xb )
{
	assert( xb != NULL );

	if( g_queue_get_length(xb->out_frames) )
		return TRUE;
	else
		return FALSE;
}

static uint8_t xbee_module_sum_block( uint8_t* buf, uint16_t len, uint8_t cur )
{
	assert( buf != NULL );

	for( ; len > 0; len-- )
		cur += buf[len - 1];

	return cur;
}

static gboolean xbee_module_out_queue_add( XbeeModule* xb, uint8_t *data, uint8_t len )
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

static void xbee_module_out_queue_add_frame( XbeeModule* xb, xb_frame_t* frame )
{
	assert( xb != NULL && frame != NULL );

	g_queue_push_head( xb->out_frames, frame );
}


static void xbee_module_out_queue_del( XbeeModule* xb )
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

int xbee_transmit( XbeeModule* xb, xb_addr_t* addr, void* buf, uint8_t len )
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
	frame->data[1] = 1;	/* TODO: Frame ID */

	/* Copy data */
	g_memmove( pos, buf, len );

	xbee_module_out_queue_add_frame( xb, frame );

	return 0;
}

void hack( XbeeModule* xb )
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

static void xbee_module_print_stats( XbeeModule* xb )
{
	assert( xb != NULL );

	printf( "\rFrames: %6lu IN, %6lu OUT. Bytes: %9lu IN, %9lu OUT",
		(long unsigned int)xb->frames_rx, 
		(long unsigned int)xb->frames_tx, 
		(long unsigned int)xb->bytes_rx, 
		(long unsigned int)xb->bytes_tx );
}


void xbee_free( XbeeModule* xb )
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

gboolean xbee_serial_init( XbeeModule* xb )
{
	struct termios t;
	assert( xb != NULL );

	if( !isatty( xb->fd ) )
	{
		fprintf( stderr, "File isn't a serial device\n" );
		return FALSE;
	}

	if( tcgetattr( xb->fd, &t ) < 0 )
	{
		fprintf( stderr, "Failed to get terminal device information: %m\n" );
		return 0;
	}
	
	
	switch( xb->parity )
	{
	case PARITY_NONE:
		t.c_iflag &= ~INPCK;
		t.c_cflag &= ~PARENB;
		break;

	case PARITY_ODD:
		t.c_cflag |= PARODD;

	case PARITY_EVEN:
		t.c_cflag &= ~PARODD;
		
		t.c_iflag |= INPCK;
		t.c_cflag |= PARENB;
	}
	
	/* Ignore bytes with parity errors */
	t.c_iflag |= IGNPAR;

	/* 8 bits */
	/* Ignore break conditions */
	/* Keep carriage returns & prevent carriage return translation */
	t.c_iflag &= ~(ISTRIP | IGNBRK | IGNCR | ICRNL);

	switch( xb->flow_control )
	{
	case FLOW_NONE:
		t.c_iflag &= ~( IXOFF | IXON );
 		t.c_cflag &= ~CRTSCTS;
		break;

	case FLOW_RTSCTS:
		t.c_cflag |= CRTSCTS;
		break;

	case FLOW_SOFTWARE:
 		t.c_cflag |= CRTSCTS;
		t.c_iflag |= IXOFF | IXON;
	}

/* 	t.c_cflag &= ~MDMBUF; */

	/* Disable character mangling */
	t.c_oflag &= ~OPOST;
	
	/* No modem disconnect excitement */
	t.c_cflag &= ~(HUPCL | CSIZE);

	/* Use input from the terminal */
	/* Don't use the carrier detect lines  */
	t.c_cflag |= CREAD | CS8 | CLOCAL;

	switch( xb->stop_bits )
	{
	case 0:
		/*  */
		t.c_cflag &= ~CSTOPB;
		break;

	case 1:
		/*  */
		t.c_cflag &= ~CSTOPB;
		break;

	case 2:
		/*  */
		t.c_cflag |= CSTOPB;
		break;
	}

	/*** c_lflag stuff ***/
	/* non-canonical (i.e. non-line-based) */
	/* no input character looping */
	/* no erase printing or usage */
	/* no special character processing */
	t.c_lflag &= ~(ICANON | ECHO | ECHO | ECHOPRT | ECHOK
			| ECHOKE | ISIG | IEXTEN | TOSTOP /* | NOKERNINFO */ );
	t.c_lflag |= ECHONL;


	/* Line speed config */
	if( cfsetspeed( &t, xb->baud ) < 0 ) 
	{
		fprintf( stderr, "Failed to set serial baud rate to %u: %m\n", xb->baud );
		return FALSE; 
	}

	if( tcsetattr( xb->fd, TCSANOW, &t ) < 0 )
	{
		fprintf( stderr, "Failed to configure terminal settings: %m\n" );
		return FALSE;
	}
	return TRUE;
}

void xbee_module_add_source( XbeeModule *xb, GMainContext *context )
{
	assert( xb != NULL && context != NULL );
	GPollFD *pfd;

	xb->source = (xbee_source_t*) g_source_new( &xbee_sourcefuncs, sizeof( xbee_source_t ) );

	/* Set up polling of the serial port file descriptor */
	pfd = &xb->source->pollfd;
	pfd->fd = xb->fd;
	pfd->events = G_IO_IN | G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL;
	g_source_add_poll( (GSource*) xb->source, pfd );

	xb->source->xb = xb;

	xb->source_id = g_source_attach( (GSource*)xb->source, context );

	g_source_set_callback( (GSource*)xb->source,
			       (GSourceFunc)xbee_source_callback,
			       (gpointer)xb,
			       NULL );
}

/* Configure the timeout, and we can't determine if we're ready here */
static gboolean xbee_module_source_prepare( GSource *source, gint *timeout_ )
{
	assert( timeout_ != NULL && source != NULL );

	*timeout_ = -1;

	/* For the moment, we're not ready */
	return FALSE;
}

/* Return TRUE if ready to be dispatched */
static gboolean xbee_module_source_check( GSource *source )
{
	assert( source != NULL );
	xbee_source_t *xb_source = (xbee_source_t*)source; 
	gushort r = xb_source->pollfd.revents;

	if( r & (G_IO_ERR | G_IO_HUP | G_IO_IN | G_IO_OUT | G_IO_NVAL) )
	{
		/* ready to dispatch */
		return TRUE;
	}

	return FALSE; 
}

/* Process incoming/outgoing data */
static gboolean xbee_module_source_dispatch( GSource *source,
				      GSourceFunc callback, 
				      gpointer user_data )
{
	assert( source != NULL );
	xbee_source_t *xb_source = (xbee_source_t*)source; 
	gboolean rval = FALSE;

	/* Call the callback */
	if( callback != NULL )
		rval = callback( xb_source->xb );

	/* Modulate the write requirement if necessary */
	if( xbee_module_outgoing_queued( xb_source->xb ) )
		xb_source->pollfd.events |= G_IO_OUT;
	else
		xb_source->pollfd.events &= ~G_IO_OUT;

	return rval;
}

void xbee_module_source_finalize( GSource *source )
{
	/* Don't need to do anything here. */
	/* glib should free the source structure */
}

gboolean xbee_source_callback( XbeeModule *xb )
{
	assert( xb != NULL );

	if( xb->source->pollfd.revents & (G_IO_ERR | G_IO_HUP) )
	{
		fprintf( stderr, "IO Error\n" );
		return FALSE;
	}
	
	if( xb->source->pollfd.revents & G_IO_IN )
		xbee_module_proc_incoming( xb );

	if( xb->source->pollfd.revents & G_IO_OUT )
		xbee_module_proc_outgoing( xb );

	hack( xb );

	return TRUE;
}

XbeeModule* xbee_module_open( char* fname, GMainContext *context )
{
	XbeeModule *xb = NULL;
	assert( fname != NULL );

	xb = g_object_new( XBEE_MODULE_TYPE, NULL  );

	xb->fd = open( fname, O_RDWR | O_NONBLOCK );
	if( xb->fd < 0 )
	{
		fprintf( stderr, "Error: Failed to open serial port\n" );
		return NULL; 
	}
	
	if( !xbee_init( xb ) )
		return FALSE;

	xbee_module_add_source( xb, context );

	return xb;
}

void xbee_module_close( XbeeModule* xb )
{
	assert( xb != NULL );

	close( xb->fd );

	xbee_free( xb );

	g_object_unref( xb );
}

GType xbee_module_get_type( void )
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo info = {
			sizeof (XbeeModuleClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			NULL,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (XbeeModule),
			0,      /* n_preallocs */
			xbee_instance_init    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_OBJECT,
					       "XbeeModuleType",
					       &info, 0);
	}
	return type;
}

void xbee_instance_init( GTypeInstance *gti, gpointer g_class )
{
	XbeeModule *xb = (XbeeModule*)gti;

	xb->api_mode = FALSE;
	xb->at_time.tv_sec = 0;
	xb->at_time.tv_usec = 0;
	xb->at_mode = FALSE;

	/* The default serial mode is 9600bps 8n1  */
	xb->baud = 9600;
	xb->parity = PARITY_NONE;
	xb->stop_bits = 1;
	xb->flow_control = FLOW_NONE;

	xb->out_frames = g_queue_new();

	xb->in_len = 0;
	xb->escape = FALSE;
	xb->in_callback = NULL;

	xb->bytes_discarded = 0;
	xb->frames_discarded = 0;
	xb->bytes_rx = xb->bytes_tx = 0;
	xb->frames_rx = xb->frames_tx = 0;

	xb->o_chk = xb->tx_pos = 0;
	xb->checked = FALSE;
	xb->tx_escaped = FALSE;
}

gboolean xbee_module_proc_incoming( XbeeModule* xb )
{
	assert( xb != NULL );

	while( xbee_module_read_frame( xb ) == 0 )
	{
		uint16_t flen;
		flen = (xb->inbuf[1] << 8) | xb->inbuf[2];

		/* Frame received */
		if( xb->in_callback != NULL )
			xb->in_callback( xb, xb->inbuf, xb->in_len );

		/* TODO: Process frame! */
		debug_show_frame( xb->inbuf, flen + 4 );

		/* Discard frame after processing? */
		xb->in_len = 0;
	}

	return TRUE;
}

/* Reads in available bytes from the input.
 * When a full frame is achieved, it returns 0.
 * When a full frame has not been acheived, it returns 1.
 * When an error occurs, it returns -1 */
int xbee_module_read_frame( XbeeModule* xb )
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
				chk = xbee_module_checksum( &xb->inbuf[3], flen );

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

static uint8_t xbee_module_checksum( uint8_t* buf, uint16_t len )
{
	uint8_t c = 0;
	assert( buf != NULL );

	for( ; len > 0; len -- )
		c += buf[len - 1];

	return 0xFF - c;
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

static uint8_t xbee_module_outgoing_escape_byte( XbeeModule* xb, uint8_t d )
{
	assert( xb != NULL );

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

	return d;
}

void xbee_module_set_incoming_callback( XbeeModule *xb, 
					xbee_callback_t f )
{
	assert( xb != NULL );
		
	xb->in_callback = f;
}
