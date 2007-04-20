#ifndef __XBEE_SERVER_H
#define __XBEE_SERVER_H
#include <glib.h>
#include <stdint.h>


#define XB_SERVER_INBUF_LEN 512

/* Server stuff */
/* The mainloop needs to monitor the sockets for incoming data.
 * The xbee_proc_incoming function needs to feed data back through  
 * the sockets. */

struct xbee_con_ts;
typedef struct xbee_con_ts xbee_con_t;

/* Server related properties */
typedef struct
{
	/* Listening socket */
	int l_fd;

	
	/* List of clients (xbee_con_t*) */
	GSList *clients;

} xbee_server_t;

/* Connection related properties */
struct
{
	/* Socket file descriptor */
	int fd;

	/* Incoming frames */
	GQueue *in_frames;

	/* Socket frame reception stuff */
	uint8_t inbuf[ XB_SERVER_INBUF_LEN ];
	uint16_t inpos;

} xbee_con_ts;

#include "xbee.h"

/* Create a server. Arguments:
 *  - xb: The xbee that we'll be serving
 *  Returns the new server structure */
xbee_server_t* xbee_server_new( Xbee* xb );

void xbee_server_free( xbee_server_t* serv );

/* Process a frame from the xbee.
 * xbee structure contains the frame. */
void xbee_server_proc_frame( xbee_server_t* serv, Xbee* xb );


#endif	/* __XBEE_SERVER_H */
