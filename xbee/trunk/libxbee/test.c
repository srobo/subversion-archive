#include "xbee-conn.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "xbee-conn.h"

gboolean tx( XbeeConn *xbc );
void xbtest ( XbeeConn *xbc );
void rx_data (uint8_t * data, uint16_t len, xbee_conn_info_t *info);

int main( int argc, char** argv )
{
	XbeeConn *xbc;
	GMainLoop* ml;
	GMainContext *context;
	
	xb_conn_callbacks_t callbacks;

	ml = g_main_loop_new( NULL, FALSE );
	context = g_main_loop_get_context( ml );

	g_type_init ();

	if (argc < 2)
	{
		fprintf (stdout, "USAGE:\n");
		fprintf (stdout, "  test [OPTIONS]\n\n");
		fprintf (stdout, "Options:\n");
		fprintf (stdout, " -s <socket>     Transmit/Receive via specified socket\n");
		return (0);
	}
			
	if (argc > 2)
	{		
		if (!strcmp (argv[1], "-s"))
		{
			xbc = xbee_conn_new (argv[2], context);
			printf ("\nReceive mode activated:\n");
			callbacks.rx_frame = &rx_data;
			xbee_conn_register_callbacks (xbc, &callbacks);
			printf ("\nBegin Transmit:\n");
			g_timeout_add(3000, (GSourceFunc)tx, (gpointer)xbc);
		}
	}
	
	
	g_main_loop_run( ml );

	return 0;
} 

gboolean tx( XbeeConn *xbc )
{
	uint8_t data[] = {1,1,1,1,1,1,1,1,1,1,1};
	xb_addr_t addr =
		{
			.type = XB_ADDR_64,
			.addr = {0x00, 0x13, 0xA2, 0x00, 0x40, 0x09, 0x00, 0xA8}
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

void rx_data (uint8_t * data, uint16_t len, xbee_conn_info_t *info)
{
	int i =0;
	
	assert (data != NULL && info != NULL);

	printf ("\nRx_data Function:\n");
	
	if (info->src_addr.type == XB_ADDR_16)
	{
		printf ("RX_16 frame received\n");
		printf ("Source address: %2.2X %2.2X\n", info->src_addr.addr[0], info->src_addr.addr[1]);
	}
	else
	{
		printf ("RX_64 frame received\n");
		printf ("Source address: ");
		for (i=0; i<8; i++)
		{
			printf ("%2.2X ", info->src_addr.addr[i]);
		}
		printf (" \n ");
	}
	
	printf ("Signal Strength (RSSI): %d\n", info->rssi);

	if (info->pan_broadcast)
		printf ("Pan Broadcast: YES\n");
	else
		printf ("Pan Broadcast: NO\n");
	
	if (info->address_broadcast)
		printf ("Address Broadcast: YES\n");
	else
		printf ("Address Broadcast: NO\n");

	printf ("Data Length: %d\n", (unsigned int)len);
	printf ("Data: ");
	for (i=0; i<len; i++)
	{
		printf ("%2.2X ", (unsigned int)data[i]);
	}
	printf (" \n ");
}

