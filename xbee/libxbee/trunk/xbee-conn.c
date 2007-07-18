#include "xbee-conn.h"

gboolean xbee_conn_transmit( XbeeConn* xbc, xb_addr_t addr, uint8_t* data, uint16_t len )
{
	g_print( "Transmit!\n" );
	return TRUE;
}
