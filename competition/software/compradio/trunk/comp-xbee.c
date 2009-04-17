#define _GNU_SOURCE
#include "comp-xbee.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Callback for info_ready */
static void comp_xbee_info_ready(XbeeConn *conn, uint8_t* snum, gpointer userdata );

/* Callback for chan_set */
static void comp_xbee_chan_set( XbeeConn *conn, uint8_t channel, gpointer userdata );

/* Callback for receiving a frame */
static void comp_xbee_rx_frame( XbeeConn *conn, uint8_t *data, uint16_t len, xbee_conn_info_t *info, gpointer userdata );

static XbeeConn *xbc;
/* The channel we get from the server */
uint8_t MY_CHANNEL;

const xb_conn_callbacks_t xb_callbacks = 
{
	.rx_frame = comp_xbee_rx_frame,
	.chan_set = comp_xbee_chan_set,
	.info_ready = comp_xbee_info_ready
};

void (*comp_xbee_response) ( xb_addr_t* addr ) = NULL;

void (*comp_xbee_ready) ( void ) = NULL;

gboolean comp_xbee_init( void )
{
	xbc = xbee_conn_new( "/tmp/xbee", NULL );
	if( xbc == NULL )
		return FALSE;

	xbee_conn_register_callbacks( xbc, &xb_callbacks );
	return TRUE;
}

void comp_xbee_ping( xb_addr_t* addr )
{
	uint8_t data[] = { RADIO_CMD_PING };
	uint8_t i;
	assert( addr != NULL );

	xbee_conn_transmit( xbc, *addr, data, 1, 1 );
	printf("Pinging: ");
	for(i=0; i<8; i++) {
		printf("%2.2hhx", addr->addr[i]);
		if( i < 7 ) printf(":");
	}
	printf("\n");
}


void comp_xbee_start( xb_addr_t* addr, char* info )
{
	uint8_t *data = NULL;
	assert( addr != NULL );

	data = g_malloc(strlen(info) + 1);
	g_memmove(data+1,info,strlen(info));
	data[0] = RADIO_CMD_START;

	xbee_conn_transmit( xbc, *addr, data, strlen(info)+1, 1 );

	printf("START: %s\n", info);

	free(data);
}

void comp_xbee_ping_req_resp( xb_addr_t* addr )
{
	uint8_t d[] = {RADIO_CMD_PING_RESP, 0};

	d[1] = MY_CHANNEL;

	xbee_conn_transmit( xbc, *addr, d, 2, 1 );
}

static void comp_xbee_info_ready(XbeeConn *conn, uint8_t* snum, gpointer userdata )
{
	/* Request a channel -- any will do */
	xbee_conn_set_channel( conn, 0 );
}

static void comp_xbee_chan_set( XbeeConn *conn, uint8_t channel, gpointer userdata )
{
	g_debug("Successfully listening on channel %hhu", channel);
	MY_CHANNEL = channel;

	if( comp_xbee_ready != NULL )
		comp_xbee_ready();
}

void comp_xbee_addrtostr( xb_addr_t *addr, char** str )
{
        if( addr->type == XB_ADDR_64 ) {
                asprintf( str, "%2.2hhx%2.2hhx%2.2hhx%2.2hhx%2.2hhx%2.2hhx%2.2hhx%2.2hhx",
                          addr->addr[0], addr->addr[1], addr->addr[2], addr->addr[3],
                          addr->addr[4], addr->addr[5], addr->addr[6], addr->addr[7] );
        } else
                g_error( "16-bit addresses not supported\n" );
}

static void comp_xbee_rx_frame( XbeeConn *conn, uint8_t *data, uint16_t len, xbee_conn_info_t *info, gpointer userdata )
{
        /* Discard frames that are too short */
        if( len != 1 ) {
                printf( "Discarding frame of incorrect length (%hu bytes)\n", len );
                return;
        }

        /* Ping response is a single frame with one 0x00 byte in it  */
        if( data[0] == 0x00 ) {
                char *s;
                comp_xbee_addrtostr( &info->src_addr, &s );

		if( comp_xbee_response != NULL )
			comp_xbee_response( &info->src_addr );

		printf( "Ping response from %s\n", s );
                free(s);
        }
}

void comp_xbee_strtoaddr( char* str, xb_addr_t* addr )
{
	sscanf(str, "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
	       &addr->addr[0], &addr->addr[1], &addr->addr[2], &addr->addr[3],
	       &addr->addr[4], &addr->addr[5], &addr->addr[6], &addr->addr[7] );
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
