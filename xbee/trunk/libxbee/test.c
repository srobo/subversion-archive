#include "xbee-conn.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <glib.h>

#include "xbee-conn.h"

gboolean tx( XbeeConn *xbc );
void xbtest ( XbeeConn *xbc );
void rx_data (uint8_t * data, uint16_t len, xbee_conn_info_t *info);
void config_options (int argc, char **argv);

static gchar *socket = NULL,
//	*address = NULL,
//	*channel = NULL,
	*receive = NULL,
	*transmit = NULL;
static int8_t channel = -1;
static int16_t address = -1;


static GOptionEntry entries[] = 
{
	{ "socket", 's', 0, G_OPTION_ARG_FILENAME, &socket, "Socket path to connect to" },
	{ "address", 'a', 0, G_OPTION_ARG_INT, &address, "Last Byte of destination Address" },
	{ "channel", 'c', 0, G_OPTION_ARG_INT, &channel, "Destination Channel" },
	{ "receive", 'r', 0, G_OPTION_ARG_NONE, &receive, "Receive Data Only" },
	{ "transmit", 't', 0, G_OPTION_ARG_NONE, &transmit, "Transmit Data Only" },
	{ NULL }
};


int main( int argc, char** argv )
{
	XbeeConn *xbc;
	GMainLoop* ml;
	GMainContext *context;
	
	xb_conn_callbacks_t callbacks;

	config_options ( argc, argv );

	ml = g_main_loop_new( NULL, FALSE );
	context = g_main_loop_get_context( ml );

	g_type_init ();

			xbc = xbee_conn_new (socket, context);
			if (receive != NULL)
			{
				printf ("\nReceive mode activated:\n");
				callbacks.rx_frame = &rx_data;
				xbee_conn_register_callbacks (xbc, &callbacks);
			}
			else if (transmit != NULL)
			{
				printf ("\nBegin Transmit:\n");
				g_timeout_add(3000, (GSourceFunc)tx, (gpointer)xbc);
			}

	g_main_loop_run( ml );

	return 0;
} 

gboolean tx( XbeeConn *xbc)
{
	uint8_t data[] = {1,1,1,1,1,1,1,1,1,1,1};
	xb_addr_t addr =
		{
			.type = XB_ADDR_64,
			.addr = {0x00, 0x13, 0xA2, 0x00, 0x40, 0x09, 0x00, address}
		};
	assert( xbc != NULL );

	xbee_conn_transmit( xbc, addr, data, 11, channel );
	
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

void config_options ( int argc, char **argv )
{
	GOptionContext *opt_context;
	GError *error = NULL;

	opt_context = g_option_context_new ("");
	g_option_context_set_summary ( opt_context, "Test file" );
	g_option_context_add_main_entries(opt_context, entries, NULL);

	if (!g_option_context_parse (opt_context, &argc, &argv, &error ))
	{
		g_print ("Error: %s\n", error->message);
		exit (1);
	}

	if (transmit != NULL && (address <= 0 || channel <= 0))
	{
		g_print ("Value: %d %d\n", address, channel);
		g_print ("No destination address or channel\n");
		exit (1);
	}

	if (transmit == NULL && receive == NULL)
	{
		g_print ("No mode specified\n");
		exit (1);
	}


}
