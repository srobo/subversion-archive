#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <sys/un.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "xbee-conn.h"
#include "commands.h"
#include "libxcli.h"
#include "common-fd.h"

static void xbee_conn_instance_init (GTypeInstance *gti, gpointer g_class );
static gboolean xbee_conn_sock_incoming ( XbeeConn *conn );
static gboolean xbee_conn_sock_outgoing ( XbeeConn *conn );
static gboolean xbee_conn_sock_error( XbeeConn *conn );
static gboolean xbee_conn_sock_data_ready( XbeeConn *conn );

/* Attempts to write a frame to FD */
/* Return Value: Error = -1, EAGAIN = 0, Successful Write = 1 */
static int xbee_conn_write_whole_frame ( XbeeConn *conn );
/* Adds Frame to Queue */
static void xbee_conn_out_queue_add ( XbeeConn *conn, uint8_t *data, uint16_t len );
/* Connects to the server at the given address */
static gboolean xbee_conn_create_socket ( XbeeConn *conn, char *addr );


GType xbee_conn_get_type (void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo info = {
			sizeof (XbeeConnClass),
			NULL,			/* base_init */
			NULL, 			/* base_finalise */
			NULL, 			/* class_init */
			NULL, 			/* class_finalise */
			NULL, 			/* class_data */
			sizeof (XbeeConn),
			0,			/* n_preallocs */
			xbee_conn_instance_init 	/* instance_init */
		};
		type = g_type_register_static (G_TYPE_OBJECT, 
					       "XbeeConnType", 
					       &info, 0);

	}

	return (type);
}


gboolean xbee_conn_transmit( XbeeConn* conn, xb_addr_t addr, uint8_t* data, uint16_t len )
{
 	/* Transmit Frame Layout 
	   0: Command Code: XBEE_COMMAND_TRANSMIT
	   * 1: Address type
	   *** 16-bit address format:
	   *     2-3: Address - MSB first.
	   *     4->(3+len): Data for transmission.
	   *** 64-bit address format:
	   *     2-9: Address - MSB first
	   *     10->(9+len): Data for transmission */

	uint8_t fdata[len + 10];

	assert ( conn != NULL && data !=NULL );
		
	/* Copy data to frame */
	fdata[0] = XBEE_COMMAND_TRANSMIT;
	fdata[1] = addr.type;	
	if (addr.type == XB_ADDR_16)
	{
		if ((len + 6) > XBEE_MAX_FRAME) /* checks length of (data + command + address-type + 16bit address + 16 bit frame length) */
		{
			fprintf (stderr, "Error: Data exceeds maximum frame length %m\n");
			return FALSE;
		}
		memmove (&fdata[2], addr.addr, 2);
		memmove (&fdata[4], data, len);
		len = len + 4;
	}
	else
	{
		if ((len + 12) > XBEE_MAX_FRAME) /* checks length of (data + command + address-type + 64bit address + 16bit frame legnth) */
		{
			fprintf (stderr, "Error: Data exceeds maximum frame length %m\n");
			return FALSE;
		}			
		memmove (&fdata[2], addr.addr, 8);
		memmove (&fdata[10], data, len);
		len = len + 10;
	}
		 
	xbee_conn_out_queue_add ( conn, fdata, len );
	
	return TRUE;
}

XbeeConn *xbee_conn_new ( char * addr, GMainContext *context )
{
	XbeeConn *conn;
	
	conn = g_object_new ( XBEE_CONN_TYPE, NULL );

	if ( !xbee_conn_create_socket (conn, addr) )
		return NULL;
	
	conn->context = context;
	
	conn->source = xbee_fd_source_new( conn->fd, 
					   context,
					   (gpointer)conn,
					   (xbee_fd_callback)xbee_conn_sock_incoming,
					   (xbee_fd_callback)xbee_conn_sock_outgoing,
					   (xbee_fd_callback)xbee_conn_sock_error,
					   (xbee_fd_callback)xbee_conn_sock_data_ready);

	return conn;
}

static void xbee_conn_instance_init (GTypeInstance *gti, gpointer g_class )
{
	assert (gti != NULL);

	XbeeConn *conn = (XbeeConn*)gti;

	conn->out_frames = g_queue_new();
	conn->outpos = 0;
}

