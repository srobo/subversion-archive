#include "xbee-server.h"
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>

#define LISTEN_QUEUE_LEN 5

gboolean xbee_server_listen( xbee_server_t* serv );

xbee_server_t* xbee_server_new( Xbee* xb )
{
	xbee_server_t *serv;
	assert( xb != NULL );

	serv = g_malloc( sizeof( xbee_server_t ) );

	/* No clients yet */
	serv->clients = NULL;

	if( !xbee_server_listen( serv ) )
		return NULL;

	return serv;
}

gboolean xbee_server_listen( xbee_server_t* serv )
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

void xbee_server_free( xbee_server_t* serv )
{
	assert( serv != NULL );


}

void xbee_server_proc_frame( xbee_server_t* serv, Xbee* xb )
{
	assert( serv != NULL && xb != NULL );


}

