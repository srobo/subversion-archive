#include "xbee-conn.h"
#include <stdio.h>
#include <assert.h>

gboolean tx( XbeeConn *xbc );
void xbtest ( XbeeConn *xbc );

int main( int argc, char** argv )
{
	XbeeConn *xbc;
	GMainLoop* ml;
	GMainContext *context;
	
	ml = g_main_loop_new( NULL, FALSE );
	context = g_main_loop_get_context( ml );

	g_type_init ();

	xbc = xbee_conn_new( "/tmp/xbee", context );

	//g_timeout_add( 250, (GSourceFunc)tx, (gpointer)xbc );
	g_timeout_add(250, (GSourceFunc)xbtest, (gpointer)xbc);
	
	g_main_loop_run( ml );

	return 0;
} 

gboolean tx( XbeeConn *xbc )
{
  uint8_t data[] = {0,1,2,3,4,5};
	xb_addr_t addr =
		{
			.type = XB_ADDR_64,
			.addr = {0x00, 0x13, 0xA2, 0x00, 0x40, 0x09, 0x00, 0xA9}
		};
	assert( xbc != NULL );

	xbee_conn_transmit( xbc, addr, data, 6 );
	
	return TRUE;
}

void xbtest ( XbeeConn *xbc )
{
  
  gchar data[] = "Hello Dave\0";
  
  xbee_conn_command_test (xbc, data);
  
}
