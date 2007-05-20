#ifndef __XBEE_CLIENT_H
#define __XBEE_CLIENT_H
#include <glib.h>
#include <glib-object.h>

struct xbee_client_ts;
typedef struct xbee_client_ts XbeeClient;

typedef struct
{
	GObjectClass parent;

	/* Nothing here */
} XbeeClientClass;

GType xbee_client_get_type( void );

#define XBEE_CLIENT_TYPE (xbee_client_get_type())
#define XBEE_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), XBEE_CLIENT_TYPE, XbeeClient))
#define XBEE_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), XBEE_CLIENT, XbeeClientClass))
#define XBEE_IS_MODULE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XBEE_CLIENT_TYPE))
#define XBEE_IS_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XBEE_CLIENT_TYPE))
#define XBEE_CLIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XBEE_CLIENT_TYPE, XbeeClientClass))

struct xbee_client_ts
{
	GObject parent;
};



#endif	/* __XBEE_CLIENT_H */
