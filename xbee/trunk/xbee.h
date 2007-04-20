#ifndef __XBEE_H
#define __XBEE_H
#define _GNU_SOURCE		/* For TEMP_FAILURE_RETRY */
#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <assert.h>
#include <glib.h>
#include <glib-object.h>

#define XB_INBUF_LEN 512
#define XB_OUTBUF_LEN 512

typedef struct
{
	uint16_t len;
	uint8_t *data;
} xb_frame_t;

struct xbee_ts;

/* Note: redefined to capital for gobject change */
typedef struct xbee_ts Xbee;	

typedef struct
{
	GObjectClass parent;

	/* Nothing here */
} XbeeClass;

#define XBEE_TYPE (xbee_get_type())
#define XBEE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), XBEE_TYPE, Xbee))
#define XBEE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), XBEE_BAR, XbeeClass))
#define XBEE_IS_XBEE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XBEE_TYPE))
#define XBEE_IS_XBEE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XBEE_TYPE))
#define XBEE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XBEE_TYPE, XbeeClass))

GType xbee_get_type( void );

#include "xbee-server.h"

/*** Source type ***/
typedef struct
{
	GSource source;
	GPollFD pollfd;
	Xbee *xb; 		/* Parent pointer */
} xbee_source_t;

struct xbee_ts
{
	GObject parent;

	/* private */
	int fd;

	xbee_source_t *source; 	/* probably need this to change pollfd */
	guint source_id;

	/* Need to know what server we're associated with,
	   so that we can send incoming frames to it */
	xbee_server_t *server;

	/* Whether we're in API mode */
	gboolean api_mode;
	
	/* Whether we're in AT command mode */
	gboolean at_mode;
	/* The time at which we entered AT command mode */
	struct timeval at_time;

	/*** Serial configuration ***/
	uint32_t baud;
	enum { PARITY_NONE, PARITY_ODD, PARITY_EVEN } parity;
	uint8_t stop_bits;
	enum { FLOW_NONE, FLOW_RTSCTS, FLOW_SOFTWARE } flow_control;

	/*** Transmission ***/
	/* Queue of outgoing frames - all of xb_frame_t */
	/* Note: the data in these frames has already been escaped */
	GQueue* out_frames; 

	/* Checksum value for the currently transmitting frame */
	uint8_t o_chk;
	gboolean checked;	/* Whether the checksum has been calculated */
	gboolean tx_escaped;	/* Whether the next byte has been escaped */
	/* The next byte to be transmitted within the current frame */
	uint16_t tx_pos;

	/*** Reception ***/
	/* Buffer of incoming data */
	/* Always contains the beginning part of a frame */
	uint8_t inbuf[XB_INBUF_LEN];
	/* The number of bytes in the input buffer */
	uint16_t in_len;
	/* Whether the next byte received should be escaped */
	gboolean escape;

	/*** Stats ***/
	uint32_t bytes_discarded, frames_discarded; /* Frames with invalid checksums */
	uint32_t bytes_rx, bytes_tx;   
	uint32_t frames_rx, frames_tx;  /* Valid checksum frames received */

};


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

/* Create a connection to an xbee.
 * Opens the serial port given in fname, and fills the 
 * structure *xb with stuff. */
Xbee* xbee_open( char* fname );

/* Close an xbee connection */
void xbee_close( Xbee* xb );

/* Add an xbee to a mainloop */
void xbee_add_source( Xbee *xb, GMainContext *context );

#endif	/* __XBEE_H */
