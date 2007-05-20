#ifndef __XBEE_CLIENT_H
#define __XBEE_CLIENT_H
#include <glib.h>
#include <glib-object.h>
#include <stdint.h>

#define XB_SERVER_INBUF_LEN 512

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
#define XBEE_IS_CLIENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XBEE_CLIENT_TYPE))
#define XBEE_IS_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XBEE_CLIENT_TYPE))
#define XBEE_CLIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XBEE_CLIENT_TYPE, XbeeClientClass))

typedef struct
{
	GSource source;
	GPollFD pollfd;
} xbee_client_source_t;

struct xbee_client_ts
{
	GObject parent;

	/* Socket file descriptor */
	int fd;

	xbee_client_source_t *source;
	guint source_id;

	/* Incoming frames */
	GQueue *in_frames;

	/* Socket frame reception stuff */
	uint8_t inbuf[ XB_SERVER_INBUF_LEN ];
	uint16_t inpos;
};

XbeeClient* xbee_client_new( GMainContext *context, int sock ); 

#endif	/* __XBEE_CLIENT_H */
