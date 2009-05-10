#include <stdio.h>
#include <unistd.h>
#include <xbee-conn.h>

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

	/* Hang around waiting for xbd to come up */
	while( xbc == NULL ) {
		xbc = xbee_conn_new("/tmp/xbee", NULL);
		sleep(1);
	}

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
	if( len < 1 )
		return;

	switch( data[0] )
	{
	case CMD_START:
	{
		uint16_t pos;
		printf("START:");
		
		for( pos=1; pos<len; pos++ )
			printf("%c", data[pos]);
		printf("\n");
		break;
	}

	case CMD_STOP:
		printf("STOP\n");
		break;

	case CMD_PING:
		/* Ignore pings */
		break;

	case CMD_PING_RESP: {
		uint8_t d[] = {0x00};
		uint8_t channel = 10;

		if( len > 1 )
			channel = data[1];

		xbee_conn_transmit( conn, info->src_addr, d, 1, channel );
		break;
	}

	}

	fflush(stdout);
}

static void chan_set( XbeeConn *conn, uint8_t channel, gpointer userdata )
{
	if( channel == 0 )
		printf("ERROR: Could not listen on channel 1\n");
}

static void info_ready (XbeeConn *conn, uint8_t* snum, gpointer userdata)
{
	/* Channel 1 please */
	xbee_conn_set_channel( conn, 1 );

	printf( "xbee address received\n" );
}
