#include "xbee-client.h"
#include <assert.h>

static void xbee_client_instance_init( GTypeInstance *gti, gpointer g_class );

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
	
	/* TODO: register source */
	
	return client;
}

