#include "xbee-conn.h"
#include <stdio.h>
#include <assert.h>

gboolean tx( XbeeConn *xbc );
void xbtest ( XbeeConn *xbc );
void rx_data (uint8_t * data, uint16_t len);

int main( int argc, char** argv )
{
	XbeeConn *xbc;
	GMainLoop* ml;
	GMainContext *context;
	
	xb_conn_callbacks_t callbacks;

	ml = g_main_loop_new( NULL, FALSE );
	context = g_main_loop_get_context( ml );

	g_type_init ();

	xbc = xbee_conn_new( "/tmp/xbee", context );

	callbacks.rx_frame = &rx_data;
	xbee_conn_register_callbacks (xbc, &callbacks);



	//g_timeout_add( 250, (GSourceFunc)tx, (gpointer)xbc );
//	g_timeout_add(3000, (GSourceFunc)tx, (gpointer)xbc);
	
	g_main_loop_run( ml );

	return 0;
} 

gboolean tx( XbeeConn *xbc )
{
	uint8_t data[] = {1,1,1,1,1,1,1,1,1,1,1};
	xb_addr_t addr =
		{
			.type = XB_ADDR_64,
			.addr = {0x00, 0x13, 0xA2, 0x00, 0x40, 0x09, 0x00, 0xA9}
		};
	assert( xbc != NULL );

	xbee_conn_transmit( xbc, addr, data, 11 );
	
	return TRUE;
}

void xbtest ( XbeeConn *xbc )
{
  
  gchar data[] = "Hello Dave\0";
  
  xbee_conn_command_test (xbc, data);
  
}

void rx_data (uint8_t * data, uint16_t len)
{
	int i =0;
	
	printf ("\nRx_data Function:");
	printf ("\nLen is at: %2.2X\n", (unsigned int)len);

	for (i=0; i<len; i++)
	{
		printf ("%2.2X ", (unsigned int)data[i]);
	}
	
}

