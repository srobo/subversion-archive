#ifndef __commands_h
#define __commands_h

typedef enum
{
	XBEE_COMMAND_TEST = 0,
	XBEE_COMMAND_TRANSMIT = 1,
	XBEE_COMMAND_SET_CHANNEL = 2
} xbee_client_command_t;

typedef enum
{
	XBEE_CONN_RX_CHANNEL = 0,
	XBEE_CONN_RECEIVE_TXDATA = 1
} xbee_conn_command_t;

#endif
