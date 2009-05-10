/* Ping */
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <glib.h>
#include <xbee-conn.h>
#include <stdlib.h>

enum
{
	/* Start */
	CMD_START,
	/* Stop */
	CMD_STOP,
	/* Ping command */
	CMD_PING,		
	/* Ping with response */
	CMD_PING_RESP
};

static uint8_t MY_CHANNEL;

/* Receive callback */
static void rx_frame( XbeeConn *conn,
		      uint8_t *data,
		      uint16_t len,
		      xbee_conn_info_t *info,
		      gpointer userdata );

/* Channel request response callback */
static void chan_set( XbeeConn *conn, uint8_t channel, gpointer userdata );

/* info_ready callback */
static void info_ready (XbeeConn *conn, uint8_t* snum, gpointer userdata);

void strtoaddr( char* str, xb_addr_t* addr )
{
        sscanf(str, "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
               &addr->addr[0], &addr->addr[1], &addr->addr[2], &addr->addr[3],
               &addr->addr[4], &addr->addr[5], &addr->addr[6], &addr->addr[7] );
}

/* Caller must free string */
void addrtostr( xb_addr_t *addr, char** str )
{
	if( addr->type == XB_ADDR_64 ) {
		asprintf( str, "%2.2hhx%2.2hhx%2.2hhx%2.2hhx%2.2hhx%2.2hhx%2.2hhx%2.2hhx",
			  addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3],
			  addr->addr[4], addr->addr[5], addr->addr[6], addr->addr[7] );
	} else
		g_error( "16-bit addresses not supported\n" );
}

gboolean cmp_address( xb_addr_t *a1, xb_addr_t *a2 )
{
	uint8_t i;
	g_assert( a1->type == XB_ADDR_64 && a2->type == XB_ADDR_64 );

	for( i=0; i<8; i++ )
		if( a1->addr[i] != a2->addr[i] )
			return FALSE;

	return TRUE;	
}

static xb_addr_t ping_addr = {
	.type = XB_ADDR_64
};

int main( int argc, char** argv )
{
	XbeeConn *xbc = NULL;
	GMainLoop* ml;
	xb_conn_callbacks_t callbacks = 
		{
			.userdata = NULL,
			.rx_frame = rx_frame,
			.chan_set = chan_set,
			.info_ready = info_ready
		};

	g_type_init();
	ml = g_main_loop_new( NULL, FALSE );

	if( argc != 2 ) {
		fprintf( stderr, "Usage: pinger XBEE_ADDRESS\n" );
		return 1;
	}

	strtoaddr( argv[1], &ping_addr );

	xbc = xbee_conn_new("/tmp/xbee", NULL);
	xbee_conn_register_callbacks( xbc, &callbacks );
	g_main_loop_run( ml );

	return 0;
}

static void rx_frame( XbeeConn *conn, 
		      uint8_t *data, 
		      uint16_t len, 
		      xbee_conn_info_t *info, 
		      gpointer userdata )
{
	/* First byte of the incoming frame is the command */

	/* Discard frames that are too short */
	if( len != 1 ) {
		printf( "Discarding frame of incorrect length (%hu bytes)\n", len );
		return;
	}

	/* Ping response is a single frame with one 0x00 byte in it  */
	if( data[0] == 0x00 ) {
		char *s;
		addrtostr( &info->src_addr, &s );

		if( cmp_address( &ping_addr, &info->src_addr ) ) {
			printf( "Ping response from %s\n", s );
			exit(0);
		}
		else
			printf( "Unexpected response from different host (%s)\n", s );
		free(s);
	}

}

static void chan_set( XbeeConn *conn, uint8_t channel, gpointer userdata )
{
	if( channel == 0 )
		g_error("Couldn't listen on channel 10\n");
	else {
		uint8_t d[] = {CMD_PING_RESP, 0};

		d[1] = MY_CHANNEL = channel;
		g_debug("Successfully listening on channel %hhu\n", channel);

		xbee_conn_transmit( conn, ping_addr, d, 2, 1 );
	}
}

static void info_ready (XbeeConn *conn, uint8_t* snum, gpointer userdata)
{
	/* Request any free channel */
	xbee_conn_set_channel( conn, 0 );
}
