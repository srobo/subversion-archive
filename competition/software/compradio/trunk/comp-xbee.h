#ifndef __COMP_XBEE_H
#define __COMP_XBEE_H
#include <xbee-conn.h>

enum {
	RADIO_CMD_START,
	RADIO_CMD_STOP,
	RADIO_CMD_PING,
	RADIO_CMD_PING_RESP
};

/* Initialise the radio connection.
   Returns FALSE on failure. */
gboolean comp_xbee_init( void );

/* Ping a robot */
void comp_xbee_ping( xb_addr_t* addr );


#endif	/* __COMP_XBEE_H */
