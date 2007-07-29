#ifndef __COMMON_H
#define __COMMON_H
#include <glib.h>
#include <glib-object.h>

typedef gboolean (*xbee_fd_callback) ( gpointer );

typedef struct
{
	GSource source;
	GPollFD pollfd;

	gpointer userdata;

	xbee_fd_callback read_callback, write_callback, error_callback;

	/* Whether data's ready */
	xbee_fd_callback data_ready;
} xbee_fd_source_t;

xbee_fd_source_t* xbee_fd_source_new( int fd,
				      GMainContext *context,
				      gpointer userdata,
				      xbee_fd_callback read_callback,
				      xbee_fd_callback write_callback,
				      xbee_fd_callback error_callback,
				      xbee_fd_callback data_ready );

void xbee_fd_source_data_ready( xbee_fd_source_t* source );

#endif	/* __COMMON_H */
