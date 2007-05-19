#include <stdio.h>
#include <glib.h>
#include "xbee-server.h"

int main( int argc, char** argv )
{
	XbeeModule *xb;
	XbeeServer *server;
	GMainLoop* ml;
	GMainContext *context;

	g_type_init();

	if( argc < 2 )
	{
		fprintf( stderr, "Not enough arguments\n" );
		return 1;
	}

	ml = g_main_loop_new( NULL, FALSE );
	context = g_main_loop_get_context( ml );

	server = xbee_server_new( xb );
	g_return_val_if_fail( server != NULL, 3 );

	xb = xbee_module_open( argv[1], context );
	g_return_val_if_fail( xb != NULL, 1 );

	g_main_loop_run( ml );

	xbee_server_free( server );
	xbee_module_close( xb );

	return 0;
}
