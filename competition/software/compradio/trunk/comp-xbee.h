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

/* Ping a robot, and ask for a response */
void comp_xbee_ping_req_resp( xb_addr_t* addr );

/* Send a START command to a robot.
   Arguments:
    - addr: The radio address of the robot
    - info: The string to send with the start signal
 */
void comp_xbee_start( xb_addr_t* addr, char* info );

/* Callback function that gets called when xbee's ready for use */
extern void (*comp_xbee_ready) ( void );

/* Callback function that gets called when a response is received */
extern void (*comp_xbee_response) ( xb_addr_t* addr );

/* Caller must free result */
void comp_xbee_addrtostr( xb_addr_t *addr, char** str );

/* Convert a string into an xbee address */
void comp_xbee_strtoaddr( char* str, xb_addr_t* addr );

gboolean cmp_address( xb_addr_t *a1, xb_addr_t *a2 );

#endif	/* __COMP_XBEE_H */
