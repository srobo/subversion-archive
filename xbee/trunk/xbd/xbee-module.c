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

#include "common-fd.h"
#include "xbee-module.h"
#include "xbee_at.h"

gboolean xbee_module_io_error( XbeeModule* xb );

/*** Incoming Data Functions ***/

/* Process incoming data */
gboolean xbee_module_proc_incoming( XbeeModule* xb );

/* Reads in available bytes from the input.
 * When a full frame is achieved, it returns 0.
 * When a full frame has not been acheived, it returns 1.
 * When an error occurs, it returns -1 */
static int xbee_module_read_frame( XbeeModule* xb  );

static uint8_t xbee_module_checksum( uint8_t* buf, uint16_t len );

/* Displays the contents of a frame */
static void debug_show_frame( uint8_t* buf, uint16_t len );

static void debug_show_data( uint8_t* buf, uint16_t len );

/*** Outgoing Queue Functions ***/

/* Process outgoing data */
static gboolean xbee_module_proc_outgoing( XbeeModule* xb );

static uint8_t xbee_module_outgoing_escape_byte( XbeeModule* xb, uint8_t d );

/* Whether data's ready to transmit */
static gboolean xbee_module_outgoing_queued( XbeeModule* xb );

/* Returns the next byte to transmit */
static uint8_t xbee_module_outgoing_next( XbeeModule* xb );

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

/*** "Internal" Client API Functions ***/

/* Configure the serial port */
gboolean xbee_serial_init( XbeeModule* xb );

/* Initialise the module. */
gboolean xbee_init( XbeeModule* xb );

void xbee_instance_init( GTypeInstance *xb, gpointer g_class );

/* Send an AT command 
   Args:
    -   command: Pointer to a 2 character array containing the AT
                 command string.
    - parameter: String containing the arguments for the AT command.
                 Can be NULL (useful for just reading values).
    -  callback: The callback function to call when the response comes in.
                 Can be NULL.
    -  userdata: The userdata to pass to the callback.
*/
gboolean xbee_module_at_command( XbeeModule *xb,
				 gchar *command,
				 gchar *parameter,
				 xb_response_callback_t callback,
				 gpointer userdata );

/* Get the next free frame ID.
   Returns 0 when none are free. */
static uint8_t xbee_module_get_next_fid( XbeeModule *xb );

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

static void xbee_module_out_queue_add_frame( XbeeModule* xb, xb_frame_t* frame )
{
	assert( xb != NULL && frame != NULL );

	g_queue_push_head( xb->out_frames, frame );
	xbee_fd_source_data_ready( xb->source );
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

	printf("Transmitting: ");
	debug_show_data( buf, len );
	printf("\n");

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
	frame->data[1] = 0;	/* TODO: Frame ID */

	/* Copy data */
	g_memmove( pos, buf, len );

	xbee_module_out_queue_add_frame( xb, frame );

	return 0;
}

