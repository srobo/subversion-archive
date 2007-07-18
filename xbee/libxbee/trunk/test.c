#include "xbee-conn.h"

int main( int argc, char** argv )
{
	XbeeConn *xbc;
	GMainLoop* ml;
	GMainContext *context;
	
	ml = g_main_loop_new( NULL, FALSE );
	context = g_main_loop_get_context( ml );

	xbc = xbee_conn_new( "/tmp/xbee", context );
	
	g_main_loop_run( ml );

	return 0;
} 
