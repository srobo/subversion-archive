#include <stdio.h>
#include <glib.h>
#include "xbee-server.h"

int main( int argc, char** argv )
{
	XbeeModule *xb;
	XbeeServer *server;
	GMainLoop* ml;

	g_type_init();

	if( argc < 2 )
	{
		fprintf( stderr, "Not enough arguments\n" );
		return 1;
	}

	/* Create the server */
	server = xbee_server_new( xb );
	g_return_val_if_fail( server != NULL, 3 );

	xb = xbee_module_open( argv[1] );
	g_return_val_if_fail( xb != NULL, 2 );

	ml = g_main_loop_new( NULL, FALSE );

	/* Add the xbee source */
	xbee_module_add_source( xb, g_main_loop_get_context( ml ) );




	g_main_loop_run( ml );

	xbee_server_free( server );
	xbee_module_close( xb );

	return 0;
}
