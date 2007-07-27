#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <sys/un.h>
#include <assert.h>
#include <unistd.h>

#include "xbee-conn.h"

static void xbee_conn_instance_init (GTypeInstance *gti, gpointer g_class );
static gboolean xbee_conn_sock_incoming ( XbeeConn *conn );
static gboolean xbee_conn_sock_outgoing ( XbeeConn *conn );
static gboolean xbee_conn_sock_error( XbeeConn *conn );
static gboolean xbee_conn_sock_data_ready( XbeeConn *conn );
static gboolean xbee_conn_write_whole_frame ( XbeeConn *conn );

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

gboolean xbee_conn_transmit( XbeeConn* xbc, xb_addr_t addr, uint8_t* data, uint16_t len )
{
	g_print( "Transmit!\n" );
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

	conn->out_frames = NULL;
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
	
	if ( bind( conn->fd, (struct sockaddr*)&sock_addr,
		   sizeof ( short int ) + strlen (sock_addr.sun_path) ) == -1 )
	{
		fprintf ( stderr, "Failed to bind to socket: %m\n" );
		return FALSE;
	}
	
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

	assert ( conn != NULL );

	while ( xbee_conn_sock_data_ready(conn) )
	{
		if ( !xbee_conn_write_whole_frame( conn) )
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

static gboolean xbee_conn_write_whole_frame ( XbeeConn *conn )
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
			flen[0] = (frame->len >> 8) | 0xFF;
			flen[1] = frame->len | 0xFF;
			
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
				return TRUE;
				
			fprintf ( stderr, "Error: Failed to write to server: %m\n" );
			return FALSE;
		}
		
		/* Do not do anything further in this iteration if no bytes written */
		if ( b == 0 )
			continue;
		
		conn->outpos += b;
		assert ( conn->outpos <= (frame->len + 2) ); 
	}

	g_queue_pop_tail ( conn->out_frames );
	conn->outpos = 0;
	return TRUE;
}
