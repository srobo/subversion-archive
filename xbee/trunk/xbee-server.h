#ifndef __XBEE_SERVER_H
#define __XBEE_SERVER_H
#include <glib.h>
#include <glib-object.h>
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
	GObject parent;

	/* Listening socket */
	int l_fd;

	/* List of clients (xbee_con_t*) */
	GSList *clients;

} XbeeServer;

typedef struct
{
	GObjectClass parent;

} XbeeServerClass;

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

GType xbee_server_get_type( void );

#define XBEE_SERVER_TYPE (xbee_server_get_type())
#define XBEE_SERVER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), XBEE_SERVER_TYPE, XbeeServer))
#define XBEE_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), XBEE_SERVER, XbeeModuleClass))
#define XBEE_IS_SERVER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XBEE_SERVER_TYPE))
#define XBEE_IS_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XBEE_SERVER_TYPE))
#define XBEE_SERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XBEE_SERVER_TYPE, XbeeServerClass))

#include "xbee-module.h"

/* Create a server. Arguments:
 *  - xb: The xbee that we'll be serving
 *  Returns the new server structure */
XbeeServer* xbee_server_new( XbeeModule* xb );

void xbee_server_free( XbeeServer* serv );

/* Process a frame from the xbee.
 * xbee structure contains the frame. */
void xbee_server_proc_frame( XbeeServer* serv, XbeeModule* xb );

/* Attach an xbee module to the server */
void xbee_server_attach( XbeeServer* serv, XbeeModule* xb );

#endif	/* __XBEE_SERVER_H */
