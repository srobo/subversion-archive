#ifndef __XBEE_CONN_H
#define __XBEE_CONN_H
#include <glib.h>
#include <glib-object.h>
#include <stdint.h>

typedef enum
{
	XB_ADDR_16,
	XB_ADDR_64
} xb_addr_len_t;

typedef struct
{
	xb_addr_len_t type;
	uint8_t addr[8];
} xb_addr_t;

typedef uint8_t chan_num_t;


struct xbee_conn_t;

typedef struct xbee_conn_t XbeeConn;



/* Open a connection to the server */
XbeeConn* xbee_conn_new( char* addr , GMainContext *context );

/* Close the connection */
void xbee_conn_close( XbeeConn* xbc );

/* Get a channel
 * Returns 0 on error (?) */
chan_num_t xbee_conn_get_channel( XbeeConn* xbc, 
				  gboolean (*receive) ( XbeeConn*, uint8_t* data, uint16_t len ) );

/* Release a channel */
void xbee_conn_release_channel( XbeeConn* xbc, chan_num_t channel );

/* Transmit some data */
gboolean xbee_conn_transmit( XbeeConn* xbc, xb_addr_t addr, uint8_t* data, uint16_t len );

#endif
