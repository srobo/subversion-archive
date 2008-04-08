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
static void rx_frame( uint8_t *data, uint16_t len, xbee_conn_info_t *info );

/* Channel request response callback */
static void chan_set( uint8_t channel );

static const xb_conn_callbacks_t callbacks = 
{
	.rx_frame = rx_frame,
	.chan_set = chan_set
};

int main( int argc, char** argv )
{
	XbeeConn *xbc = NULL;
	GMainLoop* ml;

	g_type_init();
	ml = g_main_loop_new( NULL, FALSE );

	/* Hang around waiting for xbd to come up */
	while( xbc == NULL ) {
		xbc = xbee_conn_new("/tmp/xbee", NULL);
		sleep(1);
	}

	xbee_conn_register_callbacks( xbc, &callbacks );

	/* Channel 1 please */
	xbee_conn_set_channel( xbc, 1 );

	g_main_loop_run( ml );

	return 0;
}

static void rx_frame( uint8_t *data, uint16_t len, xbee_conn_info_t *info )
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

	case CMD_PING_RESP:
		break;
	}

	fflush(stdout);
}

static void chan_set( uint8_t channel )
{
	if( channel == 0 )
		printf("ERROR: Could not listen on channel 1\n");
}
