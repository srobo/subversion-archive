#include "xbee-server.h"
#include "xbee-client.h"
#include "common-fd.h"
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>

#define LISTEN_QUEUE_LEN 5

gboolean xbee_server_listen( XbeeServer* serv, gchar* spath );

void xbee_server_instance_init( GTypeInstance *gti, gpointer g_class );

/* Process a frame received from the xbee. */
void xbee_server_proc_frame( XbeeModule* xb, uint8_t *data, uint16_t len );

static gboolean xbee_server_source_error( XbeeServer* serv );

/* Callback for when connection is pending */
static gboolean xbee_server_req_con( XbeeServer *serv );

/* Callback for incoming data from an XbeeModule */
static void xbee_server_incoming_data( xb_rx_info_t *info, uint8_t *data, uint16_t len, gpointer *userdata );

/* Callback for when an XbeeClient is disconnected */
void xbee_client_disconnect( XbeeClient *client,
			     XbeeServer *server );

static void xbee_server_dispose( GObject *obj );
static void xbee_server_finalize( GObject *obj );
static void xbee_server_class_init( XbeeServerClass *klass );

static GObjectClass *parent_class = NULL;

GType xbee_server_get_type( void )
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo info = {
			sizeof (XbeeServerClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc)xbee_server_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (XbeeServer),
			0,      /* n_preallocs */
			xbee_server_instance_init    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_OBJECT,
					       "XbeeServerType",
					       &info, 0);
	}
	return type;
}

void xbee_server_instance_init( GTypeInstance *gti, gpointer g_class )
{
	XbeeServer *s = (XbeeServer*)gti;
	/* Initialise the members of the XbeeServer */
	
	s->context = NULL;
	s->clients = NULL;
	s->modules = NULL;
	s->dispose_has_run = FALSE;
	s->sock_fname = NULL;

	parent_class = g_type_class_peek_parent (g_class);
}

XbeeServer* xbee_server_new( GMainContext *context, gchar* spath )
{
	XbeeServer *serv;

	serv = g_object_new( XBEE_SERVER_TYPE, NULL );

	if( !xbee_server_listen( serv, spath ) )
		return NULL;

	serv->context = context;

	serv->source = xbee_fd_source_new( serv->l_fd,
					   context,
					   (gpointer)serv,
					   (xbee_fd_callback)xbee_server_req_con,
					   NULL,
					   (xbee_fd_callback)xbee_server_source_error,
					   NULL );


	return serv;
}

gboolean xbee_server_listen( XbeeServer* serv, gchar* spath )
{
	struct sockaddr_un addr = 
	{
		.sun_family = AF_LOCAL,
	};
	assert( serv != NULL && spath != NULL );

	strncpy( addr.sun_path, spath, 108 );

	/* Create the listening socket  */
	serv->l_fd = socket( PF_LOCAL, SOCK_STREAM, 0 );

	if( serv->l_fd == -1 )
	{
		fprintf( stderr, "Failed to create socket: %m\n" );
		return FALSE;
	}
	
	if( !fd_set_nonblocking( serv->l_fd ) )
		return FALSE;

	if( bind( serv->l_fd, 
		  (struct sockaddr*)&addr, 
		  sizeof( short int ) + strlen( addr.sun_path ) ) == -1 )
	{
		fprintf( stderr, "Failed to bind socket: %m\n" );
		return FALSE;
	}

	if( listen( serv->l_fd, LISTEN_QUEUE_LEN ) == -1 )
	{
		fprintf( stderr, "Failed to listen on socket: %m\n" );
		return FALSE;
	}

	serv->sock_fname = spath;

	return TRUE;
}

void xbee_server_free( XbeeServer* serv )
{
	assert( serv != NULL );
	
	
}

void xbee_server_proc_frame( XbeeModule* xb, uint8_t *data, uint16_t len )
{
	assert( xb != NULL && data != NULL );
	
	printf( "Received a frame from the xbee\n" );
}

void xbee_server_attach( XbeeServer* serv, XbeeModule* xb )
{
	assert( serv != NULL && xb != NULL );
	
	serv->modules = g_slist_append( serv->modules, xb );

	xbee_module_events_t callbacks = 
	{
		.rx_frame = &xbee_server_incoming_data
	};

	xbee_module_register_callbacks ( xb, &callbacks, (gpointer*)serv );
}

