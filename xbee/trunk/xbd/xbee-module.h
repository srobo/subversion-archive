#ifndef __XBEE_MODULE_H
#define __XBEE_MODULE_H
#define _GNU_SOURCE		/* For TEMP_FAILURE_RETRY */
#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <assert.h>
#include <glib.h>
#include <glib-object.h>
#include "xb-fd-source.h"

#define XB_INBUF_LEN 512
#define XB_OUTBUF_LEN 512

typedef struct
{
	uint16_t len;
	uint8_t *data;
} xb_frame_t;

struct xbee_ts;

typedef struct xbee_ts XbeeModule;	

typedef void (*xbee_callback_t) ( XbeeModule *xb,
				  uint8_t *data,
				  uint16_t len );

typedef struct
{
	GObjectClass parent;

	/* Nothing here */
} XbeeModuleClass;

GType xbee_module_get_type( void );

#define XBEE_MODULE_TYPE (xbee_module_get_type())
#define XBEE_MODULE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), XBEE_MODULE_TYPE, XbeeModule))
#define XBEE_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), XBEE_MODULE, XbeeModuleClass))
#define XBEE_IS_MODULE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XBEE_MODULE_TYPE))
#define XBEE_IS_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XBEE_MODULE_TYPE))
#define XBEE_MODULE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XBEE_MODULE_TYPE, XbeeModuleClass))

/*** Source type ***/
typedef struct
{
	GSource source;
	GPollFD pollfd;
	XbeeModule *xb; /* Parent pointer */
} xbee_source_t;

struct xbee_ts
{
	GObject parent;

	/* private */
	int fd;

	xbee_fd_source_t *source;
	guint source_id;

	/* Whether we're in API mode */
	gboolean api_mode;
	
	/* Whether we're in AT command mode */
	gboolean at_mode;
	/* The time at which we entered AT command mode */
	struct timeval at_time;

	/*** serial configuration ***/
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

	/* Callback for receiving a frame.
	 * The data pointed to contains the beginning part of the frame */
	xbee_callback_t in_callback;

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
XbeeModule* xbee_module_open( char *fname, GMainContext *context );

/* Close an xbee connection */
void xbee_module_close( XbeeModule *xb );

/* Add an xbee to a mainloop */
void xbee_module_add_source( XbeeModule *xb, GMainContext *context );

/* Set the callback for when a frame is received.
 * Frame is freed after callback function completes.
 * Data pointed to by data and len pointers is unavailable after
 * the callback has returned.
 */
void xbee_module_set_incoming_callback( XbeeModule *xb, 
					xbee_callback_t f );

#endif	/* __XBEE_MODULE_H */
