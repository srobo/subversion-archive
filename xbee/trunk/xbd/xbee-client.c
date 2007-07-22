#include "xbee-client.h"
#include <stdio.h>
#include <assert.h>

static void xbee_client_instance_init( GTypeInstance *gti, gpointer g_class );
static gboolean xbee_client_sock_incoming( XbeeClient *client );
static gboolean xbee_client_sock_outgoing( XbeeClient *client );
static gboolean xbee_client_sock_error( XbeeClient *client );

/* Returns TRUE if data is ready to be transmitted */
static gboolean xbee_client_data_ready( XbeeClient *client );

GType xbee_client_get_type( void )
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo info = {
			sizeof (XbeeClientClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			NULL,   /* class_init */
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
	
	g_print( "Wooo.  Client initialised\n" );
}

XbeeClient* xbee_client_new( GMainContext *context, int sock )
{
	XbeeClient* client;
	
	client = g_object_new( XBEE_CLIENT_TYPE, NULL );

	client->fd = sock;
	
	
	client->source = xbee_fd_source_new( client->fd,
					     context,
					     (gpointer)client,
					     (xbee_fd_callback)xbee_client_sock_incoming,
					     (xbee_fd_callback)xbee_client_sock_outgoing,
					     (xbee_fd_callback)xbee_client_sock_error,
					     (xbee_fd_callback)xbee_client_data_ready );

	return client;
}

static gboolean xbee_client_sock_incoming( XbeeClient *client )
{
	assert( client != NULL );

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
