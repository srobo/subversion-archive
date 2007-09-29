#ifndef __XBEE_CONN_H
#define __XBEE_CONN_H
#include <glib.h>
#include <glib-object.h>
#include <stdint.h>

#include "xb-fd-source.h"
#include "libxcli.h"

#define XB_CONN_OUTBUF_LEN 512

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

typedef struct
{
  GObjectClass parent; 
} XbeeConnClass;

GType xbee_conn_get_type ( void );

#define XBEE_CONN_TYPE (xbee_conn_get_type())
#define XBEE_CONN(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), XBEE_CONN_TYPE, XbeeConn))
#define XBEE_CONN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), XBEE_CONN, XbeeConnClass))
#define XBEE_IS_CONN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XBEE_CONN_TYPE))
#define XBEE_IS_CONN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XBEE_CONN_TYPE))
#define XBEE_CONN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), XBEE_CONN_TYPE, XbeeConnClass))


typedef struct
{
	/* frame->len represents the length of frame (exluding the 2 frame length bytes) */
	uint16_t len;
	uint8_t *data;
} xb_frame_t;



typedef struct
{
	xb_addr_t src_addr;
	uint8_t rssi;
	gboolean pan_broadcast;
	gboolean address_broadcast;
	uint8_t src_channel;
	uint8_t dst_channel;
}xbee_conn_info_t;


typedef struct
{
	void (*rx_frame) (uint8_t *data, uint16_t len, xbee_conn_info_t *info);
	void (*chan_set) (int16_t channel);
} xb_conn_callbacks_t;

struct xbee_conn_t
{
	GObject parent;
	
	/* Socket file descriptor */
	int fd;
	
	xbee_fd_source_t *source;
	guint source_id;

	GMainContext *context;	

	GQueue *out_frames;
	int outpos;	

	uint16_t inpos;

	//Input buffer for data, contains frame length prefix
	uint8_t inbuf [ XBEE_MAX_FRAME ];

	uint16_t flen;
	
	xb_conn_callbacks_t callbacks;

	/* Client Channel Number */
	int16_t channel;

};

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
gboolean xbee_conn_transmit( XbeeConn* conn, xb_addr_t addr, uint8_t* data, uint16_t len , uint8_t dst_channel);

/* Xbee Command Test */
void xbee_conn_command_test ( XbeeConn * conn, gchar *data);

/* Register the Libxbee Callbacks */
void xbee_conn_register_callbacks (XbeeConn *conn, xb_conn_callbacks_t *callbacks);

/* Sets the channel of the client associated with the XbeeConn */

void xbee_conn_set_channel ( XbeeConn *conn, int16_t channel );


#endif	/* __XBEE_CONN_H */
