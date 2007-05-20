#include "xbee-server.h"
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>
#include <errno.h>

#define LISTEN_QUEUE_LEN 5

gboolean xbee_server_listen( XbeeServer* serv );

void xbee_server_instance_init( GTypeInstance *gti, gpointer g_class );

/* Process a frame received from the xbee. */
void xbee_server_proc_frame( XbeeModule* xb, uint8_t *data, uint16_t len );

static gboolean xbee_server_source_prepare( GSource *source, gint *timeout_ );
static gboolean xbee_server_source_check( GSource *source );
static gboolean xbee_server_source_dispatch( GSource *source,
					     GSourceFunc callback, 
					     gpointer user_data );
static void xbee_server_source_finalize( GSource *source );

static void xbee_server_add_source( XbeeServer *serv, GMainContext *context );

/* Callback for when connection is pending */
static gboolean xbee_server_req_con( XbeeServer *serv );

/* The source for the  */
static GSourceFuncs xbee_server_sourcefuncs = 
{
	.prepare = xbee_server_source_prepare,
	.check = xbee_server_source_check,
	.dispatch = xbee_server_source_dispatch,
	.finalize = xbee_server_source_finalize,

	.closure_callback = NULL,
	.closure_marshal = NULL
};

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
	
	s->clients = NULL;

}

XbeeServer* xbee_server_new( XbeeModule* xb, GMainContext *context )
{
	XbeeServer *serv;
	assert( xb != NULL );

	serv = g_object_new( XBEE_SERVER_TYPE, NULL );

	if( !xbee_server_listen( serv ) )
		return NULL;

	xbee_server_add_source( serv, context );

	return serv;
}

gboolean xbee_server_listen( XbeeServer* serv )
{
	struct sockaddr_un addr = 
	{
		.sun_family = AF_LOCAL,
		.sun_path = "/tmp/xbee"
	};
	assert( serv != NULL );

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
	
	serv->clients = g_slist_append( serv->clients, xb );
	
}

static gboolean xbee_server_source_prepare( GSource *source, gint *timeout_ )
{
	assert( timeout_ != NULL && source != NULL );

	*timeout_ = -1;

	/* For the moment, we're not ready */
	return FALSE;
}

static gboolean xbee_server_source_check( GSource *source )
{
	xbee_server_source_t *serv_source = (xbee_server_source_t*) source;
	gushort r;
	assert( source != NULL );

	r = serv_source->pollfd.revents;

	if( r & (G_IO_ERR | G_IO_IN) )
		/* Ready to dispatch */
		return TRUE;

	return FALSE;
}

static gboolean xbee_server_source_dispatch( GSource *source,
					     GSourceFunc callback, 
					     gpointer user_data )
{
	assert( source != NULL );
	xbee_server_source_t *serv_source = (xbee_server_source_t*)source;
	gboolean rval = FALSE;

	/* Call the callback */
	if( callback != NULL )
		rval = callback( user_data );

	return rval;
}

static void xbee_server_source_finalize( GSource *source )
{
	/* ... */
}

static void xbee_server_add_source( XbeeServer *serv, GMainContext *context )
{
	assert( serv != NULL && context != NULL );
	GPollFD *pfd;

	serv->source = (xbee_server_source_t*)g_source_new( &xbee_server_sourcefuncs,
							    sizeof( xbee_server_source_t ) );

	pfd = &serv->source->pollfd;
	pfd->fd = serv->l_fd;
	pfd->events = G_IO_IN | G_IO_ERR;
	g_source_add_poll( (GSource*) serv->source, pfd );

	serv->source_id = g_source_attach( (GSource*)serv->source, context );

	g_source_set_callback( (GSource*)serv->source,
			       (GSourceFunc)xbee_server_req_con,
			       (gpointer)serv,
			       NULL );
	
}

static gboolean xbee_server_req_con( XbeeServer *serv )
{
	assert( serv != NULL );
	struct sockaddr_un addr;
	socklen_t len;
	int s;

	addr.sun_family = AF_LOCAL;

	/* Accept the connection */
	printf( "%i\n", serv->l_fd );
	s = accept( serv->l_fd, NULL, NULL );

	if( s == -1 )
	{
		fprintf(stderr, "Error accepting connection: %m\n");
		return FALSE;
	}

	g_print("Face\n\n");

	return TRUE;
}