static gboolean xbee_server_req_con( XbeeServer *serv )
{
	assert( serv != NULL );
	XbeeClient *client;
	int s;

	/* Accept the connection */
	printf( "%i\n", serv->l_fd );
	s = accept( serv->l_fd, NULL, NULL );

	if( s == -1 )
	{
		fprintf(stderr, "Error accepting connection: %m\n");
		return FALSE;
	}

	if( !fd_set_nonblocking(s) )
		return FALSE;

	client = xbee_client_new( serv->context, s, serv,
				  xbee_client_disconnect );
	assert( client != NULL );
	serv->clients = g_slist_append( serv->clients, client );

//	client->channel = g_slist_position ( serv->clients, g_slist_last (serv->clients) );
//	printf ("Channel: %d\n", client->channel);

	return TRUE;
}

static gboolean xbee_server_source_error( XbeeServer* serv )
{
	fprintf( stderr, "Whoops, listening socket related error - don't know what to do\n" );
	return FALSE;
}

static void xbee_server_incoming_data( xb_rx_info_t *info, uint8_t *data, uint16_t len, gpointer *userdata)
{

	XbeeServer *server = (XbeeServer*)userdata;

	g_debug ("Server: Server has received a frame from XB\n");

	assert (info != NULL && data != NULL && server != NULL);

	XbeeClient *client;
	
	if ((client = server->channels [info->dst_channel]) == NULL)
	{
		fprintf (stderr, "Unable to transmit to channel...channel not available...handle this\n");
	}
	else 
	{
		assert (client != NULL);
		xbee_client_transmit ( client, data, info, len);
	}
	    

	/* TODO: look at frame channel number and send to correct client */
}

void xbee_server_transmit( XbeeServer* serv, 
			   xb_addr_t* addr,
			   void *buf, 
			   uint8_t len )
{
	XbeeModule *xb;
	assert( serv != NULL && addr != NULL && buf != NULL );

	xb = (XbeeModule*)g_slist_nth_data( serv->modules, 0 );
	assert( xb != NULL );

	xbee_transmit( xb, addr, buf, len );
}

void xbee_client_disconnect( XbeeClient *client,
			     XbeeServer *server )
{
	assert( client != NULL && server != NULL );
	
	server->clients = g_slist_remove( server->clients, 
					  client );

	g_object_unref( client );
}

static void xbee_server_dispose( GObject *obj )
{
	XbeeServer *self = (XbeeServer*)obj;

	if( self->dispose_has_run )
		return;

	self->dispose_has_run = TRUE;

	/* Unreference things here. */

	G_OBJECT_CLASS(parent_class)->dispose(obj);
}

static void xbee_server_finalize( GObject *obj )
{
	XbeeServer *serv = (XbeeServer*)obj;

	/* Delete the xb-fd-source */
	if( serv->source != NULL )
		xbee_fd_source_free( serv->source );

	/* Shutdown all connections */
	XbeeClient *n, *c = (XbeeClient*)g_slist_nth( serv->clients, 0 );
	while( c != NULL )
	{
		n = (XbeeClient*)g_slist_next( c );
		g_object_unref( c );
		c = n;
	}

	/* Stop listening */
	shutdown( serv->l_fd, 2 );

	/* Delete the socket file */
	if( serv->sock_fname != NULL )
		unlink( serv->sock_fname );
	
	G_OBJECT_CLASS (parent_class)->finalize (obj);	
}

static void xbee_server_class_init( XbeeServerClass *klass )
{
	assert( klass != NULL );
	GObjectClass *gobject_class = G_OBJECT_CLASS( klass );

	gobject_class->dispose = xbee_server_dispose;
	gobject_class->finalize = xbee_server_finalize;	
}

int16_t xbee_server_req_client_channel ( XbeeServer *server, XbeeClient *client, int16_t channel )
{

	uint16_t i;

	assert (server != NULL && client != NULL);

	/* Assign Requested Channel */
	if (channel >= 1 && channel <=255)
	{
		if (server->channels[channel] == NULL)
		{
			server->channels[channel] = client;
			return channel;
		}
		else
			return -1;
	}
	else
	{
		/* Assign any availableChannel */
		if (channel == -1) 
		{
			for (i=1; i != 256; i++)
			{
				if (server->channels[i] == NULL)
				{
					server->channels[i] = client;
					return i;
				}
			}
			return -2;
		}
	}
	
	/* Invalid Channel Number Requested */
	return 0;
}