static void xbee_module_print_stats( XbeeModule* xb )
{
	assert( xb != NULL );
	return;

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
	
	if( !fd_set_nonblocking( xb->fd ) )
		return FALSE;

	if( !xbee_init( xb ) )
		return FALSE;

	xb->source = xbee_fd_source_new( xb->fd, context, (gpointer)xb,
					 (xbee_fd_callback)xbee_module_proc_incoming,
					 (xbee_fd_callback)xbee_module_proc_outgoing,
					 (xbee_fd_callback)xbee_module_io_error,
					 (xbee_fd_callback)xbee_module_outgoing_queued );
					 
//	xbee_module_add_source( xb, context );

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
	uint16_t i;

	/* Clear all frame callbacks */
	for( i=0; i<256; i++ )
		xb->response_callbacks[i].callback = NULL;

	xb->next_frame_id = 1;

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
		uint8_t *data;
		flen = ((uint16_t)xb->inbuf[1] << 8) | (uint16_t)xb->inbuf[2];
		fprintf (stderr, "Received Frame-length is: %d\n", flen);
		data = &xb->inbuf[3];

		/* TODO: Process frame! */

		switch( data[0] )
		{
		case XBEE_FRAME_RX_16:
		{
			xb_rx_info_t info;
			
			printf ("RX_16 received\n");

			info.src_addr.type = XB_ADDR_16;
			memmove (info.src_addr.addr, &data[1], 2);
			info.rssi = data[3];
			
			info.address_broadcast = (data[4] & 0x02) ? TRUE : FALSE;
			info.pan_broadcast  = (data[4] & 0x04) ? TRUE : FALSE;
			
			info.src_channel = data[5];
			info.dst_channel = data[6];

			xb->xb_callbacks.rx_frame (&info, &data[7], flen - 7, xb->userdata);
			break;
		}
		case XBEE_FRAME_RX_64:
		{
			xb_rx_info_t info;

			info.src_addr.type = XB_ADDR_64;
			memmove (info.src_addr.addr, &data[1], 8);
			info.rssi = data[9];
						
			info.address_broadcast = (data[10] & 0x02) ? TRUE : FALSE;
			info.pan_broadcast  = (data[10] & 0x04) ? TRUE : FALSE;

			info.src_channel = data[11];
			info.dst_channel = data[12];

			xb->xb_callbacks.rx_frame (&info, &data[13], flen - 13, xb->userdata);
			break;
		}

		case XBEE_FRAME_TX_STAT:
			printf("Transmit status received.\n");
			break;

		case XBEE_FRAME_AT_COMMAND_RESP:
		{
			/* Response to an AT Command */
			uint8_t fid;

			/* Format:
			     0: API code
			     1: Frame ID
			   2-3: AT Command Code
			     4: Status
			    5-: Requested value	*/

			fid = data[1];

			if( xb->response_callbacks[fid].callback != NULL )
			{
				xb->response_callbacks[fid].callback( xb, 
								      &data[5],
								      flen - 5,
								      xb->response_callbacks[fid].userdata );
				xb->response_callbacks[fid].callback = NULL;
			}
			else
				g_warning( "AT command response received, but no handler is registered" );

			break;
		}

		default:
			printf("Unhandled frame received:\n");
			debug_show_frame( xb->inbuf, flen + 4 );
		}

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

		/* Serial devices can return 0 - but doesn't mean EOF */
		if( r == 0 ) break;


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
	printf("IN: ");
	debug_show_data( buf, len );
	printf("\n");
}

static void debug_show_data( uint8_t* buf, uint16_t len )
{
	uint16_t i;

	for( i=0 ; i < len; i++ )
	{
		printf( "%2.2X ", (unsigned int)buf[i] );
	}
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

gboolean xbee_module_io_error( XbeeModule* xb )
{
	fprintf( stderr, "Erk, error.  I should do something\n" );
	return FALSE;
}

void xbee_module_register_callbacks ( XbeeModule *xb, xbee_module_events_t *callbacks, gpointer *userdata)
{
	assert (callbacks != NULL && xb != NULL );
	
	xb->xb_callbacks = *callbacks;
	xb->userdata = userdata;

}

gboolean xbee_module_at_command( XbeeModule *xb,
				 gchar *command,
				 gchar *parameter,
				 xb_response_callback_t callback,
				 gpointer userdata )
{
	xb_frame_t *frame;
	uint8_t fid = 0;
	assert( xb != NULL && command != NULL );

	if( callback != NULL )
	{
		fid = xbee_module_get_next_fid( xb );
		if( fid == 0 )
		{
			g_debug( "No free frame IDs" );
			return FALSE;
		}

		xb->response_callbacks[fid].callback = callback;
		xb->response_callbacks[fid].userdata = userdata;
	}

	frame = g_malloc( sizeof(xb_frame_t) );

	/* Frame structure:
	   0: API Identifier
	   1: Frame ID
	   2-3: AT Command
	   4-: Parameter string */
	
	frame->len = 4;
	/* Add on the parameter string length */
	if( parameter != NULL )
		frame->len += strlen( parameter );

	frame->data = g_malloc( frame->len );

	frame->data[0] = XBEE_FRAME_AT_COMMAND;
	frame->data[1] = fid;
	frame->data[2] = command[0];
	frame->data[3] = command[1];

	if( parameter != NULL )
		g_memmove( &frame->data[4], parameter, strlen( parameter ) );

	/* Queue the frame for sending */
	xbee_module_out_queue_add_frame( xb, frame );

	return TRUE;
}

static uint8_t xbee_module_get_next_fid( XbeeModule *xb )
{
	uint16_t i,j;
	assert( xb != NULL );
	
	for( i=0; i<256; i++ )
	{
		j = (xb->next_frame_id + i) % 256;

		/* Skip entry 0 */
		if( j==0 ) continue;

		if( xb->response_callbacks[j].callback == NULL )
			break;
	}

	if( i == 256 || j == 0 || xb->response_callbacks[j].callback != NULL )
		return 0;

	/* Set the next frame ID to be one higher than the current one */
	xb->next_frame_id = (j+1)%256;
	if( xb->next_frame_id == 0 )
		xb->next_frame_id = 1;

	return j;
}
