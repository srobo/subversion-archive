#include "xbee-client.h"
#include <stdio.h>
#include <assert.h>

static void xbee_client_instance_init( GTypeInstance *gti, gpointer g_class );

static gboolean xbee_client_source_prepare( GSource *source, gint *timeout_ );
static gboolean xbee_client_source_check( GSource *source );
static gboolean xbee_client_source_dispatch( GSource *source,
					     GSourceFunc callback, 
					     gpointer user_data );
static void xbee_client_source_finalize( GSource *source );

static void xbee_client_add_source( XbeeClient *client, GMainContext *context );

static GSourceFuncs xbee_client_sourcefuncs = 
{
	.prepare = xbee_client_source_prepare,
	.check = xbee_client_source_check,
	.dispatch = xbee_client_source_dispatch,
	.finalize = xbee_client_source_finalize,

	.closure_callback = NULL,
	.closure_marshal = NULL
};

static gboolean xbee_client_event( XbeeClient *client );

static void xbee_client_sock_incoming( XbeeClient *client );
static void xbee_client_sock_outgoing( XbeeClient *client );

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
	
	xbee_client_add_source( client, context );
	
	return client;
}

static gboolean xbee_client_source_prepare( GSource *source, gint *timeout_ )
{
	assert( timeout_ != NULL && source != NULL );

	*timeout_ = -1;

	/* We're not ready */
	return FALSE;
}

/* Return TRUE if ready to be dispatched */
static gboolean xbee_client_source_check( GSource *source )
{
	assert( source != NULL );
	xbee_client_source_t *client_source = (xbee_client_source_t*)source; 
	gushort r = client_source->pollfd.revents;

	if( r & (G_IO_ERR | G_IO_HUP | G_IO_IN | G_IO_OUT | G_IO_NVAL) )
	{
		/* ready to dispatch */
		return TRUE;
	}

	return FALSE; 
}

static gboolean xbee_client_source_dispatch( GSource *source,
					     GSourceFunc callback, 
					     gpointer user_data )
{
	assert( source != NULL );
/* 	xbee_client_source_t *client_source = (xbee_client_source_t*)source;  */
	gboolean rval = FALSE;

	/* Call the callback */
	if( callback != NULL )
		rval = callback( user_data );

	/* TODO */
/* 	/\* Modulate the write requirement if necessary *\/ */
/* 	if( xbee_module_outgoing_queued( xb_source->xb ) ) */
/* 		xb_source->pollfd.events |= G_IO_OUT; */
/* 	else */
/* 		xb_source->pollfd.events &= ~G_IO_OUT; */

	return rval;
}

static void xbee_client_source_finalize( GSource *source )
{

}

static void xbee_client_add_source( XbeeClient *client, GMainContext *context )
{
	assert( client != NULL && context != NULL );
	GPollFD *pfd;

	client->source = (xbee_client_source_t*)g_source_new( &xbee_client_sourcefuncs,
							      sizeof( xbee_client_source_t ) );

	pfd = &client->source->pollfd;
	pfd->fd = client->fd;
	pfd->events = G_IO_IN | G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL;
	g_source_add_poll( (GSource*) client->source, pfd );

	client->source_id = g_source_attach( (GSource*)client->source, context );

	g_source_set_callback( (GSource*)client->source,
			       (GSourceFunc)xbee_client_event,
			       (gpointer)client,
			       NULL );
}

static gboolean xbee_client_event( XbeeClient *client )
{
	assert( client != NULL );

	if( client->source->pollfd.revents & (G_IO_ERR | G_IO_HUP) )
	{
		fprintf( stderr, "IO Error\n" );
		return FALSE;
	}
	
	if( client->source->pollfd.revents & G_IO_IN )
		xbee_client_sock_incoming( client );

	if( client->source->pollfd.revents & G_IO_OUT )
		xbee_client_sock_outgoing( client );

	return TRUE;
}

static void xbee_client_sock_incoming( XbeeClient *client )
{
	assert( client != NULL );

}

static void xbee_client_sock_outgoing( XbeeClient *client )
{
	assert( client != NULL );

}

