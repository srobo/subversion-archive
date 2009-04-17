#define _GNU_SOURCE
#include "comp-xbee.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static XbeeConn *xbc;

const xb_conn_callbacks_t xb_callbacks = 
{
	.rx_frame = NULL,
	.chan_set = NULL,
	.info_ready = NULL       
};

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
