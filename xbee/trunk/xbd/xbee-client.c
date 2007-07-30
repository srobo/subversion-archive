#include "xbee-client.h"
#include "commands.h"
#include "libxcli.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

static void xbee_client_instance_init( GTypeInstance *gti, gpointer g_class );
static gboolean xbee_client_sock_incoming( XbeeClient *client );
static gboolean xbee_client_sock_outgoing( XbeeClient *client );
static gboolean xbee_client_sock_error( XbeeClient *client );

/* Returns TRUE if data is ready to be transmitted */
static gboolean xbee_client_data_ready( XbeeClient *client );

/* Reads available bytes from the input.
 * When a full frame is achieved, returns 0.
 * When a full frame has not been acheived, it returns 1.
 * When an error occurs, it returns -1 */
static int xbee_client_read_frame( XbeeClient *client );

static void xbee_client_dispose( GObject *obj );
static void xbee_client_finalize( GObject *obj );
static void xbee_client_class_init( XbeeClientClass *klass );

static GObjectClass *parent_class = NULL;

GType xbee_client_get_type( void )
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo info = {
			sizeof (XbeeClientClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc)xbee_client_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (XbeeClient),
			0,      /* n_preallocs */
			xbee_client_instance_init    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_OBJECT,
					       "XbeeClientType",
					       &info, 0);
	}
	return type;
}

static void xbee_client_instance_init( GTypeInstance *gti, gpointer g_class )
{
	XbeeClient *client = (XbeeClient*)gti;
	assert( gti != NULL );

	client->in_frames = NULL;
	client->inpos = 0;
	client->server = NULL;
	client->dispose_has_run = FALSE;

	parent_class = g_type_class_peek_parent (g_class);

	g_print( "Wooo.  Client initialised\n" );
}

XbeeClient* xbee_client_new( GMainContext *context, 
			     int sock, 
			     XbeeServer *server,
			     xbee_client_disconnect_t dis )
{
	XbeeClient* client;
	assert( server != NULL && dis != NULL );
	
	client = g_object_new( XBEE_CLIENT_TYPE, NULL );

	client->fd = sock;
	client->disconn = dis;
	client->server = server;

	client->source = xbee_fd_source_new( client->fd,
					     context,
					     (gpointer)client,
					     (xbee_fd_callback)xbee_client_sock_incoming,
					     (xbee_fd_callback)xbee_client_sock_outgoing,
					     (xbee_fd_callback)xbee_client_sock_error,
					     (xbee_fd_callback)xbee_client_data_ready );

	return client;
}

static int xbee_client_read_frame( XbeeClient *client )
{
	int r;
	uint8_t d;
	gboolean whole_frame = FALSE;
	assert( client != NULL );

	while( !whole_frame )
	{
		r = read( client->fd, &d, 1 );
		
		if( r == -1 )
		{
			if( errno == EAGAIN )
				break;
			fprintf( stderr, "Error: Failed to read client input: %m\n" );
			return -1;
		}

		if( r == 0 ) {
			client->disconn( client, client->server );
			return 1;
		}


		client->inbuf[ client->inpos ] = d;
		client->inpos++;

		/* Calculate frame length */
		if( client->inpos == 2 )
		{
			client->flen = (((uint16_t)client->inbuf[0]) << 8) | client->inbuf[1];
			if( client->flen + 2 > XBEE_MAX_FRAME )
			{
				printf( "Frame too long (%u) :-S\n", client->flen );
				return -1;
			}
		}
		
		if( client->inpos == client->flen + 2 ) 
			whole_frame = TRUE;
	}

	if( !whole_frame )
		return 1;
	return 0;		/* Whole frame */
}

static gboolean xbee_client_sock_incoming( XbeeClient *client )
{
	assert( client != NULL );

	while( xbee_client_read_frame( client ) == 0 )
	{
		g_debug( "xbee_client: Frame received.\n" );

		if( client->flen > 0 )
		{
			uint8_t *f = &client->inbuf[2];
			switch( f[0] )
			{
			case XBEE_COMMAND_TEST:
			{
				char* c, *s = (char*)f+1;
				int len = client->flen - 1;

				printf( "Received: " );
				for( c=s; c-s < len; c++ )
					putc( (int)*c, stdout );
				putc( (int)'\n', stdout );
			}
			break;
			}
		}

		/* Discard frame - it's been processed */
		client->inpos = client->flen = 0;
	}

	return TRUE;
}

static gboolean xbee_client_sock_outgoing( XbeeClient *client )
{
	assert( client != NULL );

	return TRUE;
}

static gboolean xbee_client_data_ready( XbeeClient *client )
{
	return FALSE;
}

static gboolean xbee_client_sock_error( XbeeClient *client )
{
	fprintf( stderr, "Error with client socket.  Handle this...\n" );
	return FALSE;
}

static void xbee_client_class_init( XbeeClientClass *klass )
{
	assert( klass != NULL );
	GObjectClass *gobject_class = G_OBJECT_CLASS( klass );

	gobject_class->dispose = xbee_client_dispose;
	gobject_class->finalize = xbee_client_finalize;	
}

static void xbee_client_dispose( GObject *obj )
{
	XbeeClient *self = (XbeeClient*)obj;

	if( self->dispose_has_run ) 
		return;

	self->dispose_has_run = TRUE;

	/* Unreference things here. */

	G_OBJECT_CLASS(parent_class)->dispose(obj);
}

static void xbee_client_finalize( GObject *obj )
{
	XbeeClient *self = (XbeeClient*)obj;

	/* Free stuff */
	/* Free the queues etc. */

	/* Delete the xb-fd-source */
	xbee_fd_source_free( self->source );
	
	G_OBJECT_CLASS (parent_class)->finalize (obj);	
}
