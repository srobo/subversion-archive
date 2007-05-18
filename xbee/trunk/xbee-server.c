#include "xbee-server.h"
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>

#define LISTEN_QUEUE_LEN 5

gboolean xbee_server_listen( XbeeServer* serv );

void xbee_server_instance_init( GTypeInstance *gti, gpointer g_class );

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

XbeeServer* xbee_server_new( XbeeModule* xb )
{
	XbeeServer *serv;
	assert( xb != NULL );

	serv = g_object_new( XBEE_SERVER_TYPE, NULL );

	if( !xbee_server_listen( serv ) )
		return NULL;

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

	return TRUE;
}

void xbee_server_free( XbeeServer* serv )
{
	assert( serv != NULL );
	
	
}

void xbee_server_proc_frame( XbeeServer* serv, XbeeModule* xb )
{
	assert( serv != NULL && xb != NULL );
	
	
}

void xbee_server_attach( XbeeServer* serv, XbeeModule* xb )
{
	
}
