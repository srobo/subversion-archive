#ifndef __XBEE_SERVER_H
#define __XBEE_SERVER_H
#include <glib.h>
#include <glib-object.h>
#include <stdint.h>

#include "xbee-module.h"
#include "xb-fd-source.h"

/* Server stuff */
/* The mainloop needs to monitor the sockets for incoming data.
 * The xbee_proc_incoming function needs to feed data back through  
 * the sockets. */

struct xbee_con_ts;
typedef struct xbee_con_ts xbee_con_t;

struct xbee_server_ts;
typedef struct xbee_server_ts XbeeServer;

/* Server related properties */
struct xbee_server_ts
{
	GObject parent;

	xbee_fd_source_t *source;
	guint source_id;

	/* The context */
	GMainContext *context;

	/* Listening socket */
	int l_fd;

	/* List of clients (xbee_con_t*) */
	GSList *clients;

	/* List of modules */
	GSList *modules;
};

typedef struct
{
	GObjectClass parent;

} XbeeServerClass;

GType xbee_server_get_type( void );

#define XBEE_SERVER_TYPE (xbee_server_get_type())
#define XBEE_SERVER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), XBEE_SERVER_TYPE, XbeeServer))
#define XBEE_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), XBEE_SERVER, XbeeModuleClass))
#define XBEE_IS_SERVER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XBEE_SERVER_TYPE))
#define XBEE_IS_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XBEE_SERVER_TYPE))
#define XBEE_SERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XBEE_SERVER_TYPE, XbeeServerClass))

/* Create a server. Arguments:
 *  - xb: The xbee that we'll be serving
 *  Returns the new server structure */
XbeeServer* xbee_server_new( GMainContext *context );

void xbee_server_free( XbeeServer* serv );

/* Attach an xbee module to the server */
void xbee_server_attach( XbeeServer* serv, XbeeModule* xb );

#endif	/* __XBEE_SERVER_H */
