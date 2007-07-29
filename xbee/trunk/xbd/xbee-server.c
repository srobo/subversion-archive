#include "xbee-server.h"
#include "xbee-client.h"
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>
#include <errno.h>

#define LISTEN_QUEUE_LEN 5

gboolean xbee_server_listen( XbeeServer* serv, gchar* spath );

void xbee_server_instance_init( GTypeInstance *gti, gpointer g_class );

/* Process a frame received from the xbee. */
void xbee_server_proc_frame( XbeeModule* xb, uint8_t *data, uint16_t len );

static gboolean xbee_server_source_error( XbeeServer* serv );

/* Callback for when connection is pending */
static gboolean xbee_server_req_con( XbeeServer *serv );

/* Callback for an incoming frame from an XbeeModule  */
static void xbee_server_incoming_frame( XbeeModule *xb, 
					uint8_t *data,
					uint16_t len );

GType xbee_server_get_type( void )
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo info = {
			sizeof (XbeeServerClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			NULL,   /* class_init */
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

	printf( "%i\n", serv->l_fd );

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

	xbee_module_set_incoming_callback( xb, xbee_server_incoming_frame );
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

	client = xbee_client_new( serv->context, s );
	assert( client != NULL );
	serv->clients = g_slist_append( serv->clients, client );

	return TRUE;
}

static gboolean xbee_server_source_error( XbeeServer* serv )
{
	fprintf( stderr, "Whoops, listening socket related error - don't know what to do\n" );
	return FALSE;
}

static void xbee_server_incoming_frame( XbeeModule *xb, 
					uint8_t *data,
					uint16_t len )
{
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