gboolean xbee_conn_create_socket ( XbeeConn *conn, char * addr )
{
	struct sockaddr_un sock_addr =
		{
			.sun_family = AF_LOCAL,
		};
	assert( conn != NULL && addr != NULL );

	if (strlen (addr) < 108)
		strncpy ( (char*)(&sock_addr.sun_path), addr, strlen (addr));
	else
	{
		fprintf (stderr, "Socket address too long: %m\n");
		return FALSE;
	}

	/* Attempt to create socket to server */
	conn->fd = socket ( PF_LOCAL, SOCK_STREAM, 0 );

	if ( conn->fd == -1 )
	{
		fprintf ( stderr, "Failed to create socket: %m\n");
		return FALSE;
	}

	if( !fd_set_nonblocking( conn->fd ) )
		return FALSE;
	
	if ( connect ( conn->fd, (struct sockaddr*)&sock_addr,
		       sizeof ( short int ) + strlen (sock_addr.sun_path) ) == -1 )
	{
		fprintf ( stderr, "Failed to make a connection: %m\n" );
		return FALSE;
	}
	
	return TRUE;
}
	

static gboolean xbee_conn_sock_incoming( XbeeConn *conn )
{
	assert ( conn != NULL );
	
	return TRUE;
}

static gboolean xbee_conn_sock_outgoing( XbeeConn *conn )
{
	
	int ret_val = 1;

	assert ( conn != NULL );

	while ( xbee_conn_sock_data_ready(conn) && (ret_val == 1) )
	{
		ret_val = xbee_conn_write_whole_frame (conn); 
		if (ret_val == -1)	
			return FALSE;
	}	
	return TRUE;

}

static gboolean xbee_conn_sock_data_ready( XbeeConn *conn )
{

	if ( g_queue_peek_tail ( conn->out_frames ) == NULL )
		return FALSE;
	else
		return TRUE;
}

static gboolean xbee_conn_sock_error( XbeeConn *conn )
{
	fprintf ( stderr, "Error with connection socket.\n");
	return FALSE;
}

static int xbee_conn_write_whole_frame ( XbeeConn *conn )
{
	xb_frame_t *frame;
	assert ( conn != NULL );
	
	frame = (xb_frame_t*)g_queue_peek_tail ( conn->out_frames );
	assert ( frame != NULL );

	while ( conn->outpos < (frame->len + 2) )
	{
		int b = 0;

		if (conn->outpos < 2)
		{
			uint8_t flen[2];
			
			/* MSByte for frame length 1st */
			flen[0] = (frame->len >> 8) & 0xFF;
			flen[1] = frame->len & 0xFF;
			
			/* Write Frame Length */
			b = write ( conn->fd, &flen [ conn->outpos ], 2 - conn->outpos );
		}
		else
			/* Write frame data */
			b = write ( conn->fd, &frame->data[ conn->outpos -2 ], frame->len + 2 - conn->outpos );
		
		/* If error occurs */
		if ( b == -1 )
		{
			if ( errno == EAGAIN )
				return 0;
			
			fprintf ( stderr, "Error: Failed to write to server: %m\n" );
			return -1;
		}
		
		/* Do not do anything further in this iteration if no bytes written */
		if ( b == 0 )
			continue;
		
		conn->outpos += b;
		assert ( conn->outpos <= (frame->len + 2) ); 
	}

	g_queue_pop_tail ( conn->out_frames );
	conn->outpos = 0;
	return 1;
}

static void xbee_conn_out_queue_add ( XbeeConn *conn, uint8_t *data, uint16_t len )
{
	xb_frame_t *frame;

	frame = (xb_frame_t*)g_malloc ( sizeof (xb_frame_t) );
	assert ( conn != NULL && data != NULL && frame != NULL);

	frame->data = g_memdup ( data, len );
	assert ( frame->data != NULL );
	
	frame->len = len;
	
	/* Add to the head of the queue */
	g_queue_push_head ( conn->out_frames, frame );

	xbee_fd_source_data_ready( conn->source );
}
    
void xbee_conn_command_test ( XbeeConn * conn, gchar *data)
{
	int datalen;
	datalen = (strlen (data) + 1);

	uint8_t fdata[datalen];
	assert ( conn != NULL && data != NULL );
	
	fdata[0] = XBEE_COMMAND_TEST;
	memmove (&fdata[1], data, datalen - 1);
	
	xbee_conn_out_queue_add ( conn, fdata, datalen);
}

